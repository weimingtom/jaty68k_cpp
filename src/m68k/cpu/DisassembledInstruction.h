#pragma once

#include <cstdint>
#include <string>

#include "m68k/cpu/DisassembledOperand.h"

namespace m68k::cpu {

/*
 * C++ port of m68k.cpu.DisassembledInstruction.
 * The three Java constructors (0, 1 and 2 operands) are kept as overloads.
 */
class DisassembledInstruction {
public:
    const int32_t address;
    const int32_t opcode;
    const int32_t num_operands;
    const std::string instruction;

    const DisassembledOperand op1;
    const DisassembledOperand op2;

    // no operands
    DisassembledInstruction(int32_t address, int32_t opcode, const std::string& instruction);

    // one operand
    DisassembledInstruction(int32_t address, int32_t opcode, const std::string& instruction,
                            const DisassembledOperand& dop);

    // two operands
    DisassembledInstruction(int32_t address, int32_t opcode, const std::string& instruction,
                            const DisassembledOperand& dop1, const DisassembledOperand& dop2);

    int32_t size() const;

    void shortFormat(std::string& buffer) const;

    void formatInstruction(std::string& buffer) const;

    std::string toString() const;
};

} // namespace m68k::cpu
