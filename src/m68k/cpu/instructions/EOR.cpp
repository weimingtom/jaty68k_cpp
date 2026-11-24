#include "m68k/cpu/instructions/EOR.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

EOR::EOR(Cpu& cpu) : cpu(cpu) {}

void EOR::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    // destination ea
    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            // eor byte
            base = 0xb100;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return eor_byte(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            // eor word
            base = 0xb140;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return eor_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // eor long
            base = 0xb180;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return eor_long(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }
        for (int32_t reg = 0; reg < 8; reg++) {
            for (int32_t ea_mode = 0; ea_mode < 8; ea_mode++) {
                if (ea_mode == 1) {
                    continue;
                }

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

int32_t EOR::eor_byte(int32_t opcode) {
    const int32_t s = cpu.getDataRegisterByte((opcode >> 9) & 0x07);
    Operand* dst = cpu.resolveDstEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Byte);
    const int32_t d = dst->getByte();
    const int32_t r = s ^ d;
    dst->setByte(r);
    cpu.calcFlags(InstructionType::EOR, s, d, r, Size::Byte);
    return (dst->isRegisterMode() ? 4 : 8 + dst->getTiming());
}

int32_t EOR::eor_word(int32_t opcode) {
    const int32_t s = cpu.getDataRegisterWord((opcode >> 9) & 0x07);
    Operand* dst = cpu.resolveDstEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Word);
    const int32_t d = dst->getWord();
    const int32_t r = s ^ d;
    dst->setWord(r);
    cpu.calcFlags(InstructionType::EOR, s, d, r, Size::Word);
    return (dst->isRegisterMode() ? 4 : 8 + dst->getTiming());
}

int32_t EOR::eor_long(int32_t opcode) {
    const int32_t s = cpu.getDataRegisterLong((opcode >> 9) & 0x07);
    Operand* dst = cpu.resolveDstEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Long);
    const int32_t d = dst->getLong();
    const int32_t r = s ^ d;
    dst->setLong(r);
    cpu.calcFlags(InstructionType::EOR, s, d, r, Size::Long);
    return (dst->isRegisterMode() ? 8 : 12 + dst->getTiming());
}

DisassembledInstruction EOR::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src("d" + std::to_string((opcode >> 9) & 0x07));
    DisassembledOperand dst = cpu.disassembleDstEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);

    return DisassembledInstruction(address, opcode, std::string("eor") + ext(sz), src, dst);
}

} // namespace m68k::cpu::instructions
