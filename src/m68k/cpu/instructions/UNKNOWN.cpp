#include "m68k/cpu/instructions/UNKNOWN.h"

namespace m68k::cpu::instructions {

UNKNOWN::UNKNOWN(Cpu& cpu) : cpu(cpu) {}

int32_t UNKNOWN::execute(int32_t opcode) {
    // illegal instruction
    int32_t vector;
    if ((opcode & 0xf000) == 0xa000) {
        // Line 1010 emulator
        vector = 10;
    } else if ((opcode & 0xf000) == 0xf000) {
        // Line 1111 emulator
        vector = 11;
    } else {
        // illegal instruction
        vector = 4;
    }

    cpu.raiseException(vector);
    return 34;
}

DisassembledInstruction UNKNOWN::disassemble(int32_t address, int32_t opcode) {
    // allow disassembly
    return DisassembledInstruction(address, opcode, "????");
}

} // namespace m68k::cpu::instructions
