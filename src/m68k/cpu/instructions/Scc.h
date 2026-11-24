#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.Scc.
 */
class Scc : public InstructionHandler {
public:
    explicit Scc(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t sxx(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode);

    Cpu& cpu;

    static const std::array<std::string, 16> names;
};

} // namespace m68k::cpu::instructions
