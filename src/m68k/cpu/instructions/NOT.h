#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.NOT.
 */
class NOT : public InstructionHandler {
public:
    explicit NOT(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t not_byte(int32_t opcode);
    int32_t not_word(int32_t opcode);
    int32_t not_long(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, Size sz);

    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
