#include "m68k/cpu/instructions/NOT.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

NOT::NOT(Cpu& cpu) : cpu(cpu) {}

void NOT::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            // not byte
            base = 0x4600;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return not_byte(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            // not word
            base = 0x4640;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return not_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // not long
            base = 0x4680;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return not_long(opcode); },
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

int32_t NOT::not_byte(int32_t opcode) {
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Byte);
    const int32_t s = op->getByte();
    const int32_t r = (~s) & 0xFF;
    op->setByte(r);
    cpu.calcFlags(InstructionType::NOT, s, 0, r, Size::Byte);
    return (op->isRegisterMode() ? 4 : 8 + op->getTiming());
}

int32_t NOT::not_word(int32_t opcode) {
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Word);
    const int32_t s = op->getWord();
    const int32_t r = (~s) & 0xffff;
    op->setWord(r);
    cpu.calcFlags(InstructionType::NOT, s, 0, r, Size::Word);
    return (op->isRegisterMode() ? 4 : 8 + op->getTiming());
}

int32_t NOT::not_long(int32_t opcode) {
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Long);
    const int32_t s = op->getLong();
    const int32_t r = ~s;
    op->setLong(r);
    cpu.calcFlags(InstructionType::NOT, s, 0, r, Size::Long);
    return (op->isRegisterMode() ? 6 : 12 + op->getTiming());
}

DisassembledInstruction NOT::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src = cpu.disassembleDstEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    return DisassembledInstruction(address, opcode, std::string("not") + ext(sz), src);
}

} // namespace m68k::cpu::instructions
