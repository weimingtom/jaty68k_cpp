#include "m68k/cpu/instructions/JSR.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

const std::array<int32_t, 11> JSR::TIMING = { 0, 0, 16, 0, 0, 18, 22, 18, 20, 18, 22 };

JSR::JSR(Cpu& cpu) : cpu(cpu) {}

void JSR::registerInstructions(InstructionSet& is) {
    int32_t base = 0x4e80;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return jsr(opcode); },
        [this](int32_t address, int32_t opcode) {
            return disassembleOp(address, opcode, Size::Long);
        });

    for (int32_t ea_mode = 2; ea_mode < 8; ea_mode++) {
        if (ea_mode == 3 || ea_mode == 4) {
            continue;
        }

        for (int32_t ea_reg = 0; ea_reg < 8; ea_reg++) {
            if (ea_mode == 7 && ea_reg > 3) {
                break;
            }
            is.addInstruction(base + (ea_mode << 3) + ea_reg, i);
        }
    }
}

int32_t JSR::jsr(int32_t opcode) {
    const int32_t mode = (opcode >> 3) & 0x07;
    const int32_t reg = (opcode & 0x07);
    Operand* op = cpu.resolveSrcEA(mode, reg, Size::Long);
    cpu.pushLong(cpu.getPC());
    cpu.setPC(op->getComputedAddress());

    int32_t idx;
    if (mode == 7) {
        idx = mode + reg;
    } else {
        idx = mode;
    }

    return TIMING[idx];
}

DisassembledInstruction JSR::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand op = cpu.disassembleSrcEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    return DisassembledInstruction(address, opcode, "jsr", op);
}

} // namespace m68k::cpu::instructions
