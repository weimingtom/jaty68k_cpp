#pragma once

#include <cstdint>

namespace m68k::cpu {

/*
 * C++ port of m68k.cpu.CpuUtils.
 * Java static methods -> inline free functions in m68k::cpu.
 */
inline int32_t signExtendByte(int32_t value) {
    if ((value & 0x80) == 0x80) {
        value |= static_cast<int32_t>(0xffffff00u);
    } else {
        value &= 0x000000ff;
    }
    return value;
}

inline int32_t signExtendWord(int32_t value) {
    if ((value & 0x8000) == 0x8000) {
        value |= static_cast<int32_t>(0xffff0000u);
    } else {
        value &= 0x0000ffff;
    }
    return value;
}

} // namespace m68k::cpu
