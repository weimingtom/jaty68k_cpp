#include "m68k/cpu/instructions/TRAP.h"

#include "m68k/cpu/LambdaInstruction.h"
#include "m68k/util/Format.h"

namespace m68k::cpu::instructions {

TRAP::TRAP(Cpu& cpu) : cpu(cpu) {}

void TRAP::registerInstructions(InstructionSet& is) {
    int32_t base = 0x4e40;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return trap(opcode); },
        [this](int32_t address, int32_t opcode) { return disassembleOp(address, opcode); });

    for (int32_t v = 0; v < 16; v++) {
        is.addInstruction(base + v, i);
    }
}

int32_t TRAP::trap(int32_t opcode) {
    const int32_t v = (opcode & 0x0f);
    cpu.raiseException(32 + v);
    return 34;
}

DisassembledInstruction TRAP::disassembleOp(int32_t address, int32_t opcode) {
    DisassembledOperand op(m68k::util::format("#%d", (opcode & 0x0f)));
    return DisassembledInstruction(address, opcode, "trap", op);
}

} // namespace m68k::cpu::instructions
