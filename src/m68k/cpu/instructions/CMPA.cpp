#include "m68k/cpu/instructions/CMPA.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

CMPA::CMPA(Cpu& cpu) : cpu(cpu) {}

void CMPA::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t sz = 0; sz < 2; sz++) {
        if (sz == 0) { // word
            base = 0xb0c0;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return cmpa_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else { // long
            base = 0xb1c0;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return cmpa_long(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }

        for (int32_t ea_mode = 0; ea_mode < 8; ea_mode++) {
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

int32_t CMPA::cmpa_word(int32_t opcode) {
    // the address register is always used as long word, the source reg is sign extended to 32 bits
    Operand* op = cpu.resolveSrcEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Word);
    const int32_t d = cpu.getAddrRegisterLong((opcode >> 9) & 0x07);
    const int32_t s = op->getWordSigned();
    const int32_t r = d - s;
    // calculate flags on long, not word
    cpu.calcFlags(InstructionType::CMP, s, d, r, Size::Long);

    return 6 + op->getTiming();
}

int32_t CMPA::cmpa_long(int32_t opcode) {
    Operand* op = cpu.resolveSrcEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Long);
    const int32_t d = cpu.getAddrRegisterLong((opcode >> 9) & 0x07);
    const int32_t s = op->getLong();

    const int32_t r = d - s;

    cpu.calcFlags(InstructionType::CMP, s, d, r, Size::Long);

    return 6 + op->getTiming();
}

DisassembledInstruction CMPA::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src = cpu.disassembleSrcEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    DisassembledOperand dst("a" + std::to_string((opcode >> 9) & 0x07));
    return DisassembledInstruction(address, opcode, std::string("cmpa") + ext(sz), src, dst);
}

} // namespace m68k::cpu::instructions
