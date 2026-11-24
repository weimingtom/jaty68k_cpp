#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.AND.
 */
class AND : public InstructionHandler {
public:
    explicit AND(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t and_byte_dn_dest(int32_t opcode);
    int32_t and_byte_ea_dest(int32_t opcode);
    int32_t and_word_dn_dest(int32_t opcode);
    int32_t and_word_ea_dest(int32_t opcode);
    int32_t and_long_dn_dest(int32_t opcode);
    int32_t and_long_ea_dest(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, Size sz);

    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
