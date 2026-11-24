#include "m68k/cpu/instructions/NBCD.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

NBCD::NBCD(Cpu& cpu) : cpu(cpu) {}

void NBCD::registerInstructions(InstructionSet& is) {
    int32_t base = 0x4800;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return nbcd(opcode); },
        [this](int32_t address, int32_t opcode) { return disassembleOp(address, opcode, Size::Byte); });

    for (int32_t ea_mode = 0; ea_mode < 8; ea_mode++) {
        if (ea_mode == 1) {
            continue;
        }

        for (int32_t ea_reg = 0; ea_reg < 8; ea_reg++) {
            if (ea_mode == 7 && ea_reg > 1) {
                break;
            }
            is.addInstruction(base + (ea_mode << 3) + ea_reg, i);
        }
    }
}

int32_t NBCD::nbcd(int32_t opcode) {
    const int32_t mode = (opcode >> 3) & 0x07;
    const int32_t reg = (opcode & 0x07);
    Operand* op = cpu.resolveDstEA(mode, reg, Size::Byte);
    const int32_t s = op->getByte();

    const int32_t x = (cpu.isFlagSet(X_FLAG) ? 1 : 0);
    int32_t c;

    int32_t lo = 10 - (s & 0x0f) - x;
    if (lo < 10) {
        c = 1;
    } else {
        lo = 0;
        c = 0;
    }

    int32_t hi = 10 - ((s >> 4) & 0x0f) - c;
    if (hi < 10) {
        c = 1;
    } else {
        c = 0;
        hi = 0;
    }

    const int32_t result = (hi << 4) + lo;

    if (c != 0) {
        cpu.setFlags(X_FLAG | C_FLAG);
    } else {
        cpu.clrFlags(X_FLAG | C_FLAG);
    }

    if (result != 0) {
        cpu.clrFlags(Z_FLAG);
    }

    op->setByte(result);

    return (op->isRegisterMode() ? 6 : 8);
}

DisassembledInstruction NBCD::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand op = cpu.disassembleDstEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    return DisassembledInstruction(address, opcode, "nbcd", op);
}

} // namespace m68k::cpu::instructions
