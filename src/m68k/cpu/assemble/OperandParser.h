#pragma once

#include <cstdint>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "m68k/cpu/Size.h"
#include "m68k/cpu/assemble/AddressingMode.h"
#include "m68k/cpu/assemble/AssembledOperand.h"
#include "m68k/cpu/assemble/Conditional.h"
#include "m68k/cpu/assemble/Labels.h"

namespace m68k::cpu::assemble {

/*
 * C++ port of m68k.cpu.assemble.OperandParser.
 *
 * Notes on the conversion:
 *  - the Java field "parts" and the Java method "parts()" cannot share a name in
 *    C++, so the accessor is called getParts(); the field keeps its Java name
 *    because every statement of parse() uses it.
 *  - the Java local variable "register" cannot keep its name because "register"
 *    is a C++ keyword; it is called "register_" (as in AssembledOperand.h), and
 *    the Java local "or" is called "orMask" for the same reason.
 *  - Java's "throws ParseException" has no C++ equivalent; parse() throws
 *    m68k::cpu::assemble::ParseException (m68k/cpu/assemble/ParseException.h).
 */
class OperandParser {
private:
    static std::regex labelPattern;

    static std::unordered_map<std::string, AddressingMode> modeMapping;

    /*
     * Operand types:
     * v - value
     * a - address register
     * i - indirect address register
     * d - data register
     * p - pc register
     * s - sr register
     * c - ccr register
     * w - word
     * l - long
     * + - post increment
     * - - pre increment
     * b - could be label
     */
    class Part {
    public:
        char type = ' ';
        std::string text;
    };

    std::vector<Part> parts;
    Part current;

    void clear();
    void newPart();
    void setType(char t);

    bool couldBeLabel(const std::string& value);

public:
    // Java's instance initialiser block fills the static modeMapping here.
    OperandParser();

    AssembledOperand parse(Labels& labels, Size size, int32_t pc, int32_t lineNumber, const std::string& operand);

    int32_t parseValue(const std::string& value);

    std::string descriptor();

    std::vector<Part> getParts();
};

} // namespace m68k::cpu::assemble
