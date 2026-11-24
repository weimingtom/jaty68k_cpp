#include "m68k/cpu/DisassembledOperand.h"

#include <stdexcept>

namespace m68k::cpu {

DisassembledOperand::DisassembledOperand(const std::string& operand)
    : bytes(0), memory_read(0), operand(operand) {}

DisassembledOperand::DisassembledOperand(const std::string& operand, int32_t bytes, int32_t memory_read)
    : bytes(bytes), memory_read(memory_read), operand(operand) {
    if (bytes > 8) {
        throw std::invalid_argument("Are these the wrong way around ?");
    }
}

} // namespace m68k::cpu
