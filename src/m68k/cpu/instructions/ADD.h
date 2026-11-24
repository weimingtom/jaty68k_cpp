#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.ADD.
 */
class ADD : public InstructionHandler {
public:
    explicit ADD(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t add_byte_dn_dest(int32_t opcode);
    int32_t add_byte_ea_dest(int32_t opcode);
    int32_t add_word_dn_dest(int32_t opcode);
    int32_t add_word_ea_dest(int32_t opcode);
    int32_t add_long_dn_dest(int32_t opcode);
    int32_t add_long_ea_dest(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, Size sz);

    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
