#include "cpu/armv7.h"

#include "memory/memory.h"

#include <stdexcept>

namespace emu {

namespace {
constexpr std::uint32_t N_FLAG = 1u << 31;
constexpr std::uint32_t Z_FLAG = 1u << 30;

std::uint32_t rotate_right(std::uint32_t value, unsigned amount) {
    amount &= 31u;
    if (amount == 0) return value;
    return (value >> amount) | (value << (32u - amount));
}
}

ARMv7::ARMv7(Memory& memory) : memory_(memory) { reset(); }

void ARMv7::reset(std::uint32_t entry) {
    for (auto& reg : regs_) reg = 0;
    cpsr_ = 0;
    thumb_ = false;
    regs_[15] = entry;
}

std::uint32_t ARMv7::reg(unsigned index) const {
    if (index >= 16) throw std::out_of_range("ARM register index out of range");
    return regs_[index];
}

bool ARMv7::condition_passed(std::uint32_t condition) const {
    const bool n = (cpsr_ & N_FLAG) != 0;
    const bool z = (cpsr_ & Z_FLAG) != 0;
    switch (condition) {
    case 0x0: return z;
    case 0x1: return !z;
    case 0xA: return n == !z;
    case 0xB: return n != !z;
    case 0xE: return true;
    default: return false;
    }
}

void ARMv7::set_nz(std::uint32_t result) {
    if (result & 0x80000000u) cpsr_ |= N_FLAG;
    else cpsr_ &= ~N_FLAG;
    if (result == 0) cpsr_ |= Z_FLAG;
    else cpsr_ &= ~Z_FLAG;
}

void ARMv7::step() {
    const auto address = regs_[15];

    if (thumb_) {
        const auto instruction = static_cast<std::uint16_t>(memory_.read32(address) & 0xFFFFu);
        regs_[15] = address + 4;
        execute_thumb(instruction);
    } else {
        const auto instruction = memory_.read32(address);
        // The prototype stores the current fetch address directly in r15.
        // Advance to the next instruction slot before execution so sequential
        // ARM instructions are not skipped by the architectural PC offset.
        regs_[15] = address + 4;
        execute_arm(instruction);
    }
}

void ARMv7::execute_arm(std::uint32_t instruction) {
    const std::uint32_t condition = instruction >> 28;
    if (!condition_passed(condition)) return;

    // BX / BLX register. This is enough to switch between ARM and Thumb code.
    if ((instruction & 0x0FFFFFF0u) == 0x012FFF10u ||
        (instruction & 0x0FFFFFF0u) == 0x012FFF30u) {
        const bool blx = (instruction & 0x20u) != 0;
        const unsigned rm = instruction & 0xF;
        const std::uint32_t target = regs_[rm];
        if (blx) regs_[14] = regs_[15] - 4;
        thumb_ = (target & 1u) != 0;
        regs_[15] = target & ~1u;
        return;
    }

    // B / BL, immediate.
    if ((instruction & 0x0E000000u) == 0x0A000000u) {
        const bool link = (instruction & (1u << 24)) != 0;
        std::int32_t offset = static_cast<std::int32_t>(instruction & 0x00FFFFFFu);
        if (offset & 0x00800000) offset |= static_cast<std::int32_t>(0xFF000000u);
        offset <<= 2;
        if (link) regs_[14] = regs_[15];
        regs_[15] = static_cast<std::uint32_t>(static_cast<std::int32_t>(regs_[15]) + offset);
        return;
    }

    // LDR/STR immediate word.
    if ((instruction & 0x0C000000u) == 0x04000000u) {
        const bool immediate_offset = (instruction & (1u << 25)) == 0;
        const bool pre_index = (instruction & (1u << 24)) != 0;
        const bool up = (instruction & (1u << 23)) != 0;
        const bool byte = (instruction & (1u << 22)) != 0;
        const bool writeback = (instruction & (1u << 21)) != 0;
        const bool load = (instruction & (1u << 20)) != 0;
        const unsigned rn = (instruction >> 16) & 0xF;
        const unsigned rd = (instruction >> 12) & 0xF;
        if (!immediate_offset || byte) throw std::runtime_error("unsupported ARM load/store encoding");
        const std::uint32_t offset = instruction & 0xFFF;
        const std::uint32_t base = regs_[rn];
        const std::uint32_t adjusted = up ? base + offset : base - offset;
        const std::uint32_t address = pre_index ? adjusted : base;
        if (load) regs_[rd] = memory_.read32(address);
        else memory_.write32(address, regs_[rd]);
        if (writeback || !pre_index) regs_[rn] = adjusted;
        return;
    }

    const bool immediate = (instruction & (1u << 25)) != 0;
    const std::uint32_t opcode = (instruction >> 21) & 0xF;
    const bool set_flags = (instruction & (1u << 20)) != 0;
    const unsigned rn = (instruction >> 16) & 0xF;
    const unsigned rd = (instruction >> 12) & 0xF;
    if (!immediate) throw std::runtime_error("register-shifted ARM operands are not implemented yet");

    const unsigned rotate = ((instruction >> 8) & 0xF) * 2;
    const std::uint32_t operand2 = rotate_right(instruction & 0xFF, rotate);
    std::uint32_t result = 0;

    switch (opcode) {
    case 0xD: result = operand2; regs_[rd] = result; break; // MOV
    case 0x4: result = regs_[rn] + operand2; regs_[rd] = result; break; // ADD
    case 0x2: result = regs_[rn] - operand2; regs_[rd] = result; break; // SUB
    case 0xA: result = regs_[rn] - operand2; break; // CMP
    default: throw std::runtime_error("unsupported ARM opcode");
    }
    if (set_flags || opcode == 0xA) set_nz(result);
}

void ARMv7::execute_thumb(std::uint16_t instruction) {
    // Thumb-1 core instructions used by the first prototype.
    

    // MOVS Rd, #imm8
    if ((instruction & 0xF800u) == 0x2000u) {
        const unsigned rd = (instruction >> 8) & 7;
        const std::uint32_t imm = instruction & 0xFF;
        regs_[rd] = imm;
        set_nz(imm);
        return;
    }

    // ADDS Rd, #imm8
    if ((instruction & 0xF800u) == 0x3000u) {
        const unsigned rd = (instruction >> 8) & 7;
        const std::uint32_t imm = instruction & 0xFF;
        regs_[rd] += imm;
        set_nz(regs_[rd]);
        return;
    }

    // SUBS Rd, #imm8
    if ((instruction & 0xF800u) == 0x3800u) {
        const unsigned rd = (instruction >> 8) & 7;
        const std::uint32_t imm = instruction & 0xFF;
        regs_[rd] -= imm;
        set_nz(regs_[rd]);
        return;
    }

    // ADD/SUB register, low registers.
    if ((instruction & 0xFC00u) == 0x1800u) {
        const bool sub = (instruction & 0x0200u) != 0;
        const unsigned rn = (instruction >> 3) & 7;
        const unsigned rd = instruction & 7;
        const unsigned rm = (instruction >> 6) & 7;
        regs_[rd] = sub ? regs_[rn] - regs_[rm] : regs_[rn] + regs_[rm];
        set_nz(regs_[rd]);
        return;
    }

    // Conditional branch.
    if ((instruction & 0xF000u) == 0xD000u && ((instruction >> 8) & 0xF) != 0xF) {
        const std::uint32_t condition = (instruction >> 8) & 0xF;
        std::int32_t offset = static_cast<std::int8_t>(instruction & 0xFF);
        if (condition_passed(condition)) {
            regs_[15] = static_cast<std::uint32_t>(static_cast<std::int32_t>(regs_[15]) + (offset << 1));
        }
        return;
    }

    // Unconditional B.
    if ((instruction & 0xF800u) == 0xE000u) {
        std::int32_t offset = static_cast<std::int32_t>(instruction & 0x7FF);
        if (offset & 0x400) offset |= ~0x7FF;
        regs_[15] = static_cast<std::uint32_t>(static_cast<std::int32_t>(regs_[15]) + (offset << 1));
        return;
    }

    // BX Rs.
    if ((instruction & 0xFF87u) == 0x4700u) {
        const unsigned rm = (instruction >> 3) & 0xF;
        const std::uint32_t target = regs_[rm];
        thumb_ = (target & 1u) != 0;
        regs_[15] = target & ~1u;
        return;
    }

    throw std::runtime_error("unsupported Thumb instruction");
}

} // namespace emu
