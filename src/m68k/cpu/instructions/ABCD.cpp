#include "m68k/cpu/instructions/ABCD.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

ABCD::ABCD(Cpu& cpu) : cpu(cpu) {}

void ABCD::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t f = 0; f < 2; f++) {
        if (f == 0) {
            // data reg mode
            base = 0xc100;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return abcd_dr(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, true);
                });
        } else {
            // addr reg mode
            base = 0xc108;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return abcd_ar(opcode); },
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

int32_t ABCD::abcd_dr(int32_t opcode) {
    const int32_t sreg = (opcode & 0x07);
    const int32_t dreg = (opcode >> 9) & 0x07;
    const int32_t s = cpu.getDataRegisterByte(sreg);
    const int32_t d = cpu.getDataRegisterByte(dreg);

    const int32_t result = doCalc(s, d);
    cpu.setDataRegisterByte(dreg, result);
    return 6;
}

int32_t ABCD::abcd_ar(int32_t opcode) {
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

int32_t ABCD::doCalc(int32_t s, int32_t d) {
    const int32_t x = (cpu.isFlagSet(X_FLAG) ? 1 : 0);
    int32_t c;

    int32_t lo = (s & 0x0f) + (d & 0x0f) + x;
    if (lo > 9) {
        lo -= 10;
        c = 1;
    } else {
        c = 0;
    }

    int32_t hi = ((s >> 4) & 0x0f) + ((d >> 4) & 0x0f) + c;
    if (hi > 9) {
        hi -= 10;
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

DisassembledInstruction ABCD::disassembleOp(int32_t address, int32_t opcode, bool data_reg_mode) {
    if (data_reg_mode) {
        DisassembledOperand src("d" + std::to_string(opcode & 0x07));
        DisassembledOperand dst("d" + std::to_string((opcode >> 9) & 0x07));
        return DisassembledInstruction(address, opcode, "abcd", src, dst);
    }

    DisassembledOperand src("-(a" + std::to_string(opcode & 0x07) + ")");
    DisassembledOperand dst("-(a" + std::to_string((opcode >> 9) & 0x07) + ")");
    return DisassembledInstruction(address, opcode, "abcd", src, dst);
}

} // namespace m68k::cpu::instructions
