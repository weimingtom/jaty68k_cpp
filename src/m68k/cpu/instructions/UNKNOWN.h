#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/Instruction.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.UNKNOWN.
 *
 * Unlike the other instruction classes this one implements Instruction directly
 * (it is the fallback handler for opcodes that are not in the table).
 */
class UNKNOWN : public Instruction {
public:
    explicit UNKNOWN(Cpu& cpu);

    int32_t execute(int32_t opcode) override;

    DisassembledInstruction disassemble(int32_t address, int32_t opcode) override;

protected:
    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
