#include "m68k/cpu/instructions/MOVE_TO_SR.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

MOVE_TO_SR::MOVE_TO_SR(Cpu& cpu) : cpu(cpu) {}

void MOVE_TO_SR::registerInstructions(InstructionSet& is) {
    // move to sr
    int32_t base = 0x46c0;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return move_to_sr(opcode); },
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

int32_t MOVE_TO_SR::move_to_sr(int32_t opcode) {
    Operand* src = cpu.resolveSrcEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Word);
    const int32_t s = src->getWord() & 0xf71f; // mask out bits 5,6,7 & 11 : they are always 0!;
    if (!cpu.isSupervisorMode()) {
        cpu.raiseSRException();
        return 34;
    }

    cpu.setSR(s);
    return 12 + src->getTiming();
}

DisassembledInstruction MOVE_TO_SR::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src = cpu.disassembleSrcEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    DisassembledOperand dst("sr");

    return DisassembledInstruction(address, opcode, "move", src, dst);
}

} // namespace m68k::cpu::instructions
