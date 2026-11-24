#include "m68k/cpu/instructions/CMP.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

CMP::CMP(Cpu& cpu) : cpu(cpu) {}

void CMP::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            base = 0xb000;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return cmp_byte(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            base = 0xb040;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return cmp_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            base = 0xb080;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return cmp_long(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }

        for (int32_t ea_mode = 0; ea_mode < 8; ea_mode++) {
            if (ea_mode == 1 && sz == 0) {
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
}

int32_t CMP::cmp_byte(int32_t opcode) {
    Operand* op = cpu.resolveSrcEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Byte);

    const int32_t s = op->getByte();
    const int32_t d = cpu.getDataRegisterByte((opcode >> 9) & 0x07);

    const int32_t r = d - s;

    cpu.calcFlags(InstructionType::CMP, s, d, r, Size::Byte);

    return 4 + op->getTiming();
}

int32_t CMP::cmp_word(int32_t opcode) {
    Operand* op = cpu.resolveSrcEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Word);

    const int32_t s = op->getWord();
    const int32_t d = cpu.getDataRegisterWord((opcode >> 9) & 0x07);

    const int32_t r = d - s;

    cpu.calcFlags(InstructionType::CMP, s, d, r, Size::Word);

    return 4 + op->getTiming();
}

int32_t CMP::cmp_long(int32_t opcode) {
    Operand* op = cpu.resolveSrcEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Long);

    const int32_t s = op->getLong();
    const int32_t d = cpu.getDataRegisterLong((opcode >> 9) & 0x07);

    const int32_t r = d - s;

    cpu.calcFlags(InstructionType::CMP, s, d, r, Size::Long);

    return 6 + op->getTiming();
}

DisassembledInstruction CMP::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src = cpu.disassembleSrcEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    DisassembledOperand dst("d" + std::to_string((opcode >> 9) & 0x07));

    return DisassembledInstruction(address, opcode, std::string("cmp") + ext(sz), src, dst);
}

} // namespace m68k::cpu::instructions
