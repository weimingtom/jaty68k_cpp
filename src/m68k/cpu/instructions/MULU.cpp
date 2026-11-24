#include "m68k/cpu/instructions/MULU.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

MULU::MULU(Cpu& cpu) : cpu(cpu) {}

void MULU::registerInstructions(InstructionSet& is) {
    int32_t base = 0xc0c0;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return mulu(opcode); },
        [this](int32_t address, int32_t opcode) { return disassembleOp(address, opcode, Size::Word); });

    for (int32_t ea_mode = 0; ea_mode < 8; ea_mode++) {
        if (ea_mode == 1) {
            continue;
        }

        for (int32_t ea_reg = 0; ea_reg < 8; ea_reg++) {
            if (ea_mode == 7 && ea_reg > 4) {
                break;
            }

            for (int32_t r = 0; r < 8; r++) {
                is.addInstruction(base + (r << 9) + (ea_mode << 3) + ea_reg, i);
            }
        }
    }
}

int32_t MULU::mulu(int32_t opcode) {
    Operand* op = cpu.resolveSrcEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Word);

    const int32_t s = op->getWord();
    const int32_t reg = (opcode >> 9) & 0x07;
    // mulu for the 68008,68000,68010 only uses the lower word of the reg
    const int32_t d = cpu.getDataRegisterWord(reg);

    const int32_t r = s * d;

    if (r < 0) {
        cpu.setFlags(N_FLAG);
    } else {
        cpu.clrFlags(N_FLAG);
    }
    if (r == 0) {
        cpu.setFlags(Z_FLAG);
    } else {
        cpu.clrFlags(Z_FLAG);
    }

    cpu.clrFlags((V_FLAG | C_FLAG));

    cpu.setDataRegisterLong(reg, r);

    int32_t x = s;
    x = (x & 0x5555) + ((x >> 1) & 0x5555);
    x = (x & 0x3333) + ((x >> 2) & 0x3333);
    x = (x & 0x0f0f) + ((x >> 4) & 0x0f0f);
    x = (x & 0x00ff) + ((x >> 8) & 0x00ff);

    return 38 + (x << 1);
}

DisassembledInstruction MULU::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src = cpu.disassembleSrcEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    DisassembledOperand dst("d" + std::to_string((opcode >> 9) & 0x07));

    return DisassembledInstruction(address, opcode, "mulu", src, dst);
}

} // namespace m68k::cpu::instructions
