#include "m68k/cpu/instructions/MOVE.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

const std::array<std::array<int32_t, 9>, 12> MOVE::ShortExecutionTime = {
    {
        { 4,  4,  8,  8,  8, 12, 14, 12, 16 },
        { 4,  4,  8,  8,  8, 12, 14, 12, 16 },
        { 8,  8, 12, 12, 12, 16, 18, 16, 20 },
        { 8,  8, 12, 12, 12, 16, 18, 16, 20 },
        { 10, 10, 14, 14, 14, 18, 20, 18, 22 },
        { 12, 12, 16, 16, 16, 20, 22, 20, 24 },
        { 14, 14, 18, 18, 18, 22, 24, 22, 26 },
        { 12, 12, 16, 16, 16, 20, 22, 20, 24 },
        { 16, 16, 20, 20, 20, 24, 26, 24, 28 },
        { 12, 12, 16, 16, 16, 20, 22, 20, 24 },
        { 14, 14, 18, 18, 18, 22, 24, 22, 26 },
        { 8,  8, 12, 12, 12, 16, 18, 16, 20 }
    }
};

const std::array<std::array<int32_t, 9>, 12> MOVE::LongExecutionTime = {
    {
        { 4,  4, 12, 12, 12, 16, 18, 16, 20 },
        { 4,  4, 12, 12, 12, 16, 18, 16, 20 },
        { 12, 12, 20, 20, 20, 24, 26, 24, 28 },
        { 12, 12, 20, 20, 20, 24, 26, 24, 28 },
        { 14, 14, 22, 22, 22, 26, 28, 26, 30 },
        { 16, 16, 24, 24, 24, 28, 30, 28, 32 },
        { 18, 18, 26, 26, 26, 30, 32, 30, 34 },
        { 16, 16, 24, 24, 24, 28, 30, 28, 32 },
        { 20, 20, 28, 28, 28, 32, 34, 32, 36 },
        { 16, 16, 24, 24, 24, 28, 30, 28, 32 },
        { 18, 18, 26, 26, 26, 30, 32, 30, 34 },
        { 12, 12, 20, 20, 20, 24, 26, 24, 28 }
    }
};

MOVE::MOVE(Cpu& cpu) : cpu(cpu) {}

void MOVE::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    // destination ea
    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            // move byte
            base = 0x1000;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return move_byte(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            // move word
            base = 0x3000;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return move_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // move long
            base = 0x2000;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return move_long(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }
        for (int32_t sea_mode = 0; sea_mode < 8; sea_mode++) {
            for (int32_t sea_reg = 0; sea_reg < 8; sea_reg++) {
                if (sea_mode == 7 && sea_reg > 4) {
                    break;
                }

                for (int32_t dea_mode = 0; dea_mode < 8; dea_mode++) {
                    if (dea_mode == 1) {
                        continue;
                    }

                    for (int32_t dea_reg = 0; dea_reg < 8; dea_reg++) {
                        if (dea_mode == 7 && dea_reg > 1) {
                            break;
                        }

                        is.addInstruction(base + (dea_reg << 9) + (dea_mode << 6) + (sea_mode << 3) + sea_reg, i);
                    }
                }
            }
        }
    }
}

int32_t MOVE::move_byte(int32_t opcode) {
    Operand* src = cpu.resolveSrcEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Byte);
    Operand* dst = cpu.resolveDstEA((opcode >> 6) & 0x07, (opcode >> 9) & 0x07, Size::Byte);
    int32_t s = src->getByte();
    dst->setByte(s);
    cpu.calcFlags(InstructionType::MOVE, s, s, s, Size::Byte);
    return ShortExecutionTime[src->index()][dst->index()];
}

int32_t MOVE::move_word(int32_t opcode) {
    Operand* src = cpu.resolveSrcEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Word);
    Operand* dst = cpu.resolveDstEA((opcode >> 6) & 0x07, (opcode >> 9) & 0x07, Size::Word);
    int32_t s = src->getWord();
    dst->setWord(s);
    cpu.calcFlags(InstructionType::MOVE, s, s, s, Size::Word);
    return ShortExecutionTime[src->index()][dst->index()];
}

int32_t MOVE::move_long(int32_t opcode) {
    Operand* src = cpu.resolveSrcEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Long);
    Operand* dst = cpu.resolveDstEA((opcode >> 6) & 0x07, (opcode >> 9) & 0x07, Size::Long);
    int32_t s = src->getLong();
    dst->setLong(s);
    cpu.calcFlags(InstructionType::MOVE, s, s, s, Size::Long);
    return LongExecutionTime[src->index()][dst->index()];
}

DisassembledInstruction MOVE::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src = cpu.disassembleSrcEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    DisassembledOperand dst =
        cpu.disassembleDstEA(address + 2 + src.bytes, (opcode >> 6) & 0x07, (opcode >> 9) & 0x07, sz);

    return DisassembledInstruction(address, opcode, std::string("move") + ext(sz), src, dst);
}

} // namespace m68k::cpu::instructions
