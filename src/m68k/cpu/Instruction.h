#pragma once

#include <cstdint>

#include "m68k/cpu/DisassembledInstruction.h"

namespace m68k::cpu {

/*
 * C++ port of m68k.cpu.Instruction (Java interface).
 */
class Instruction {
public:
    virtual ~Instruction() = default;

    virtual int32_t execute(int32_t opcode) = 0;
    virtual DisassembledInstruction disassemble(int32_t address, int32_t opcode) = 0;
};

} // namespace m68k::cpu
