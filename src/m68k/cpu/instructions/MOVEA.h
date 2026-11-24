#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.MOVEA.
 *
 * The execution timing tables live in MOVE (MOVE.ShortExecutionTime /
 * MOVE.LongExecutionTime in Java) and are accessed from there, exactly as in
 * the original - see MOVE.h.
 */
class MOVEA : public InstructionHandler {
public:
    explicit MOVEA(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t movea_word(int32_t opcode);
    int32_t movea_long(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, Size sz);

    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
