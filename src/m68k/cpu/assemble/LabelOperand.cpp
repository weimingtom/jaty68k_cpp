#include "m68k/cpu/assemble/LabelOperand.h"

namespace m68k::cpu::assemble {

LabelOperand::LabelOperand(int32_t pc, const std::string& operand)
    : AssembledOperand(operand, 0, pc) {}

} // namespace m68k::cpu::assemble
