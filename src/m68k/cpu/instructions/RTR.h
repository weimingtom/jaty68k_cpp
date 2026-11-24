#pragma once

#include <cstdint>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.RTR.
 */
class RTR : public InstructionHandler {
public:
    explicit RTR(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    Cpu& cpu;
};

} // namespace m68k::cpu::instructions
