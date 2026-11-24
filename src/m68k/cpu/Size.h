#pragma once

#include <cstdint>

namespace m68k::cpu {

/*
 * C++ port of m68k.cpu.Size (Java enum).
 *
 * The Java enum had the constants Byte/Word/Long/Unsized carrying a byte count,
 * a mnemonic extension and a most-significant-bit mask.  Here the constants are
 * an "enum class" and the former accessors are free functions, so Java's
 *      Size.Byte        -> Size::Byte
 *      sz.byteCount()   -> byteCount(sz)
 *      sz.ext()         -> ext(sz)
 *      sz.msb()         -> msb(sz)
 */
enum class Size {
    Byte    = 0,
    Word    = 1,
    Long    = 2,
    Unsized = 3
};

constexpr int32_t byteCount(Size sz) {
    switch (sz) {
        case Size::Byte:    return 1;
        case Size::Word:    return 2;
        case Size::Long:    return 4;
        case Size::Unsized: return 0;
    }
    return 0;
}

constexpr const char* ext(Size sz) {
    switch (sz) {
        case Size::Byte:    return ".b";
        case Size::Word:    return ".w";
        case Size::Long:    return ".l";
        case Size::Unsized: return "";
    }
    return "";
}

constexpr int32_t msb(Size sz) {
    switch (sz) {
        case Size::Byte:    return 0x80;
        case Size::Word:    return 0x8000;
        case Size::Long:    return static_cast<int32_t>(0x80000000u);
        case Size::Unsized: return 0;
    }
    return 0;
}

} // namespace m68k::cpu
