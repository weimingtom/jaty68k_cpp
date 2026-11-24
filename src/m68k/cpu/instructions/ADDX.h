#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.ADDX.
 */
class ADDX : public InstructionHandler {
public:
    explicit ADDX(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t addx_byte_reg(int32_t opcode);
    int32_t addx_word_reg(int32_t opcode);
    int32_t addx_long_reg(int32_t opcode);
    int32_t addx_byte_mem(int32_t opcode);
    int32_t addx_word_mem(int32_t opcode);
    int32_t addx_long_mem(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, Size sz);

    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
