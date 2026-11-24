#pragma once

#include <cstdint>
#include <string>

#include "m68k/cpu/assemble/AssembledOperand.h"

namespace m68k::cpu::assemble {

/*
 * C++ port of m68k.cpu.assemble.LabelOperand.
 */
class LabelOperand : public AssembledOperand {
public:
    LabelOperand(int32_t pc, const std::string& operand);
};

} // namespace m68k::cpu::assemble
