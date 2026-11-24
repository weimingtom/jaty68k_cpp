#include "m68k/cpu/instructions/SUBA.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

SUBA::SUBA(Cpu& cpu) : cpu(cpu) {}

void SUBA::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    // destination An
    for (int32_t sz = 0; sz < 2; sz++) {
        if (sz == 0) {
            // suba word
            base = 0x90c0;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return suba_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // adda long
            base = 0x91c0;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return suba_long(opcode); },
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
}

int32_t SUBA::suba_word(int32_t opcode) {
    Operand* src = cpu.resolveSrcEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Word);
    const int32_t s = src->getWordSigned();
    const int32_t reg = (opcode >> 9) & 0x07;
    const int32_t d = cpu.getAddrRegisterLong(reg);
    const int32_t r = d - s;
    cpu.setAddrRegisterLong(reg, r);

    //No flags affected

    return 8 + src->getTiming();
}

int32_t SUBA::suba_long(int32_t opcode) {
    Operand* src = cpu.resolveSrcEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Long);
    const int32_t s = src->getLong();
    const int32_t reg = (opcode >> 9) & 0x07;
    const int32_t d = cpu.getAddrRegisterLong(reg);
    const int32_t r = d - s;
    cpu.setAddrRegisterLong(reg, r);

    //No flags affected

    return 6 + src->getTiming();
}

DisassembledInstruction SUBA::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src = cpu.disassembleSrcEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    DisassembledOperand dst("a" + std::to_string((opcode >> 9) & 0x07));

    return DisassembledInstruction(address, opcode, std::string("suba") + ext(sz), src, dst);
}

} // namespace m68k::cpu::instructions
