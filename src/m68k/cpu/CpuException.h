#pragma once

#include <stdexcept>
#include <string>

namespace m68k::cpu {

/*
 * C++ port of m68k.cpu.CpuException (Java RuntimeException subclass).
 * Object slicing is not a concern here, so the "Throwable cause" constructors
 * of the Java version collapse into the std::runtime_error ones.
 */
class CpuException : public std::runtime_error {
public:
    CpuException() : std::runtime_error("CpuException") {}

    explicit CpuException(const std::string& message) : std::runtime_error(message) {}

    explicit CpuException(const char* message) : std::runtime_error(message) {}
};

} // namespace m68k::cpu
