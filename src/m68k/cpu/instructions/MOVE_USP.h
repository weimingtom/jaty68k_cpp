#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.MOVE_USP.
 */
class MOVE_USP : public InstructionHandler {
public:
    explicit MOVE_USP(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t move_to_usp(int32_t opcode);
    int32_t move_from_usp(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, bool move_to_usp);

    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
