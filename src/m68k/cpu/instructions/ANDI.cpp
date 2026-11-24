#include "m68k/cpu/instructions/ANDI.h"

#include <stdexcept>

#include "m68k/cpu/CpuUtils.h"
#include "m68k/cpu/LambdaInstruction.h"
#include "m68k/util/Format.h"

namespace m68k::cpu::instructions {

ANDI::ANDI(Cpu& cpu) : cpu(cpu) {}

void ANDI::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            // andi byte
            base = 0x0200;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return andi_byte(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            // andi word
            base = 0x0240;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return andi_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // andi long
            base = 0x0280;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return andi_long(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }
        for (int32_t ea_mode = 0; ea_mode < 8; ea_mode++) {
            if (ea_mode == 1) {
                continue;
            }

            for (int32_t ea_reg = 0; ea_reg < 8; ea_reg++) {
                if (ea_mode == 7 && (ea_reg == 2 || ea_reg == 3 || ea_reg == 4)) {
                    continue;
                }
                is.addInstruction(base + (ea_mode << 3) + ea_reg, i);
            }
        }
    }
}

int32_t ANDI::andi_byte(int32_t opcode) {
    const int32_t s = signExtendByte(cpu.fetchPCWord());
    Operand* dst = cpu.resolveDstEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Byte);
    const int32_t d = dst->getByteSigned();
    const int32_t r = s & d;
    dst->setByte(r);
    if (!dst->isSR()) {
        cpu.calcFlags(InstructionType::AND, s, d, r, Size::Byte);
    }

    return (dst->isRegisterMode() ? 8 : 12 + dst->getTiming());
}

int32_t ANDI::andi_word(int32_t opcode) {
    const int32_t s = cpu.fetchPCWordSigned();
    Operand* dst = cpu.resolveDstEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Word);
    const int32_t d = dst->getWordSigned();
    const int32_t r = s & d;
    if (dst->isSR()) {
        if (cpu.isSupervisorMode()) {
            cpu.setSR(r);
        } else {
            cpu.raiseSRException();
            return 34;
        }
    } else {
        dst->setWord(r);
        cpu.calcFlags(InstructionType::AND, s, d, r, Size::Word);
    }
    return (dst->isRegisterMode() ? 8 : 12 + dst->getTiming());
}

int32_t ANDI::andi_long(int32_t opcode) {
    const int32_t s = cpu.fetchPCLong();
    Operand* dst = cpu.resolveDstEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Long);
    const int32_t d = dst->getLong();
    const int32_t r = s & d;
    dst->setLong(r);
    cpu.calcFlags(InstructionType::AND, s, d, r, Size::Long);
    return (dst->isRegisterMode() ? 14 : 20 + dst->getTiming());
}

DisassembledInstruction ANDI::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    int32_t imm_bytes;
    int32_t imm;
    std::string is;

    switch (sz) {
        case Size::Byte: {
            imm = cpu.readMemoryWord(address + 2);
            is = m68k::util::format("#$%02x", static_cast<uint8_t>(imm & 0x00ff));
            imm_bytes = 2;
            break;
        }
        case Size::Word: {
            imm = cpu.readMemoryWord(address + 2);
            is = m68k::util::format("#$%04x", static_cast<uint16_t>(imm));
            imm_bytes = 2;
            break;
        }
        case Size::Long: {
            imm = cpu.readMemoryLong(address + 2);
            is = m68k::util::format("#$%08x", static_cast<uint32_t>(imm));
            imm_bytes = 4;
            break;
        }
        default: {
            throw std::invalid_argument("Size unsized for ANDI");
        }
    }

    DisassembledOperand src(is, imm_bytes, imm);
    DisassembledOperand dst = cpu.disassembleDstEA(address + 2 + imm_bytes, (opcode >> 3) & 0x07, (opcode & 0x07), sz);

    return DisassembledInstruction(address, opcode, std::string("andi") + ext(sz), src, dst);
}

} // namespace m68k::cpu::instructions
