#include "m68k/cpu/instructions/TRAPV.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

TRAPV::TRAPV(Cpu& cpu) : cpu(cpu) {}

void TRAPV::registerInstructions(InstructionSet& is) {
    std::shared_ptr<Instruction> i;
    i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return trapv(opcode); },
        [this](int32_t address, int32_t opcode) { return disassembleOp(address, opcode); });
    is.addInstruction(0x4e76, i);
}

int32_t TRAPV::trapv(int32_t opcode) {
    if (cpu.isFlagSet(V_FLAG)) {
        cpu.raiseException(7);
        return 34;
    }

    return 4;
}

DisassembledInstruction TRAPV::disassembleOp(int32_t address, int32_t opcode) {
    return DisassembledInstruction(address, opcode, "trapv");
}

} // namespace m68k::cpu::instructions
