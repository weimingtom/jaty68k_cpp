#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.SUBX.
 */
class SUBX : public InstructionHandler {
public:
    explicit SUBX(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t subx_byte_reg(int32_t opcode);
    int32_t subx_word_reg(int32_t opcode);
    int32_t subx_long_reg(int32_t opcode);
    int32_t subx_byte_mem(int32_t opcode);
    int32_t subx_word_mem(int32_t opcode);
    int32_t subx_long_mem(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, Size sz);

    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
