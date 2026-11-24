#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.OR.
 */
class OR : public InstructionHandler {
public:
    explicit OR(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t or_byte_dn_dest(int32_t opcode);
    int32_t or_byte_ea_dest(int32_t opcode);
    int32_t or_word_dn_dest(int32_t opcode);
    int32_t or_word_ea_dest(int32_t opcode);
    int32_t or_long_dn_dest(int32_t opcode);
    int32_t or_long_ea_dest(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, Size sz);

    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
