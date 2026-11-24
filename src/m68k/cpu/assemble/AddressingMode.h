#pragma once

#include <cstdint>

namespace m68k::cpu::assemble {

/*
 * C++ port of m68k.cpu.assemble.AddressingMode (Java enum).
 *
 * Every Java constant carried two ints (requiredOperants and bits).  As with
 * m68k::cpu::Size the constants become an "enum class" and the former accessors
 * become free functions, so
 *      AddressingMode.IMMEDIATE      -> AddressingMode::IMMEDIATE
 *      mode.getRequiredOperants()    -> getRequiredOperants(mode)
 *      mode.bits()                   -> bits(mode)
 *
 * The underlying values are the Java declaration ordinals, so the order of the
 * constants below is exactly the order of the Java enum.
 */
enum class AddressingMode {
    NA = 0,
    IMMEDIATE = 1,
    IMMEDIATE_DATA = 2,
    IMMEDIATE_ADDRESS = 3,
    INDIRECT = 4,
    INDIRECT_POST = 5,
    INDIRECT_PRE = 6,
    INDIRECT_DISP = 7,
    INDIRECT_INDEX = 8,
    ABSOLUTE_NEAR = 9,
    ABSOLUTE_FAR = 10,
    PC_DISP = 11,
    PC_INDEX = 12,
    SR = 13,
    CCR = 14,
    REGISTER_LIST = 15,
    LABEL = 16
};

int32_t getRequiredOperants(AddressingMode mode);

int32_t bits(AddressingMode mode);

} // namespace m68k::cpu::assemble
