#include "m68k/cpu/instructions/SWAP.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

SWAP::SWAP(Cpu& cpu) : cpu(cpu) {}

void SWAP::registerInstructions(InstructionSet& is) {
    int32_t base = 0x4840;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return swap(opcode); },
        [this](int32_t address, int32_t opcode) { return disassembleOp(address, opcode); });

    for (int32_t reg = 0; reg < 8; reg++) {
        is.addInstruction(base + reg, i);
    }
}

int32_t SWAP::swap(int32_t opcode) {
    const int32_t reg = (opcode & 0x007);
    int32_t v = cpu.getDataRegisterLong(reg);
    const int32_t vh = (v >> 16) & 0x0000ffff;
    v = (v << 16) + vh;
    cpu.setDataRegisterLong(reg, v);
    // swap affects the registers : z set if result = 0, N set if bit 31 set, v & c always cleared
    cpu.calcFlags(InstructionType::SWAP, v, v, v, Size::Long);
    return 4;
}

DisassembledInstruction SWAP::disassembleOp(int32_t address, int32_t opcode) {
    DisassembledOperand src("d" + std::to_string((opcode & 0x07)));
    return DisassembledInstruction(address, opcode, "swap", src);
}

} // namespace m68k::cpu::instructions
