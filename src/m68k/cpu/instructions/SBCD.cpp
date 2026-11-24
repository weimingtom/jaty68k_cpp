#include "m68k/cpu/instructions/SBCD.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

SBCD::SBCD(Cpu& cpu) : cpu(cpu) {}

void SBCD::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t f = 0; f < 2; f++) {
        if (f == 0) {
            // data reg mode
            base = 0x8100;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return sbcd_dr(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, true);
                });
        } else {
            // addr reg mode
            base = 0x8108;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return sbcd_ar(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, false);
                });
        }

        for (int32_t d = 0; d < 8; d++) {
            for (int32_t s = 0; s < 8; s++) {
                is.addInstruction(base + (d << 9) + s, i);
            }
        }
    }
}

int32_t SBCD::sbcd_dr(int32_t opcode) {
    const int32_t sreg = (opcode & 0x07);
    const int32_t dreg = (opcode >> 9) & 0x07;
    const int32_t s = cpu.getDataRegisterByte(sreg);
    const int32_t d = cpu.getDataRegisterByte(dreg);

    const int32_t result = doCalc(s, d);
    cpu.setDataRegisterByte(dreg, result);
    return 6;
}

int32_t SBCD::sbcd_ar(int32_t opcode) {
    const int32_t sreg = (opcode & 0x07);
    const int32_t dreg = (opcode >> 9) & 0x07;
    cpu.decrementAddrRegister(sreg, 1);
    cpu.decrementAddrRegister(dreg, 1);
    const int32_t s = cpu.readMemoryByte(cpu.getAddrRegisterLong(sreg));
    const int32_t d = cpu.readMemoryByte(cpu.getAddrRegisterLong(dreg));

    const int32_t result = doCalc(s, d);
    cpu.writeMemoryByte(cpu.getAddrRegisterLong(dreg), result);
    return 18;
}

int32_t SBCD::doCalc(int32_t s, int32_t d) {
    const int32_t x = (cpu.isFlagSet(X_FLAG) ? 1 : 0);
    int32_t c;

    int32_t lo = (d & 0x0f) - (s & 0x0f) - x;
    if (lo < 0) {
        lo += 10;
        c = 1;
    } else {
        c = 0;
    }

    int32_t hi = ((d >> 4) & 0x0f) - ((s >> 4) & 0x0f) - c;
    if (hi < 0) {
        hi += 10;
        c = 1;
    } else {
        c = 0;
    }

    const int32_t result = (hi << 4) + lo;

    if (c != 0) {
        cpu.setFlags(X_FLAG | C_FLAG);
    } else {
        cpu.clrFlags(X_FLAG | C_FLAG);
    }

    if (result != 0) {
        cpu.clrFlags(Z_FLAG);
    }

    return result;
}

DisassembledInstruction SBCD::disassembleOp(int32_t address, int32_t opcode, bool data_reg_mode) {
    if (data_reg_mode) {
        DisassembledOperand src("d" + std::to_string(opcode & 0x07));
        DisassembledOperand dst("d" + std::to_string((opcode >> 9) & 0x07));
        return DisassembledInstruction(address, opcode, "sbcd", src, dst);
    }

    DisassembledOperand src("-(a" + std::to_string(opcode & 0x07) + ")");
    DisassembledOperand dst("-(a" + std::to_string((opcode >> 9) & 0x07) + ")");
    return DisassembledInstruction(address, opcode, "sbcd", src, dst);
}

} // namespace m68k::cpu::instructions
