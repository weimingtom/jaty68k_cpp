#include "m68k/cpu/instructions/DIVS.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

DIVS::DIVS(Cpu& cpu) : cpu(cpu) {}

void DIVS::registerInstructions(InstructionSet& is) {
    int32_t base = 0x81c0;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return divs(opcode); },
        [this](int32_t address, int32_t opcode) {
            return disassembleOp(address, opcode, Size::Word);
        });

    for (int32_t ea_mode = 0; ea_mode < 8; ea_mode++) {
        if (ea_mode == 1) {
            continue;
        }

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

int32_t DIVS::divs(int32_t opcode) {
    Operand* op = cpu.resolveSrcEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Word);

    int32_t s = op->getWordSigned();
    int32_t reg = (opcode >> 9) & 0x07;
    int32_t d = cpu.getDataRegisterLong(reg);
    int32_t time;

    if (s == 0) {
        //divide by zero exception
        cpu.raiseException(5);
        time = 38 + op->getTiming();
    } else {
        int32_t quot = d / s;

        if (quot > 32767 || quot < -32768) {
            //Overflow
            cpu.setFlags(V_FLAG);
        } else {
            int32_t remain = (d % s) & 0xffff;
            int32_t result = (quot & 0x0000ffff) | (remain << 16);
            cpu.setDataRegisterLong(reg, result);

            if ((quot & 0x8000) != 0) {
                cpu.setFlags(N_FLAG);
                cpu.clrFlags(Z_FLAG);
            } else {
                cpu.clrFlags(N_FLAG);

                if (quot == 0) {
                    cpu.setFlags(Z_FLAG);
                } else {
                    cpu.clrFlags(Z_FLAG);
                }
            }

            cpu.clrFlags((V_FLAG | C_FLAG));
        }
        //worst case but less than 10% difference between best and worst cases
        time = 158 + op->getTiming();
    }

    return time;
}

DisassembledInstruction DIVS::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src = cpu.disassembleSrcEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    DisassembledOperand dst("d" + std::to_string((opcode >> 9) & 0x07));

    return DisassembledInstruction(address, opcode, "divs", src, dst);
}

} // namespace m68k::cpu::instructions
