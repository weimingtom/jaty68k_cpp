#pragma once

#include <cstdint>
#include <string>

#include "m68k/cpu/Size.h"

namespace m68k::cpu::assemble {

/*
 * C++ port of m68k.cpu.assemble.LabelUsage.
 */
class LabelUsage {
public:
    std::string name;
    int32_t address;
    Size size;
    bool relative;

    LabelUsage(const std::string& name, int32_t address, Size size, bool relative);
};

} // namespace m68k::cpu::assemble
