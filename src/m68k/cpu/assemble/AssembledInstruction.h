#pragma once

#include <string>

#include "m68k/cpu/DisassembledInstruction.h"
#include "m68k/cpu/Size.h"
#include "m68k/cpu/assemble/AssembledOperand.h"

namespace m68k::cpu::assemble {

/*
 * C++ port of m68k.cpu.assemble.AssembledInstruction.
 *
 * The Java subclass adds a "size" field that shadows DisassembledInstruction's
 * size() method; the C++ port keeps it as a data member, which hides the
 * inherited size() in the same way.
 */
class AssembledInstruction : public DisassembledInstruction {
public:
    const Size size;

    // two operands
    AssembledInstruction(const std::string& instruction, Size size, const AssembledOperand& aop1,
                         const AssembledOperand& aop2);

    std::string toString() const;
};

} // namespace m68k::cpu::assemble
