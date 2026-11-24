#include "m68k/cpu/instructions/NOP.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

NOP::NOP(Cpu& cpu) : cpu(cpu) {}

void NOP::registerInstructions(InstructionSet& is) {
    std::shared_ptr<Instruction> i;
    i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return 4; },
        [this](int32_t address, int32_t opcode) {
            return DisassembledInstruction(address, opcode, "nop");
        });
    is.addInstruction(0x4e71, i);
}

} // namespace m68k::cpu::instructions
