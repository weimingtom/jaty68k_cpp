#include "m68k/cpu/instructions/ROR.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

ROR::ROR(Cpu& cpu) : cpu(cpu) {}

void ROR::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            // ror byte imm
            base = 0xe018;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return ror_byte_imm(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            // ror word imm
            base = 0xe058;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return ror_word_imm(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // ror long imm
            base = 0xe098;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return ror_long_imm(opcode); },
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
            // ror byte reg
            base = 0xe038;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return ror_byte_reg(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            // ror word reg
            base = 0xe078;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return ror_word_reg(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // ror long reg
            base = 0xe0b8;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return ror_long_reg(opcode); },
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

    // ror word mem
    base = 0xe6c0;
    i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return ror_word_mem(opcode); },
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

int32_t ROR::ror_byte_imm(int32_t opcode) {
    int32_t shift = (opcode >> 9) & 0x07;
    if (shift == 0) {
        shift = 8;
    }

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterByte(reg);

    int32_t last_out = 0;
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x01;
        d = static_cast<int32_t>(static_cast<uint32_t>(d) >> 1);
        if (last_out != 0) {
            d |= 0x80;
        }
    }
    d &= 0x00ff;
    cpu.setDataRegisterByte(reg, d);

    cpu.calcFlags(InstructionType::ROR, shift, last_out, d, Size::Byte);
    return 6 + shift + shift;
}

int32_t ROR::ror_word_imm(int32_t opcode) {
    int32_t shift = (opcode >> 9) & 0x07;
    if (shift == 0) {
        shift = 8;
    }

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterWord(reg);

    int32_t last_out = 0;
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x01;
        d = static_cast<int32_t>(static_cast<uint32_t>(d) >> 1);
        if (last_out != 0) {
            d |= 0x8000;
        }
    }
    d &= 0x0000ffff;
    cpu.setDataRegisterWord(reg, d);

    cpu.calcFlags(InstructionType::ROR, shift, last_out, d, Size::Word);
    return 6 + shift + shift;
}

int32_t ROR::ror_long_imm(int32_t opcode) {
    int32_t shift = (opcode >> 9) & 0x07;
    if (shift == 0) {
        shift = 8;
    }

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterLong(reg);

    int32_t last_out = 0;
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x01;
        d = static_cast<int32_t>(static_cast<uint32_t>(d) >> 1);
        if (last_out != 0) {
            d |= 0x80000000;
        }
    }
    cpu.setDataRegisterLong(reg, d);

    cpu.calcFlags(InstructionType::ROR, shift, last_out, d, Size::Long);
    return 8 + shift + shift;
}

int32_t ROR::ror_byte_reg(int32_t opcode) {
    // shift count is mode 64 not 32
    int32_t shift = cpu.getDataRegisterLong((opcode >> 9) & 0x07) & 63;

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterByte(reg);

    int32_t last_out = 0;
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x01;
        d = static_cast<int32_t>(static_cast<uint32_t>(d) >> 1);
        if (last_out != 0) {
            d |= 0x80;
        }
    }
    d &= 0x00ff;
    cpu.setDataRegisterByte(reg, d);

    cpu.calcFlags(InstructionType::ROR, shift, last_out, d, Size::Byte);
    return 6 + shift + shift;
}

int32_t ROR::ror_word_reg(int32_t opcode) {
    // shift count is mode 64 not 32
    int32_t shift = cpu.getDataRegisterLong((opcode >> 9) & 0x07) & 63;

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterWord(reg);

    int32_t last_out = 0;
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x01;
        d = static_cast<int32_t>(static_cast<uint32_t>(d) >> 1);
        if (last_out != 0) {
            d |= 0x8000;
        }
    }
    d &= 0x0000ffff;
    cpu.setDataRegisterWord(reg, d);

    cpu.calcFlags(InstructionType::ROR, shift, last_out, d, Size::Word);
    return 6 + shift + shift;
}

int32_t ROR::ror_long_reg(int32_t opcode) {
    // shift count is mode 64
    int32_t shift = cpu.getDataRegisterLong((opcode >> 9) & 0x07) & 63;

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterLong(reg);

    int32_t last_out = 0;
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x01;
        d = static_cast<int32_t>(static_cast<uint32_t>(d) >> 1);
        if (last_out != 0) {
            d |= 0x80000000;
        }
    }
    cpu.setDataRegisterLong(reg, d);

    cpu.calcFlags(InstructionType::ROR, shift, last_out, d, Size::Long);
    return 8 + shift + shift;
}

int32_t ROR::ror_word_mem(int32_t opcode) {
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Word);
    int32_t v = op->getWord();
    int32_t last_out = v & 0x01;

    v = static_cast<int32_t>(static_cast<uint32_t>(v) >> 1);
    if (last_out != 0) {
        v |= 0x8000;
    }

    op->setWord(v);
    cpu.calcFlags(InstructionType::ROR, 1, last_out, v, Size::Word);
    return 8 + op->getTiming();
}

DisassembledInstruction ROR::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    if ((opcode & 0x00c0) == 0x00c0) {
        //mem mode
        DisassembledOperand src = cpu.disassembleDstEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
        return DisassembledInstruction(address, opcode, std::string("ror") + ext(sz), src);
    } else if ((opcode & 0x0020) == 0x0020) {
        //reg mode
        DisassembledOperand src("d" + std::to_string((opcode >> 9) & 0x07));
        DisassembledOperand dst("d" + std::to_string(opcode & 0x07));
        return DisassembledInstruction(address, opcode, std::string("ror") + ext(sz), src, dst);
    } else {
        //immediate mode
        int32_t count = (opcode >> 9) & 0x07;
        if (count == 0) {
            count = 8;
        }

        DisassembledOperand src("#" + std::to_string(count));
        DisassembledOperand dst("d" + std::to_string(opcode & 0x07));
        return DisassembledInstruction(address, opcode, std::string("ror") + ext(sz), src, dst);
    }
}

} // namespace m68k::cpu::instructions
