#include "m68k/cpu/instructions/RTS.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

RTS::RTS(Cpu& cpu) : cpu(cpu) {}

void RTS::registerInstructions(InstructionSet& is) {
    std::shared_ptr<Instruction> i;
    i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) {
            cpu.setPC(cpu.popLong());
            return 16;
        },
        [this](int32_t address, int32_t opcode) {
            return DisassembledInstruction(address, opcode, "rts");
        });
    is.addInstruction(0x4e75, i);
}

} // namespace m68k::cpu::instructions
