#pragma once

#include "m68k/cpu/InstructionSet.h"

namespace m68k::cpu {

/*
 * C++ port of m68k.cpu.InstructionHandler (Java interface).
 *
 * Note: the Java method is named register(InstructionSet), which cannot be kept
 * because "register" is a C++ keyword - it is renamed to registerInstructions().
 */
class InstructionHandler {
public:
    virtual ~InstructionHandler() = default;

    virtual void registerInstructions(InstructionSet& is) = 0;
};

} // namespace m68k::cpu
