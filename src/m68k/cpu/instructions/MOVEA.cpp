#include "m68k/cpu/instructions/MOVEA.h"

#include "m68k/cpu/LambdaInstruction.h"
#include "m68k/cpu/instructions/MOVE.h"

namespace m68k::cpu::instructions {

MOVEA::MOVEA(Cpu& cpu) : cpu(cpu) {}

void MOVEA::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t sz = 0; sz < 2; sz++) {
        if (sz == 0) {
            base = 0x3040;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return movea_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            base = 0x2040;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return movea_long(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }

        for (int32_t ea_mode = 0; ea_mode < 8; ea_mode++) {
            for (int32_t ea_reg = 0; ea_reg < 8; ea_reg++) {
                if (ea_mode == 7 && ea_reg > 4)
                    break;

                for (int32_t r = 0; r < 8; r++) {
                    is.addInstruction(base + (r << 9) + (ea_mode << 3) + ea_reg, i);
                }
            }
        }
    }
}

int32_t MOVEA::movea_word(int32_t opcode) {
    Operand* src = cpu.resolveSrcEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Word);
    cpu.setAddrRegisterLong((opcode >> 9) & 0x07, src->getWordSigned());
    return MOVE::ShortExecutionTime[src->index()][1];
}

int32_t MOVEA::movea_long(int32_t opcode) {
    Operand* src = cpu.resolveSrcEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Long);
    cpu.setAddrRegisterLong((opcode >> 9) & 0x07, src->getLong());
    return MOVE::LongExecutionTime[src->index()][1];
}

DisassembledInstruction MOVEA::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src = cpu.disassembleSrcEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    DisassembledOperand dst("a" + std::to_string((opcode >> 9) & 0x07));

    return DisassembledInstruction(address, opcode, std::string("movea") + ext(sz), src, dst);
}

} // namespace m68k::cpu::instructions
