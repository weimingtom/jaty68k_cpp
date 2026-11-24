#include "m68k/cpu/instructions/TST.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

TST::TST(Cpu& cpu) : cpu(cpu) {}

void TST::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            base = 0x4a00;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return tst_byte(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            base = 0x4a40;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return tst_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            base = 0x4a80;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return tst_long(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }

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
}

int32_t TST::tst_byte(int32_t opcode) {
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

    return 4 + op->getTiming();
}

int32_t TST::tst_word(int32_t opcode) {
    const int32_t mode = (opcode >> 3) & 0x07;
    const int32_t reg = (opcode & 0x07);
    Operand* op = cpu.resolveSrcEA(mode, reg, Size::Word);
    const int32_t v = op->getWord();

    if (v == 0) {
        cpu.setFlags(Z_FLAG);
    } else {
        cpu.clrFlags(Z_FLAG);
    }
    if ((v & 0x8000) != 0) {
        cpu.setFlags(N_FLAG);
    } else {
        cpu.clrFlags(N_FLAG);
    }
    cpu.clrFlags(C_FLAG | V_FLAG);

    return 4 + op->getTiming();
}

int32_t TST::tst_long(int32_t opcode) {
    const int32_t mode = (opcode >> 3) & 0x07;
    const int32_t reg = (opcode & 0x07);
    Operand* op = cpu.resolveSrcEA(mode, reg, Size::Long);
    const int32_t v = op->getLong();

    if (v == 0) {
        cpu.setFlags(Z_FLAG);
    } else {
        cpu.clrFlags(Z_FLAG);
    }
    if ((v & 0x80000000) != 0) {
        cpu.setFlags(N_FLAG);
    } else {
        cpu.clrFlags(N_FLAG);
    }
    cpu.clrFlags(C_FLAG | V_FLAG);

    return 4 + op->getTiming();
}

DisassembledInstruction TST::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand op = cpu.disassembleSrcEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    return DisassembledInstruction(address, opcode, std::string("tst") + ext(sz), op);
}

} // namespace m68k::cpu::instructions
