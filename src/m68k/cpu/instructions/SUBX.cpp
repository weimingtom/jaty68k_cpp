#include "m68k/cpu/instructions/SUBX.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

SUBX::SUBX(Cpu& cpu) : cpu(cpu) {}

void SUBX::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    // register mode
    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            // subx byte (reg)
            base = 0x9100;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return subx_byte_reg(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            // subx word (reg)
            base = 0x9140;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return subx_word_reg(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // subx long (reg)
            base = 0x9180;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return subx_long_reg(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }
        for (int32_t regx = 0; regx < 8; regx++) {
            for (int32_t regy = 0; regy < 8; regy++) {
                is.addInstruction(base + (regx << 9) + regy, i);
            }
        }
    }

    // Memory mode
    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            // subx byte (mem)
            base = 0x9108;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return subx_byte_mem(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            // subx word (mem)
            base = 0x9148;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return subx_word_mem(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // subx long (mem)
            base = 0x9188;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return subx_long_mem(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }
        for (int32_t regx = 0; regx < 8; regx++) {
            for (int32_t regy = 0; regy < 8; regy++) {
                is.addInstruction(base + (regx << 9) + regy, i);
            }
        }
    }
}

int32_t SUBX::subx_byte_reg(int32_t opcode) {
    const int32_t s = cpu.getDataRegisterByteSigned((opcode & 0x07));
    const int32_t d = cpu.getDataRegisterByteSigned((opcode >> 9) & 0x07);
    const int32_t r = d - s - (cpu.isFlagSet(X_FLAG) ? 1 : 0);
    cpu.setDataRegisterByte((opcode >> 9) & 0x07, r);
    cpu.calcFlags(InstructionType::SUBX, s, d, r, Size::Byte);
    return 4;
}

int32_t SUBX::subx_word_reg(int32_t opcode) {
    const int32_t s = cpu.getDataRegisterWordSigned((opcode & 0x07));
    const int32_t d = cpu.getDataRegisterWordSigned((opcode >> 9) & 0x07);
    const int32_t r = d - s - (cpu.isFlagSet(X_FLAG) ? 1 : 0);
    cpu.setDataRegisterWord((opcode >> 9) & 0x07, r);
    cpu.calcFlags(InstructionType::SUBX, s, d, r, Size::Word);
    return 4;
}

int32_t SUBX::subx_long_reg(int32_t opcode) {
    const int32_t s = cpu.getDataRegisterLong((opcode & 0x07));
    const int32_t d = cpu.getDataRegisterLong((opcode >> 9) & 0x07);
    const int32_t r = d - s - (cpu.isFlagSet(X_FLAG) ? 1 : 0);
    cpu.setDataRegisterLong((opcode >> 9) & 0x07, r);
    cpu.calcFlags(InstructionType::SUBX, s, d, r, Size::Long);
    return 8;
}

int32_t SUBX::subx_byte_mem(int32_t opcode) {
    const int32_t rx = (opcode >> 9) & 0x07;
    const int32_t ry = (opcode & 0x07);
    cpu.decrementAddrRegister(rx, 1);
    cpu.decrementAddrRegister(ry, 1);
    const int32_t s = cpu.readMemoryByteSigned(cpu.getAddrRegisterLong(ry));
    const int32_t d = cpu.readMemoryByteSigned(cpu.getAddrRegisterLong(rx));
    const int32_t r = d - s - (cpu.isFlagSet(X_FLAG) ? 1 : 0);
    cpu.writeMemoryByte(cpu.getAddrRegisterLong(rx), r);
    cpu.calcFlags(InstructionType::SUBX, s, d, r, Size::Byte);
    return 18;
}

int32_t SUBX::subx_word_mem(int32_t opcode) {
    const int32_t rx = (opcode >> 9) & 0x07;
    const int32_t ry = (opcode & 0x07);
    cpu.decrementAddrRegister(rx, 2);
    cpu.decrementAddrRegister(ry, 2);
    const int32_t s = cpu.readMemoryWordSigned(cpu.getAddrRegisterLong(ry));
    const int32_t d = cpu.readMemoryWordSigned(cpu.getAddrRegisterLong(rx));
    const int32_t r = d - s - (cpu.isFlagSet(X_FLAG) ? 1 : 0);
    cpu.writeMemoryWord(cpu.getAddrRegisterLong(rx), r);
    cpu.calcFlags(InstructionType::SUBX, s, d, r, Size::Word);
    return 18;
}

int32_t SUBX::subx_long_mem(int32_t opcode) {
    const int32_t rx = (opcode >> 9) & 0x07;
    const int32_t ry = (opcode & 0x07);
    cpu.decrementAddrRegister(rx, 4);
    cpu.decrementAddrRegister(ry, 4);
    const int32_t s = cpu.readMemoryLong(cpu.getAddrRegisterLong(ry));
    const int32_t d = cpu.readMemoryLong(cpu.getAddrRegisterLong(rx));
    const int32_t r = d - s - (cpu.isFlagSet(X_FLAG) ? 1 : 0);
    cpu.writeMemoryLong(cpu.getAddrRegisterLong(rx), r);
    cpu.calcFlags(InstructionType::SUBX, s, d, r, Size::Byte);
    return 30;
}

DisassembledInstruction SUBX::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    if ((opcode & 0x08) == 0) {
        // data reg mode
        DisassembledOperand src("d" + std::to_string(opcode & 0x07));
        DisassembledOperand dst("d" + std::to_string((opcode >> 9) & 0x07));
        return DisassembledInstruction(address, opcode, std::string("subx") + ext(sz), src, dst);
    }

    // memory mode
    DisassembledOperand src("-(a" + std::to_string(opcode & 0x07) + ")");
    DisassembledOperand dst("-(a" + std::to_string((opcode >> 9) & 0x07) + ")");
    return DisassembledInstruction(address, opcode, std::string("subx") + ext(sz), src, dst);
}

} // namespace m68k::cpu::instructions
