#include "m68k/cpu/instructions/UNLK.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

UNLK::UNLK(Cpu& cpu) : cpu(cpu) {}

void UNLK::registerInstructions(InstructionSet& is) {
    int32_t base = 0x4e58;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return unlk(opcode); },
        [this](int32_t address, int32_t opcode) { return disassembleOp(address, opcode); });

    for (int32_t reg = 0; reg < 8; reg++) {
        is.addInstruction(base + reg, i);
    }
}

int32_t UNLK::unlk(int32_t opcode) {
    const int32_t reg = (opcode & 0x007);

    cpu.setAddrRegisterLong(7, cpu.getAddrRegisterLong(reg));
    cpu.setAddrRegisterLong(reg, cpu.popLong());
    return 12;
}

DisassembledInstruction UNLK::disassembleOp(int32_t address, int32_t opcode) {
    DisassembledOperand src("a" + std::to_string((opcode & 0x07)));
    return DisassembledInstruction(address, opcode, "unlk", src);
}

} // namespace m68k::cpu::instructions
