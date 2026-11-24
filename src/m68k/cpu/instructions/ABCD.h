#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.ABCD.
 */
class ABCD : public InstructionHandler {
public:
    explicit ABCD(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t abcd_dr(int32_t opcode);
    int32_t abcd_ar(int32_t opcode);
    int32_t doCalc(int32_t s, int32_t d);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, bool data_reg_mode);

    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
