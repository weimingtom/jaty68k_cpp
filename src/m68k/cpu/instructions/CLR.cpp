#include "m68k/cpu/instructions/CLR.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

CLR::CLR(Cpu& cpu) : cpu(cpu) {}

void CLR::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            base = 0x4200;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return clr_byte(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            base = 0x4240;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return clr_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            base = 0x4280;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return clr_long(opcode); },
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

int32_t CLR::clr_byte(int32_t opcode) {
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Byte);

    //anomaly as the memory is always read (might affect mapped memory) before being cleared
    op->getByte();
    op->setByte(0);

    int32_t flags = cpu.getCCRegister();
    flags &= ~(N_FLAG | C_FLAG | V_FLAG);
    flags |= Z_FLAG;
    cpu.setCCRegister(flags);

    return (op->isRegisterMode() ? 4 : 8 + op->getTiming());
}

int32_t CLR::clr_word(int32_t opcode) {
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Word);

    //anomaly as the memory is always read (might affect mapped memory) before being cleared
    op->getWord();
    op->setWord(0);

    int32_t flags = cpu.getCCRegister();
    flags &= ~(N_FLAG | C_FLAG | V_FLAG);
    flags |= Z_FLAG;
    cpu.setCCRegister(flags);

    return (op->isRegisterMode() ? 4 : 8 + op->getTiming());
}

int32_t CLR::clr_long(int32_t opcode) {
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Long);

    //anomaly as the memory is always read (might affect mapped memory) before being cleared
    op->getLong();
    op->setLong(0);

    int32_t flags = cpu.getCCRegister();
    flags &= ~(N_FLAG | C_FLAG | V_FLAG);
    flags |= Z_FLAG;
    cpu.setCCRegister(flags);

    return (op->isRegisterMode() ? 6 : 12 + op->getTiming());
}

DisassembledInstruction CLR::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand dst = cpu.disassembleDstEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);

    return DisassembledInstruction(address, opcode, std::string("clr") + ext(sz), dst);
}

} // namespace m68k::cpu::instructions
