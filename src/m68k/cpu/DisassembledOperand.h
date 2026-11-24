#pragma once

#include <cstdint>
#include <string>

namespace m68k::cpu {

/*
 * C++ port of m68k.cpu.DisassembledOperand.
 * Java "public final" fields are modelled as public const fields.
 */
class DisassembledOperand {
public:
    const int32_t bytes;
    const int32_t memory_read;
    const std::string operand;

    explicit DisassembledOperand(const std::string& operand);

    DisassembledOperand(const std::string& operand, int32_t bytes, int32_t memory_read);
};

} // namespace m68k::cpu
