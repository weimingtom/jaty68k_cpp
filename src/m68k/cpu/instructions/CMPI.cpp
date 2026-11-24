#include "m68k/cpu/instructions/CMPI.h"

#include <stdexcept>

#include "m68k/cpu/CpuUtils.h"
#include "m68k/cpu/LambdaInstruction.h"
#include "m68k/util/Format.h"

namespace m68k::cpu::instructions {

CMPI::CMPI(Cpu& cpu) : cpu(cpu) {}

void CMPI::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            base = 0x0c00;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return cmpi_byte(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            base = 0x0c40;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return cmpi_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            base = 0x0c80;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return cmpi_long(opcode); },
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

int32_t CMPI::cmpi_byte(int32_t opcode) {
    const int32_t s = signExtendByte(cpu.fetchPCWord());
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Byte);

    const int32_t d = op->getByteSigned();

    const int32_t r = d - s;

    cpu.calcFlags(InstructionType::CMP, s, d, r, Size::Byte);

    return (op->isRegisterMode() ? 8 : 8 + op->getTiming());
}

int32_t CMPI::cmpi_word(int32_t opcode) {
    const int32_t s = cpu.fetchPCWordSigned();
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Word);

    const int32_t d = op->getWordSigned();

    const int32_t r = d - s;

    cpu.calcFlags(InstructionType::CMP, s, d, r, Size::Word);
    return (op->isRegisterMode() ? 8 : 8 + op->getTiming());
}

int32_t CMPI::cmpi_long(int32_t opcode) {
    const int32_t s = cpu.fetchPCLong();
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Long);

    const int32_t d = op->getLong();

    const int32_t r = d - s;

    cpu.calcFlags(InstructionType::CMP, s, d, r, Size::Long);

    return (op->isRegisterMode() ? 14 : 12 + op->getTiming());
}

DisassembledInstruction CMPI::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    int32_t val;
    int32_t bytes_read;
    std::string op;

    switch (sz) {
        case Size::Byte: {
            val = cpu.readMemoryWord(address + 2);
            bytes_read = 2;
            op = m68k::util::format("#$%02x", (val & 0xff));
            break;
        }
        case Size::Word: {
            val = cpu.readMemoryWord(address + 2);
            bytes_read = 2;
            op = m68k::util::format("#$%04x", (val & 0x0000ffff));
            break;
        }
        case Size::Long: {
            val = cpu.readMemoryLong(address + 2);
            bytes_read = 4;
            op = m68k::util::format("#$%08x", val);
            break;
        }
        default: {
            throw std::invalid_argument("Invalid size for CMPI");
        }
    }

    DisassembledOperand src(op, bytes_read, val);
    DisassembledOperand dst = cpu.disassembleDstEA(address + 2 + bytes_read, (opcode >> 3) & 0x07, (opcode & 0x07), sz);

    return DisassembledInstruction(address, opcode, std::string("cmpi") + ext(sz), src, dst);
}

} // namespace m68k::cpu::instructions
