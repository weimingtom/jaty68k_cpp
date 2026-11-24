#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::instructions {

/*
 * C++ port of m68k.cpu.instructions.MOVEM.
 */

// when the ea is specified by a pre-decreasing mode (eg movem.l range,-(an) , then, if the addressing register
// itself is part of the range and for a 68000 and 68010, the value stored is the INITIAL value of the addressing reg.
// For a 68020 and up, the value written is the initial value minus the size (word or long) of the operation.

class MOVEM : public InstructionHandler {
public:
    explicit MOVEM(Cpu& cpu);

    void registerInstructions(InstructionSet& is) override;

protected:
    int32_t movem_word_r2m(int32_t opcode);
    int32_t movem_long_r2m(int32_t opcode);
    int32_t movem_word_m2r(int32_t opcode);
    int32_t movem_long_m2r(int32_t opcode);

    DisassembledInstruction disassembleOp(int32_t address, int32_t opcode, Size sz, bool reg_to_mem);

    std::string regListToString(int32_t reglist, bool reversed);

    int32_t getMultipleWord(int32_t reglist, int32_t address);
    int32_t getMultipleWordPostInc(int32_t reg, int32_t reglist, int32_t address);
    int32_t getMultipleLong(int32_t reglist, int32_t address);
    int32_t getMultipleLongPostInc(int32_t reg, int32_t reglist, int32_t address);

    int32_t putMultipleWord(int32_t reglist, int32_t address);
    int32_t putMultipleWordPreDec(int32_t reg, int32_t reglist, int32_t address);
    int32_t putMultipleLong(int32_t reglist, int32_t address);
    int32_t putMultipleLongPreDec(int32_t reg, int32_t reglist, int32_t address);

    Cpu& cpu;

private:
    static constexpr std::array<int32_t, 11> M2R_Timing = {
        0, 0, 12, 12, 0, 16, 18, 16, 20, 16, 18
    };
    static constexpr std::array<int32_t, 9> R2M_Timing = {
        0, 0, 8, 0, 8, 12, 14, 12, 16
    };
};

} // namespace m68k::cpu::instructions
