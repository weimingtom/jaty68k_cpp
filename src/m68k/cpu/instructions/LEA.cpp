#include "m68k/cpu/instructions/LEA.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

const std::array<int32_t, 11> LEA::TIMING = { 0, 0, 4, 0, 0, 8, 12, 8, 12, 8, 12 };

LEA::LEA(Cpu& cpu) : cpu(cpu) {}

void LEA::registerInstructions(InstructionSet& is) {
    int32_t base = 0x41c0;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return lea(opcode); },
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

            for (int32_t r = 0; r < 8; r++) {
                is.addInstruction(base + (r << 9) + (ea_mode << 3) + ea_reg, i);
            }
        }
    }
}

int32_t LEA::lea(int32_t opcode) {
    Operand* src = cpu.resolveSrcEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Long);
    cpu.setAddrRegisterLong((opcode >> 9) & 0x07, src->getComputedAddress());

    return TIMING[src->index()];
}

DisassembledInstruction LEA::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src = cpu.disassembleSrcEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    DisassembledOperand dst("a" + std::to_string((opcode >> 9) & 0x07));

    return DisassembledInstruction(address, opcode, "lea", src, dst);
}

} // namespace m68k::cpu::instructions
