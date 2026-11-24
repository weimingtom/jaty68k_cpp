#include "m68k/cpu/instructions/MOVE_TO_CCR.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

MOVE_TO_CCR::MOVE_TO_CCR(Cpu& cpu) : cpu(cpu) {}

void MOVE_TO_CCR::registerInstructions(InstructionSet& is) {
    // move to ccr
    int32_t base = 0x44c0;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return move_to_ccr(opcode); },
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

            is.addInstruction(base + (ea_mode << 3) + ea_reg, i);
        }
    }
}

int32_t MOVE_TO_CCR::move_to_ccr(int32_t opcode) {
    // the upper three bits of the ccr must always be 0
    Operand* src = cpu.resolveSrcEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Word);
    const int32_t s = src->getWord() & 31;
    cpu.setCCRegister(s);
    return 12 + src->getTiming();
}

DisassembledInstruction MOVE_TO_CCR::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src = cpu.disassembleSrcEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    DisassembledOperand dst("ccr");

    return DisassembledInstruction(address, opcode, "move", src, dst);
}

} // namespace m68k::cpu::instructions
