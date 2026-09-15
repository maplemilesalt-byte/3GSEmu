#include "cpu/armv7.h"
#include "memory/memory.h"

#include <cstdint>
#include <iostream>

int main() {
    // 16 MiB for the first prototype. This is not yet the 3GS memory map.
    emu::Memory memory(16 * 1024 * 1024);
    emu::ARMv7 cpu(memory);

    // ARM instructions, little-endian:
    //   MOV r0, #42
    //   ADD r1, r0, #8
    //   SUB r2, r1, #10
    memory.write32(0x00, 0xE3A0002A);
    memory.write32(0x04, 0xE2801008);
    memory.write32(0x08, 0xE241200A);

    cpu.reset(0x00);
    cpu.step();
    cpu.step();
    cpu.step();

    std::cout << "3GSEmu CPU prototype\n";
    std::cout << "r0 = " << cpu.reg(0) << '\n';
    std::cout << "r1 = " << cpu.reg(1) << '\n';
    std::cout << "r2 = " << cpu.reg(2) << '\n';
    std::cout << "pc = 0x" << std::hex << cpu.pc() << std::dec << '\n';

    return 0;
}
