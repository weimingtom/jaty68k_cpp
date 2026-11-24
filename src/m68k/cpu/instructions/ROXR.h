#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.ROXR.
 */
class ROXR : public InstructionHandler {
public:
    explicit ROXR(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t roxr_byte_imm(int32_t opcode);
    int32_t roxr_word_imm(int32_t opcode);
    int32_t roxr_long_imm(int32_t opcode);
    int32_t roxr_byte_reg(int32_t opcode);
    int32_t roxr_word_reg(int32_t opcode);
    int32_t roxr_long_reg(int32_t opcode);
    int32_t roxr_word_mem(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, Size sz);

    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
