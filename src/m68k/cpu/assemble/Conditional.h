#pragma once

namespace m68k::cpu::assemble {

/*
 * C++ port of m68k.cpu.assemble.Conditional (Java enum).
 *
 * The Java enum declared no fields, no constructor and no methods, so the
 * constants keep their declaration order (Java ordinal order) and nothing else
 * is needed.
 * Java: Conditional.NA   ->   C++: Conditional::NA
 */
enum class Conditional {
    NA,
    T,
    F,
    HI,
    LS,
    CC,
    CS,
    NE,
    EQ,
    VC,
    VS,
    PL,
    MI,
    GE,
    LT,
    GT,
    LE
};

} // namespace m68k::cpu::assemble
