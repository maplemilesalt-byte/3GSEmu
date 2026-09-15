#include "cpu/armv7.h"

#include "memory/memory.h"

#include <stdexcept>

namespace emu {

ARMv7::ARMv7(Memory& memory) : memory_(memory) {
    reset();
}

void ARMv7::reset(std::uint32_t entry) {
    for (auto& reg : regs_) {
        reg = 0;
    }

    cpsr_ = 0;
    regs_[15] = entry;
}

std::uint32_t ARMv7::reg(unsigned index) const {
    if (index >= 16) {
        throw std::out_of_range("ARM register index out of range");
    }
    return regs_[index];
}

void ARMv7::step() {
    const auto instruction_address = regs_[15];
    const auto instruction = memory_.read32(instruction_address);

    // ARM state has a visible PC value of current instruction address + 8.
    regs_[15] = instruction_address + 8;
    execute_arm(instruction);
}

void ARMv7::execute_arm(std::uint32_t instruction) {
    // First instruction family: data-processing operations with an immediate.
    // This is deliberately small. We are building the interpreter incrementally.
    const std::uint32_t condition = instruction >> 28;
    if (condition != 0xE) {
        throw std::runtime_error("conditional ARM instructions are not implemented yet");
    }

    const bool immediate = (instruction & (1u << 25)) != 0;
    const std::uint32_t opcode = (instruction >> 21) & 0xF;
    const bool set_flags = (instruction & (1u << 20)) != 0;
    const unsigned rn = (instruction >> 16) & 0xF;
    const unsigned rd = (instruction >> 12) & 0xF;

    if (!immediate || set_flags) {
        throw std::runtime_error("unsupported ARM data-processing encoding");
    }

    const std::uint32_t operand2 = instruction & 0xFFF;

    switch (opcode) {
    case 0xD: // MOV Rd, #imm
        regs_[rd] = operand2;
        break;
    case 0x4: // ADD Rd, Rn, #imm
        regs_[rd] = regs_[rn] + operand2;
        break;
    case 0x2: // SUB Rd, Rn, #imm
        regs_[rd] = regs_[rn] - operand2;
        break;
    default:
        throw std::runtime_error("unsupported ARM opcode");
    }
}

} // namespace emu
