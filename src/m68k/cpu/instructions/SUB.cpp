#include "m68k/cpu/instructions/SUB.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

SUB::SUB(Cpu& cpu) : cpu(cpu) {}

void SUB::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    // destination dn
    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            // sub byte (dn dest)
            base = 0x9000;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return sub_byte_dn_dest(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            // sub word (dn dest)
            base = 0x9040;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return sub_word_dn_dest(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // sub long (dn dest)
            base = 0x9080;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return sub_long_dn_dest(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }
        for (int32_t reg = 0; reg < 8; reg++) {
            for (int32_t ea_mode = 0; ea_mode < 8; ea_mode++) {
                for (int32_t ea_reg = 0; ea_reg < 8; ea_reg++) {
                    if (ea_mode == 7 && ea_reg > 4) {
                        break;
                    }
                    is.addInstruction(base + (reg << 9) + (ea_mode << 3) + ea_reg, i);
                }
            }
        }
    }

    // destination ea
    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            // sub byte (ea dest)
            base = 0x9100;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return sub_byte_ea_dest(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            // sub word (ea dest)
            base = 0x9140;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return sub_word_ea_dest(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // sub long (ea dest)
            base = 0x9180;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return sub_long_ea_dest(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }
        for (int32_t reg = 0; reg < 8; reg++) {
            for (int32_t ea_mode = 2; ea_mode < 8; ea_mode++) {
                for (int32_t ea_reg = 0; ea_reg < 8; ea_reg++) {
                    if (ea_mode == 7 && ea_reg > 1) {
                        break;
                    }
                    is.addInstruction(base + (reg << 9) + (ea_mode << 3) + ea_reg, i);
                }
            }
        }
    }
}

int32_t SUB::sub_byte_dn_dest(int32_t opcode) {
    // data reg destination
    Operand* src = cpu.resolveSrcEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Byte);
    const int32_t s = src->getByteSigned();
    const int32_t reg = (opcode >> 9) & 0x07;
    const int32_t d = cpu.getDataRegisterByteSigned(reg);
    const int32_t r = d - s;
    cpu.setDataRegisterByte(reg, r);
    const int32_t time = 4 + src->getTiming();
    cpu.calcFlags(InstructionType::SUB, s, d, r, Size::Byte);
    return time;
}

int32_t SUB::sub_byte_ea_dest(int32_t opcode) {
    const int32_t s = cpu.getDataRegisterByteSigned((opcode >> 9) & 0x07);
    Operand* dst = cpu.resolveDstEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Byte);
    const int32_t d = dst->getByteSigned();
    const int32_t r = d - s;
    dst->setByte(r);
    const int32_t time = 8 + dst->getTiming();
    cpu.calcFlags(InstructionType::SUB, s, d, r, Size::Byte);
    return time;
}

int32_t SUB::sub_word_dn_dest(int32_t opcode) {
    // data reg destination
    Operand* src = cpu.resolveSrcEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Word);
    const int32_t s = src->getWordSigned();
    const int32_t reg = (opcode >> 9) & 0x07;
    const int32_t d = cpu.getDataRegisterWordSigned(reg);
    const int32_t r = d - s;
    cpu.setDataRegisterWord(reg, r);
    const int32_t time = 4 + src->getTiming();
    cpu.calcFlags(InstructionType::SUB, s, d, r, Size::Word);
    return time;
}

int32_t SUB::sub_word_ea_dest(int32_t opcode) {
    const int32_t s = cpu.getDataRegisterWordSigned((opcode >> 9) & 0x07);
    Operand* dst = cpu.resolveDstEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Word);
    const int32_t d = dst->getWordSigned();
    const int32_t r = d - s;
    dst->setWord(r);
    const int32_t time = 8 + dst->getTiming();
    cpu.calcFlags(InstructionType::SUB, s, d, r, Size::Word);
    return time;
}

int32_t SUB::sub_long_dn_dest(int32_t opcode) {
    // data reg destination
    Operand* src = cpu.resolveSrcEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Long);
    const int32_t s = src->getLong();
    const int32_t reg = (opcode >> 9) & 0x07;
    const int32_t d = cpu.getDataRegisterLong(reg);
    const int32_t r = d - s;
    cpu.setDataRegisterLong(reg, r);
    const int32_t time = 6 + src->getTiming();
    cpu.calcFlags(InstructionType::SUB, s, d, r, Size::Long);
    return time;
}

int32_t SUB::sub_long_ea_dest(int32_t opcode) {
    const int32_t s = cpu.getDataRegisterLong((opcode >> 9) & 0x07);
    Operand* dst = cpu.resolveDstEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Long);
    const int32_t d = dst->getLong();
    const int32_t r = d - s;
    dst->setLong(r);
    const int32_t time = 12 + dst->getTiming();
    cpu.calcFlags(InstructionType::SUB, s, d, r, Size::Long);
    return time;
}

DisassembledInstruction SUB::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    if ((opcode & 0x0100) == 0) {
        // data reg destination
        DisassembledOperand src =
            cpu.disassembleSrcEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
        DisassembledOperand dst("d" + std::to_string((opcode >> 9) & 0x07));
        return DisassembledInstruction(address, opcode, std::string("sub") + ext(sz), src, dst);
    }

    // ea destination
    DisassembledOperand src("d" + std::to_string((opcode >> 9) & 0x07));
    DisassembledOperand dst = cpu.disassembleDstEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    return DisassembledInstruction(address, opcode, std::string("sub") + ext(sz), src, dst);
}

} // namespace m68k::cpu::instructions
