#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.EXG.
 */
class EXG : public InstructionHandler {
public:
    explicit EXG(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    enum class ExgMode { EXG_DATA, EXG_ADDR, EXG_DATA_ADDR };

    int32_t exg_dd(int32_t opcode);
    int32_t exg_aa(int32_t opcode);
    int32_t exg_da(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, ExgMode mode);

    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
