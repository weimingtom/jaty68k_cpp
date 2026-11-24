#pragma once

#include <cstdint>
#include <memory>

#include "m68k/cpu/Instruction.h"

namespace m68k::cpu {

/*
 * C++ port of m68k.cpu.InstructionSet (Java interface).
 *
 * Shared ownership replaces the Java garbage collector's role: the same
 * Instruction instance is registered under many opcodes, exactly as in the
 * original, but the table now owns it.
 */
class InstructionSet {
public:
    virtual ~InstructionSet() = default;

    virtual void addInstruction(int32_t opcode, std::shared_ptr<Instruction> i) = 0;
};

} // namespace m68k::cpu
