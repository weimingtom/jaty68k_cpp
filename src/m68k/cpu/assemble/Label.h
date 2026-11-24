#pragma once

#include <cstdint>
#include <string>

namespace m68k::cpu::assemble {

/*
 * C++ port of m68k.cpu.assemble.Label.
 *
 * The Java fields carry no access modifier (package private) and there is no
 * constructor and no method.  C++ has no package-private access, so the fields
 * stay public, as the port also does for DisassembledOperand's fields.
 *
 * The Java compiler zero-initialises the int fields, so they carry the same
 * initialisers here; Java's null String name becomes an empty std::string,
 * which is the only String value that maps to std::string.
 */
class Label {
public:
    std::string name;
    int32_t location = 0;
    int32_t address = 0;
};

} // namespace m68k::cpu::assemble
