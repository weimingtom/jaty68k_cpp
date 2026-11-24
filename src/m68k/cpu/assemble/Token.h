#pragma once

namespace m68k::cpu::assemble {

/*
 * C++ port of m68k.cpu.assemble.Token (Java enum).
 *
 * The Java enum declared one constant and nothing else, so the constants keep
 * their declaration order (Java ordinal order) and no accessors are needed.
 * Java: Token.MOVE   ->   C++: Token::MOVE
 */
enum class Token {
    MOVE
};

} // namespace m68k::cpu::assemble
