#include "m68k/cpu/instructions/EXT.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

EXT::EXT(Cpu& cpu) : cpu(cpu) {}

void EXT::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t sz = 0; sz < 2; sz++) {
        if (sz == 0) {
            // ext byte to word
            base = 0x4880;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return ext_byte_to_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // ext word to long
            base = 0x48c0;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return ext_word_to_long(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }

        for (int32_t reg = 0; reg < 8; reg++) {
            is.addInstruction(base + reg, i);
        }
    }
}

int32_t EXT::ext_byte_to_word(int32_t opcode) {
    int32_t s = cpu.getDataRegisterByte(opcode & 0x07);
    if ((s & 0x80) == 0x80) {
        s |= 0xff00;
        cpu.setFlags(N_FLAG);
    } else {
        cpu.clrFlags(N_FLAG);
    }
    cpu.setDataRegisterWord((opcode & 0x07), s);
    if (s == 0) {
        cpu.setFlags(Z_FLAG);
    } else {
        cpu.clrFlags(Z_FLAG);
    }
    cpu.clrFlags(C_FLAG | V_FLAG);
    return 4;
}

int32_t EXT::ext_word_to_long(int32_t opcode) {
    int32_t s = cpu.getDataRegisterWord(opcode & 0x07);
    if ((s & 0x8000) == 0x8000) {
        s |= 0xffff0000;
        cpu.setFlags(N_FLAG);
    } else {
        cpu.clrFlags(N_FLAG);
    }
    cpu.setDataRegisterLong((opcode & 0x07), s);
    if (s == 0) {
        cpu.setFlags(Z_FLAG);
    } else {
        cpu.clrFlags(Z_FLAG);
    }
    cpu.clrFlags(C_FLAG | V_FLAG);
    return 4;
}

DisassembledInstruction EXT::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src("d" + std::to_string(opcode & 0x07));

    return DisassembledInstruction(address, opcode, std::string("ext") + ext(sz), src);
}

} // namespace m68k::cpu::instructions
