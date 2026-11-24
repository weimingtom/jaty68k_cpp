#include "m68k/cpu/instructions/BCHG.h"

#include "m68k/cpu/LambdaInstruction.h"
#include "m68k/util/Format.h"

namespace m68k::cpu::instructions {

BCHG::BCHG(Cpu& cpu) : cpu(cpu) {}

void BCHG::registerInstructions(InstructionSet& is) {
    int32_t base = 0x0140;
    std::shared_ptr<Instruction> i;

    // dynamic mode
    for (int32_t ea_mode = 0; ea_mode < 8; ea_mode++) {
        if (ea_mode == 1) {
            continue;
        }

        if (ea_mode == 0) {
            // data register destination
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return bchg_dyn_long(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        } else {
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return bchg_dyn_byte(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        }
        for (int32_t ea_reg = 0; ea_reg < 8; ea_reg++) {
            if (ea_mode == 7 && ea_reg > 1) {
                break;
            }

            for (int32_t r = 0; r < 8; r++) {
                is.addInstruction(base + (r << 9) + (ea_mode << 3) + ea_reg, i);
            }
        }
    }

    // static mode
    base = 0x0840;
    for (int32_t ea_mode = 0; ea_mode < 8; ea_mode++) {
        if (ea_mode == 1) {
            continue;
        }

        if (ea_mode == 0) {
            // data register destination
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return bchg_static_long(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        } else {
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return bchg_static_byte(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        }
        for (int32_t ea_reg = 0; ea_reg < 8; ea_reg++) {
            if (ea_mode == 7 && ea_reg > 1) {
                break;
            }

            is.addInstruction(base + (ea_mode << 3) + ea_reg, i);
        }
    }
}

int32_t BCHG::bchg_dyn_byte(int32_t opcode) {
    // for memory destination, the bitnbr is MOD 8 - for data reg destination, the bitnbr is mod 32
    int32_t bit = cpu.getDataRegisterLong((opcode >> 9) & 0x07) & 7;
    bit = 1 << bit;
    // memory destination
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Byte);
    int32_t val = op->getByte();

    // Z_FLAG set according to original value
    if ((val & bit) != 0) {
        cpu.clrFlags(Z_FLAG);
        //clear the bit
        val &= ~(bit);
    } else {
        cpu.setFlags(Z_FLAG);
        //set the bit
        val |= bit;
    }
    op->setByte(val);
    return 8 + op->getTiming();
}

int32_t BCHG::bchg_dyn_long(int32_t opcode) {
    // for memory destination, the bitnbr is MOD 8 - for data reg destination, the bitnbr is mod 32
    int32_t bit = cpu.getDataRegisterLong((opcode >> 9) & 0x07) & 31;
    bit = static_cast<int32_t>(1u << bit);

    // data register destination
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Long);
    int32_t val = op->getLong();

    // Z_FLAG set according to original value
    if ((val & bit) != 0) {
        cpu.clrFlags(Z_FLAG);
        //clear the bit
        val &= ~(bit);
    } else {
        cpu.setFlags(Z_FLAG);
        //set the bit
        val |= bit;
    }

    op->setLong(val);
    return 8;
}

int32_t BCHG::bchg_static_byte(int32_t opcode) {
    // for memory destination, the bitnbr is MOD 8 - for data reg destination, the bitnbr is mod 32
    int32_t bit = cpu.fetchPCWord() & 7;
    bit = 1 << bit;

    // memory destination
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Byte);
    int32_t val = op->getByte();

    // Z_FLAG set according to original value
    if ((val & bit) != 0) {
        cpu.clrFlags(Z_FLAG);
        //clear the bit
        val &= ~(bit);
    } else {
        cpu.setFlags(Z_FLAG);
        //set the bit
        val |= bit;
    }
    op->setByte(val);
    return 12 + op->getTiming();
}

int32_t BCHG::bchg_static_long(int32_t opcode) {
    // for memory destination, the bitnbr is MOD 8 - for data reg destination, the bitnbr is mod 32
    int32_t bit = cpu.fetchPCWord() & 31;
    bit = static_cast<int32_t>(1u << bit);

    // data register destination
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Long);
    int32_t val = op->getLong();

    // Z_FLAG set according to original value
    if ((val & bit) != 0) {
        cpu.clrFlags(Z_FLAG);
        //clear the bit
        val &= ~(bit);
    } else {
        cpu.setFlags(Z_FLAG);
        //set the bit
        val |= bit;
    }

    op->setLong(val);
    return 12;
}

DisassembledInstruction BCHG::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    int32_t bytes = 2;

    if ((opcode & 0x0100) != 0) {
        //dynamic mode
        DisassembledOperand src("d" + std::to_string((opcode >> 9) & 0x07));

        DisassembledOperand dst =
            cpu.disassembleDstEA(address + bytes, (opcode >> 3) & 0x07, (opcode & 0x07), sz);

        return DisassembledInstruction(address, opcode, "bchg", src, dst);
    }

    //static mode
    int32_t ext = cpu.readMemoryWord(address + 2);
    int32_t val;
    if (((opcode >> 3) & 0x07) == 0) {
        val = ext & 0x1f;
    } else {
        val = ext & 0x07;
    }
    DisassembledOperand src(m68k::util::format("#$%x", val), 2, ext);
    bytes += 2;

    DisassembledOperand dst = cpu.disassembleDstEA(address + bytes, (opcode >> 3) & 0x07, (opcode & 0x07), sz);

    return DisassembledInstruction(address, opcode, "bchg", src, dst);
}

} // namespace m68k::cpu::instructions
