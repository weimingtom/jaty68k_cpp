#include "m68k/cpu/instructions/ROL.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

ROL::ROL(Cpu& cpu) : cpu(cpu) {}

void ROL::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            // rol byte imm
            base = 0xe118;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return rol_byte_imm(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            // rol word imm
            base = 0xe158;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return rol_word_imm(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // rol long imm
            base = 0xe198;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return rol_long_imm(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }
        for (int32_t imm = 0; imm < 8; imm++) {
            for (int32_t reg = 0; reg < 8; reg++) {
                is.addInstruction(base + (imm << 9) + reg, i);
            }
        }
    }

    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            // rol byte reg
            base = 0xe138;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return rol_byte_reg(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            // rol word reg
            base = 0xe178;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return rol_word_reg(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // rol long reg
            base = 0xe1b8;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return rol_long_reg(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }
        for (int32_t imm = 0; imm < 8; imm++) {
            for (int32_t reg = 0; reg < 8; reg++) {
                is.addInstruction(base + (imm << 9) + reg, i);
            }
        }
    }

    // rol word mem
    base = 0xe7c0;
    i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return rol_word_mem(opcode); },
        [this](int32_t address, int32_t opcode) {
            return disassembleOp(address, opcode, Size::Word);
        });

    for (int32_t ea_mode = 2; ea_mode < 8; ea_mode++) {
        for (int32_t ea_reg = 0; ea_reg < 8; ea_reg++) {
            if (ea_mode == 7 && ea_reg > 1) {
                break;
            }
            is.addInstruction(base + (ea_mode << 3) + ea_reg, i);
        }
    }
}

int32_t ROL::rol_byte_imm(int32_t opcode) {
    int32_t shift = (opcode >> 9) & 0x07;
    if (shift == 0) {
        shift = 8;
    }

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterByte(reg);

    int32_t last_out = 0;
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x80;
        d <<= 1;
        if (last_out != 0) {
            d |= 1;
        }
    }
    d &= 0x00ff;
    cpu.setDataRegisterByte(reg, d);

    cpu.calcFlags(InstructionType::ROL, shift, last_out, d, Size::Byte);
    return 6 + shift + shift;
}

int32_t ROL::rol_word_imm(int32_t opcode) {
    int32_t shift = (opcode >> 9) & 0x07;
    if (shift == 0) {
        shift = 8;
    }

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterWord(reg);

    int32_t last_out = 0;
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x8000;
        d <<= 1;
        if (last_out != 0) {
            d |= 1;
        }
    }
    d &= 0x0000ffff;
    cpu.setDataRegisterWord(reg, d);

    cpu.calcFlags(InstructionType::ROL, shift, last_out, d, Size::Word);
    return 6 + shift + shift;
}

int32_t ROL::rol_long_imm(int32_t opcode) {
    int32_t shift = (opcode >> 9) & 0x07;
    if (shift == 0) {
        shift = 8;
    }

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterLong(reg);

    int32_t last_out = 0;
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x80000000;
        d <<= 1;
        if (last_out != 0) {
            d |= 1;
        }
    }
    cpu.setDataRegisterLong(reg, d);

    cpu.calcFlags(InstructionType::ROL, shift, last_out, d, Size::Long);
    return 8 + shift + shift;
}

int32_t ROL::rol_byte_reg(int32_t opcode) {
    // shift count is mode 64
    int32_t shift = cpu.getDataRegisterLong((opcode >> 9) & 0x07) & 63;

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterByte(reg);

    int32_t last_out = 0;
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x80;
        d <<= 1;
        if (last_out != 0) {
            d |= 1;
        }
    }
    d &= 0x00ff;
    cpu.setDataRegisterByte(reg, d);

    cpu.calcFlags(InstructionType::ROL, shift, last_out, d, Size::Byte);
    return 6 + shift + shift;
}

int32_t ROL::rol_word_reg(int32_t opcode) {
    // shift count is mode 64
    int32_t shift = cpu.getDataRegisterLong((opcode >> 9) & 0x07) & 63;

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterWord(reg);

    int32_t last_out = 0;
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x8000;
        d <<= 1;
        if (last_out != 0) {
            d |= 1;
        }
    }
    d &= 0x0000ffff;
    cpu.setDataRegisterWord(reg, d);

    cpu.calcFlags(InstructionType::ROL, shift, last_out, d, Size::Word);
    return 6 + shift + shift;
}

int32_t ROL::rol_long_reg(int32_t opcode) {
    // shift count is mode 64
    int32_t shift = cpu.getDataRegisterLong((opcode >> 9) & 0x07) & 63;

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterLong(reg);

    int32_t last_out = 0;
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x80000000;
        d <<= 1;
        if (last_out != 0) {
            d |= 1;
        }
    }
    cpu.setDataRegisterLong(reg, d);

    cpu.calcFlags(InstructionType::ROL, shift, last_out, d, Size::Long);
    return 8 + shift + shift;
}

int32_t ROL::rol_word_mem(int32_t opcode) {
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Word);
    int32_t v = op->getWord();

    int32_t last_out = v & 0x8000;
    v <<= 1;
    if (last_out != 0) {
        v |= 1;
    }
    op->setWord(v);
    cpu.calcFlags(InstructionType::ROL, 1, last_out, v, Size::Word);
    return 8 + op->getTiming();
}

DisassembledInstruction ROL::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    if ((opcode & 0x00c0) == 0x00c0) {
        //mem mode
        DisassembledOperand src = cpu.disassembleDstEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
        return DisassembledInstruction(address, opcode, std::string("rol") + ext(sz), src);
    } else if ((opcode & 0x0020) == 0x0020) {
        //reg mode
        DisassembledOperand src("d" + std::to_string((opcode >> 9) & 0x07));
        DisassembledOperand dst("d" + std::to_string(opcode & 0x07));
        return DisassembledInstruction(address, opcode, std::string("rol") + ext(sz), src, dst);
    } else {
        //immediate mode
        int32_t count = (opcode >> 9) & 0x07;
        if (count == 0) {
            count = 8;
        }

        DisassembledOperand src("#" + std::to_string(count));
        DisassembledOperand dst("d" + std::to_string(opcode & 0x07));
        return DisassembledInstruction(address, opcode, std::string("rol") + ext(sz), src, dst);
    }
}

} // namespace m68k::cpu::instructions
