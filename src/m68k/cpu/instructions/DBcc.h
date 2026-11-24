#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.DBcc.
 */
class DBcc : public InstructionHandler {
public:
    explicit DBcc(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t dbxx(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode);

    Cpu& cpu;

    static const std::array<std::string, 16> names;
};

} // namespace m68k::cpu::instructions
