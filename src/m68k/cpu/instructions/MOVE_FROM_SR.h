#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.MOVE_FROM_SR.
 */
class MOVE_FROM_SR : public InstructionHandler {
public:
    explicit MOVE_FROM_SR(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t move_from_sr(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, Size sz);

    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
