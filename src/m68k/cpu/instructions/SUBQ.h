#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.SUBQ.
 */
class SUBQ : public InstructionHandler {
public:
    explicit SUBQ(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t subq_byte(int32_t opcode);
    int32_t subq_word(int32_t opcode);
    int32_t subq_long(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, Size sz);

    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
