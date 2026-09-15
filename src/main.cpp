#include "cpu/armv7.h"
#include "memory/memory.h"

#include <iostream>

int main() {
    // 16 MiB for the first prototype. This is not yet the real 3GS memory map.
    emu::Memory memory(16 * 1024 * 1024);
    emu::ARMv7 cpu(memory);

    // Small ARM program used as a CPU smoke test.
    //
    //   MOV r0, #42
    //   ADD r1, r0, #8
    //   SUB r2, r1, #10
    //   MOV r4, #0x100
    //   MOV r5, #123
    //   STR r5, [r4]
    //   LDR r6, [r4]
    //   CMP r6, #123
    //   BEQ +1
    //   MOV r7, #0xDEAD       ; skipped when comparison succeeds
    //   MOV r7, #0xBEEF
    memory.write32(0x00, 0xE3A0002A);
    memory.write32(0x04, 0xE2801008);
    memory.write32(0x08, 0xE241200A);
    memory.write32(0x0C, 0xE3A04C01);
    memory.write32(0x10, 0xE3A0507B);
    memory.write32(0x14, 0xE5845000);
    memory.write32(0x18, 0xE5946000);
    memory.write32(0x1C, 0xE356007B);
    memory.write32(0x20, 0x0A000001);
    memory.write32(0x24, 0xE3A07EAD);
    memory.write32(0x28, 0xE3A07C0B);

    cpu.reset(0x00);
    for (int i = 0; i < 11; ++i) {
        cpu.step();
    }

    std::cout << "3GSEmu CPU prototype\n";
    std::cout << "r0 = " << cpu.reg(0) << '\n';
    std::cout << "r1 = " << cpu.reg(1) << '\n';
    std::cout << "r2 = " << cpu.reg(2) << '\n';
    std::cout << "r5 = " << cpu.reg(5) << '\n';
    std::cout << "r6 = " << cpu.reg(6) << '\n';
    std::cout << "r7 = 0x" << std::hex << cpu.reg(7) << std::dec << '\n';
    std::cout << "mem[0x100] = " << memory.read32(0x100) << '\n';
    std::cout << "pc = 0x" << std::hex << cpu.pc() << std::dec << '\n';

    return 0;
}
