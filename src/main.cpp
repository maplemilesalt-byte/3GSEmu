#include "cpu/armv7.h"
#include "memory/memory.h"

#include <iostream>

int main() {
    emu::Memory memory(16 * 1024 * 1024);
    emu::ARMv7 cpu(memory);

    // ARM -> Thumb smoke test.
    // ARM:
    //   MOV r0, #42
    //   MOV r3, #0x21
    //   BX  r3          ; enters Thumb at 0x20
    memory.write32(0x00, 0xE3A0002A);
    memory.write32(0x04, 0xE3A03021);
    memory.write32(0x08, 0xE12FFF13);

    // Thumb at 0x20:
    //   MOVS r1, #10
    //   ADDS r1, #5
    //   SUBS r1, #3
    //   ADDS r1, r0     ; r1 = 42 + 12
    memory.write16(0x20, 0x210A);
    memory.write16(0x22, 0x3105);
    memory.write16(0x24, 0x3903);
    memory.write16(0x26, 0x1809);

    cpu.reset(0x00);
    cpu.step();
    cpu.step();
    cpu.step();
    cpu.step();
    cpu.step();
    cpu.step();
    cpu.step();

    std::cout << "3GSEmu CPU prototype\n";
    std::cout << "r0 = " << cpu.reg(0) << '\n';
    std::cout << "r1 = " << cpu.reg(1) << '\n';
    std::cout << "Thumb = " << std::boolalpha << cpu.thumb() << '\n';
    std::cout << "pc = 0x" << std::hex << cpu.pc() << std::dec << '\n';

    return 0;
}
