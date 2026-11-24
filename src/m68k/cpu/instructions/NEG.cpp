#include "m68k/cpu/instructions/NEG.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

NEG::NEG(Cpu& cpu) : cpu(cpu) {}

void NEG::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            // neg byte
            base = 0x4400;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return neg_byte(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            // neg word
            base = 0x4440;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return neg_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // neg long
            base = 0x4480;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return neg_long(opcode); },
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

int32_t NEG::neg_byte(int32_t opcode) {
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Byte);
    const int32_t s = op->getByte();
    const int32_t r = 0 - s;
    op->setByte(r);
    cpu.calcFlags(InstructionType::NEG, s, 0, r, Size::Byte);
    return (op->isRegisterMode() ? 4 : 8 + op->getTiming());
}

int32_t NEG::neg_word(int32_t opcode) {
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Word);
    const int32_t s = op->getWord();
    const int32_t r = 0 - s;
    op->setWord(r);
    cpu.calcFlags(InstructionType::NEG, s, 0, r, Size::Word);
    return (op->isRegisterMode() ? 4 : 8 + op->getTiming());
}

int32_t NEG::neg_long(int32_t opcode) {
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Long);
    const int32_t s = op->getLong();
    const int32_t r = 0 - s;
    op->setLong(r);
    cpu.calcFlags(InstructionType::NEG, s, 0, r, Size::Long);
    return (op->isRegisterMode() ? 6 : 12 + op->getTiming());
}

DisassembledInstruction NEG::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src = cpu.disassembleDstEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    return DisassembledInstruction(address, opcode, std::string("neg") + ext(sz), src);
}

} // namespace m68k::cpu::instructions
