#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.ADDI.
 */
class ADDI : public InstructionHandler {
public:
    explicit ADDI(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t addi_byte(int32_t opcode);
    int32_t addi_word(int32_t opcode);
    int32_t addi_long(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, Size sz);

    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
