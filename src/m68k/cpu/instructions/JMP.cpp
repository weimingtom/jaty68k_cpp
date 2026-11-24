#include "m68k/cpu/instructions/JMP.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

const std::array<int32_t, 11> JMP::TIMING = { 0, 0, 8, 0, 0, 10, 14, 10, 12, 10, 14 };

JMP::JMP(Cpu& cpu) : cpu(cpu) {}

void JMP::registerInstructions(InstructionSet& is) {
    int32_t base = 0x4ec0;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return jmp(opcode); },
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

int32_t JMP::jmp(int32_t opcode) {
    Operand* op = cpu.resolveSrcEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Long);
    cpu.setPC(op->getComputedAddress());

    return TIMING[op->index()];
}

DisassembledInstruction JMP::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand op = cpu.disassembleSrcEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    return DisassembledInstruction(address, opcode, "jmp", op);
}

} // namespace m68k::cpu::instructions
