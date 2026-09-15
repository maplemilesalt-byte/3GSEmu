#pragma once

#include <cstdint>

namespace emu {

class Memory;

class ARMv7 {
public:
    explicit ARMv7(Memory& memory);

    void reset(std::uint32_t entry = 0);
    void step();

    std::uint32_t reg(unsigned index) const;
    std::uint32_t pc() const { return regs_[15]; }
    std::uint32_t cpsr() const { return cpsr_; }

private:
    bool condition_passed(std::uint32_t condition) const;
    void set_nz(std::uint32_t result);
    void execute_arm(std::uint32_t instruction);

    Memory& memory_;
    std::uint32_t regs_[16]{};
    std::uint32_t cpsr_{};
};

} // namespace emu
