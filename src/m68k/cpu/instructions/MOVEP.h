#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.MOVEP.
 */
class MOVEP : public InstructionHandler {
public:
    explicit MOVEP(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t r2m_word(int32_t opcode);
    int32_t r2m_long(int32_t opcode);
    int32_t m2r_word(int32_t opcode);
    int32_t m2r_long(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, Size sz, bool r2m);

    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
