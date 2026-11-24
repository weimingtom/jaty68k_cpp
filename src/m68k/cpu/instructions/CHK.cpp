#include "m68k/cpu/instructions/CHK.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

CHK::CHK(Cpu& cpu) : cpu(cpu) {}

void CHK::registerInstructions(InstructionSet& is) {
    int32_t base = 0x4180;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return chk(opcode); },
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

int32_t CHK::chk(int32_t opcode) {
    int32_t reg = (opcode >> 9) & 0x07;
    int32_t dval = cpu.getDataRegisterWordSigned(reg);

    Operand* op = cpu.resolveSrcEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Word);
    int32_t sval = op->getWord();
    bool raiseException = false;

    if (dval < 0) {
        cpu.setFlags(N_FLAG);
        raiseException = true;
    } else if (dval > sval) {
        cpu.clrFlags(N_FLAG);
        raiseException = true;
    }

    if (raiseException) {
        // CHK exception is vector 6
        cpu.raiseException(6);
        return 40 + op->getTiming();
    }

    return 10 + op->getTiming();
}

DisassembledInstruction CHK::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src("d" + std::to_string((opcode >> 9) & 0x07));
    DisassembledOperand dst = cpu.disassembleDstEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);

    return DisassembledInstruction(address, opcode, "chk", src, dst);
}

} // namespace m68k::cpu::instructions
