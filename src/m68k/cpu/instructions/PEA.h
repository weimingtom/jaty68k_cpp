#pragma once

#include <array>
#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.PEA.
 */
class PEA : public InstructionHandler {
public:
    explicit PEA(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t pea(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, Size sz);

    Cpu& cpu;

    static const std::array<int32_t, 11> TIMING;
};

} // namespace m68k::cpu::instructions
