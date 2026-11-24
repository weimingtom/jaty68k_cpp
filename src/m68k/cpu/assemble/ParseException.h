#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

namespace m68k::cpu::assemble {

/*
 * Stand-in for java.text.ParseException, which the Java assembler threw as
 *   throw new ParseException(message, lineNumber);
 *
 * Java's ParseException carries an error offset; here it carries the source line
 * number that the assembler passed in, exposed through the same
 * getErrorOffset() accessor plus a clearer lineNumber().
 */
class ParseException : public std::runtime_error {
public:
    ParseException(const std::string& message, int32_t lineNumber)
        : std::runtime_error(message), lineNumber_(lineNumber) {}

    explicit ParseException(const std::string& message) : std::runtime_error(message), lineNumber_(0) {}

    int32_t getErrorOffset() const { return lineNumber_; }

    int32_t lineNumber() const { return lineNumber_; }

private:
    int32_t lineNumber_;
};

} // namespace m68k::cpu::assemble
