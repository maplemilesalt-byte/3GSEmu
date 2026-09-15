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

private:
    void execute_arm(std::uint32_t instruction);

    Memory& memory_;
    std::uint32_t regs_[16]{};
    std::uint32_t cpsr_{};
};

} // namespace emu
