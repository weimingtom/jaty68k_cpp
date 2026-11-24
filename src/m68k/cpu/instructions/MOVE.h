#pragma once

#include <array>
#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.MOVE.
 */
class MOVE : public InstructionHandler {
public:
    explicit MOVE(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

    // In Java these two tables are package-private ("static final int[][]"), and
    // MOVEA reads them directly (MOVEA does not extend MOVE). C++ has no
    // package-private scope, so they are public here - same reachable set.
    static const std::array<std::array<int32_t, 9>, 12> ShortExecutionTime;
    static const std::array<std::array<int32_t, 9>, 12> LongExecutionTime;

protected:
    int32_t move_byte(int32_t opcode);
    int32_t move_word(int32_t opcode);
    int32_t move_long(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, Size sz);

    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
