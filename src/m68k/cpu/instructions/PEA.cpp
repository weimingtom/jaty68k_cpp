#include "m68k/cpu/instructions/PEA.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

const std::array<int32_t, 11> PEA::TIMING = { 0, 0, 12, 0, 0, 16, 20, 16, 20, 16, 20 };

PEA::PEA(Cpu& cpu) : cpu(cpu) {}

void PEA::registerInstructions(InstructionSet& is) {
    int32_t base = 0x4840;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return pea(opcode); },
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

int32_t PEA::pea(int32_t opcode) {
    Operand* src = cpu.resolveSrcEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Long);
    cpu.pushLong(src->getComputedAddress());

    return TIMING[src->index()];
}

DisassembledInstruction PEA::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src = cpu.disassembleSrcEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);

    return DisassembledInstruction(address, opcode, "pea", src);
}

} // namespace m68k::cpu::instructions
