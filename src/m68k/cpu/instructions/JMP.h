#pragma once

#include <array>
#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.JMP.
 */
class JMP : public InstructionHandler {
public:
    explicit JMP(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t jmp(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, Size sz);

    Cpu& cpu;

    static const std::array<int32_t, 11> TIMING;
};

} // namespace m68k::cpu::instructions
