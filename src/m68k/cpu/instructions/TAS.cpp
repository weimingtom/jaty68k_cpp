#include "m68k/cpu/instructions/TAS.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

TAS::TAS(Cpu& cpu) : cpu(cpu) {}

void TAS::registerInstructions(InstructionSet& is) {
    int32_t base = 0x4ac0;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return tas(opcode); },
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

int32_t TAS::tas(int32_t opcode) {
    //TODO: this is for multi-processor systems and provides an atomic read-modify-write - this isn't handled at the moment

    const int32_t mode = (opcode >> 3) & 0x07;
    const int32_t reg = (opcode & 0x07);
    Operand* op = cpu.resolveSrcEA(mode, reg, Size::Byte);
    const int32_t v = op->getByte();

    if (v == 0) {
        cpu.setFlags(Z_FLAG);
    } else {
        cpu.clrFlags(Z_FLAG);
    }
    if ((v & 0x080) != 0) {
        cpu.setFlags(N_FLAG);
    } else {
        cpu.clrFlags(N_FLAG);
    }
    cpu.clrFlags(C_FLAG | V_FLAG);

    op->setByte(v | 0x80);

    return (op->isRegisterMode() ? 4 : 14 + op->getTiming());
}

DisassembledInstruction TAS::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand op =
        cpu.disassembleSrcEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    return DisassembledInstruction(address, opcode, "tas", op);
}

} // namespace m68k::cpu::instructions
