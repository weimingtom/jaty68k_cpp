#include "m68k/cpu/assemble/LabelUsage.h"

namespace m68k::cpu::assemble {

LabelUsage::LabelUsage(const std::string& name, int32_t address, Size size, bool relative)
    : name(name), address(address), size(size), relative(relative) {}

} // namespace m68k::cpu::assemble
