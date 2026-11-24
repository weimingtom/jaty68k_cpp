#include "m68k/cpu/instructions/MOVEQ.h"

#include "m68k/cpu/CpuUtils.h"
#include "m68k/cpu/LambdaInstruction.h"
#include "m68k/util/Format.h"

namespace m68k::cpu::instructions {

MOVEQ::MOVEQ(Cpu& cpu) : cpu(cpu) {}

void MOVEQ::registerInstructions(InstructionSet& is) {
    int32_t base = 0x7000;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return moveq(opcode); },
        [this](int32_t address, int32_t opcode) { return disassembleOp(address, opcode); });

    for (int32_t reg = 0; reg < 8; reg++) {
        for (int32_t imm = 0; imm < 256; imm++) {
            is.addInstruction(base + (reg << 9) + imm, i);
        }
    }
}

int32_t MOVEQ::moveq(int32_t opcode) {
    int32_t reg = (opcode >> 9) & 0x07;
    int32_t data = signExtendWord(signExtendByte(opcode & 0xff));
    cpu.setDataRegisterLong(reg, data);
    if (data < 0) {
        cpu.setFlags(N_FLAG);
    } else {
        cpu.clrFlags(N_FLAG);
    }
    if (data == 0) {
        cpu.setFlags(Z_FLAG);
    } else {
        cpu.clrFlags(Z_FLAG);
    }
    cpu.clrFlags(C_FLAG | V_FLAG);
    return 4;
}

DisassembledInstruction MOVEQ::disassembleOp(int32_t address, int32_t opcode) {
    DisassembledOperand src(m68k::util::format("#$%02x", opcode & 0xff));
    DisassembledOperand dst("d" + std::to_string((opcode >> 9) & 0x07));

    return DisassembledInstruction(address, opcode, "moveq", src, dst);
}

} // namespace m68k::cpu::instructions
