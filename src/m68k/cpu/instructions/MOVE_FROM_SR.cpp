#include "m68k/cpu/instructions/MOVE_FROM_SR.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

MOVE_FROM_SR::MOVE_FROM_SR(Cpu& cpu) : cpu(cpu) {}

void MOVE_FROM_SR::registerInstructions(InstructionSet& is) {
    // move from sr
    int32_t base = 0x40c0;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return move_from_sr(opcode); },
        [this](int32_t address, int32_t opcode) {
            return disassembleOp(address, opcode, Size::Word);
        });

    for (int32_t ea_mode = 0; ea_mode < 8; ea_mode++) {
        if (ea_mode == 1) {
            continue;
        }

        for (int32_t ea_reg = 0; ea_reg < 8; ea_reg++) {
            if (ea_mode == 7 && ea_reg > 1) {
                break;
            }

            is.addInstruction(base + (ea_mode << 3) + ea_reg, i);
        }
    }
}

int32_t MOVE_FROM_SR::move_from_sr(int32_t opcode) {
    Operand* dst = cpu.resolveDstEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Word);
    // read before writing
    dst->getWord();
    dst->setWord(cpu.getSR());
    return (dst->isRegisterMode() ? 6 : 8 + dst->getTiming());
}

DisassembledInstruction MOVE_FROM_SR::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src("sr");
    DisassembledOperand dst = cpu.disassembleDstEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);

    return DisassembledInstruction(address, opcode, "move", src, dst);
}

} // namespace m68k::cpu::instructions
