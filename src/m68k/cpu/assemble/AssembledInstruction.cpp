#include "m68k/cpu/assemble/AssembledInstruction.h"

#include <string>

namespace m68k::cpu::assemble {

// two operands
AssembledInstruction::AssembledInstruction(const std::string& instruction, Size size,
                                           const AssembledOperand& aop1, const AssembledOperand& aop2)
    : DisassembledInstruction(0, 0, instruction, aop1, aop2), size(size) {}

std::string AssembledInstruction::toString() const {
    std::string buffer;
    buffer.reserve(80);
    formatInstruction(buffer);
    return buffer;
}

} // namespace m68k::cpu::assemble
