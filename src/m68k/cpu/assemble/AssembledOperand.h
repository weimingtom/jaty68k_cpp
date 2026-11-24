#pragma once

#include <cstdint>
#include <string>

#include "m68k/cpu/DisassembledOperand.h"
#include "m68k/cpu/Size.h"
#include "m68k/cpu/assemble/AddressingMode.h"
#include "m68k/cpu/assemble/Conditional.h"

namespace m68k::cpu::assemble {

/*
 * C++ port of m68k.cpu.assemble.AssembledOperand.
 *
 * Note: the Java field "register" cannot keep its name because "register" is a
 * reserved keyword in C++ - it is called "register_" here (the same name is
 * used for the corresponding constructor parameters).
 */
class AssembledOperand : public DisassembledOperand {
public:
    const AddressingMode mode;
    const int32_t register_;

    const int32_t ext_register;
    const int32_t ext_data;
    const Size ext_size;

    const Conditional conditional;

    std::string toString() const;

    AssembledOperand(const std::string& operand);

    AssembledOperand(const std::string& operand, int32_t bytes, int32_t memory_read);

    AssembledOperand(const std::string& operand, int32_t bytes, int32_t memory_read, AddressingMode mode,
                     Conditional conditional, int32_t register_);

    AssembledOperand(const std::string& operand, int32_t bytes, int32_t memory_read, AddressingMode mode,
                     Conditional conditional, int32_t register_, int32_t ext_register, int32_t ext_data,
                     Size ext_size);
};

} // namespace m68k::cpu::assemble
