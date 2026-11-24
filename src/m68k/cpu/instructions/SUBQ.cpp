#include "m68k/cpu/instructions/SUBQ.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

SUBQ::SUBQ(Cpu& cpu) : cpu(cpu) {}

void SUBQ::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    // destination ea
    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            // subq byte
            base = 0x5100;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return subq_byte(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            // subq word
            base = 0x5140;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return subq_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // subq long
            base = 0x5180;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return subq_long(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }
        for (int32_t ea_mode = 0; ea_mode < 8; ea_mode++) {
            if (sz == 0 && ea_mode == 1) {
                continue;
            }

            for (int32_t ea_reg = 0; ea_reg < 8; ea_reg++) {
                if (ea_mode == 7 && ea_reg > 1) {
                    break;
                }

                for (int32_t imm = 0; imm < 8; imm++) {
                    is.addInstruction(base + (imm << 9) + (ea_mode << 3) + ea_reg, i);
                }
            }
        }
    }
}

int32_t SUBQ::subq_byte(int32_t opcode) {
    int32_t s = ((opcode >> 9) & 0x07);
    if (s == 0) {
        s = 8;
    }

    Operand* dst = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Byte);
    const int32_t d = dst->getByteSigned();
    const int32_t r = d - s;
    dst->setByte(r);
    cpu.calcFlags(InstructionType::SUB, s, d, r, Size::Byte);
    return (dst->isRegisterMode() ? 4 : 8 + dst->getTiming());
}

int32_t SUBQ::subq_word(int32_t opcode) {
    // SUBQ where the destination is an address register does not affect the flags and the ENTIRE address
    // reg is affected by the subtraction (same not true for byte sized, as there is no byte sized subq with address reg).

    int32_t s = ((opcode >> 9) & 0x07);
    if (s == 0) {
        s = 8;
    }

    const int32_t mode = (opcode >> 3) & 0x07;
    if (mode != 1) {
        Operand* dst = cpu.resolveDstEA(mode, (opcode & 0x07), Size::Word);
        const int32_t d = dst->getWordSigned();
        const int32_t r = d - s;
        dst->setWord(r);
        cpu.calcFlags(InstructionType::SUB, s, d, r, Size::Word);
        return (dst->isRegisterMode() ? 4 : 8 + dst->getTiming());
    } else {
        const int32_t reg = opcode & 0x07;
        cpu.setAddrRegisterLong(reg, cpu.getAddrRegisterLong(reg) - s);
        return 4;
    }
}

int32_t SUBQ::subq_long(int32_t opcode) {
    int32_t s = ((opcode >> 9) & 0x07);
    if (s == 0) {
        s = 8;
    }

    const int32_t mode = (opcode >> 3) & 0x07;
    Operand* dst = cpu.resolveDstEA(mode, (opcode & 0x07), Size::Long);
    const int32_t d = dst->getLong();
    const int32_t r = d - s;
    dst->setLong(r);

    // if destination is An then no CC affected
    if (mode != 1) {
        cpu.calcFlags(InstructionType::SUB, s, d, r, Size::Long);
    }

    return (dst->isRegisterMode() ? 8 : 12 + dst->getTiming());
}

DisassembledInstruction SUBQ::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src("#" + std::to_string((opcode >> 9) & 0x07));
    DisassembledOperand dst = cpu.disassembleDstEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);

    return DisassembledInstruction(address, opcode, std::string("subq") + ext(sz), src, dst);
}

} // namespace m68k::cpu::instructions
