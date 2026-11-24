#include "m68k/cpu/instructions/RTR.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

RTR::RTR(Cpu& cpu) : cpu(cpu) {}

void RTR::registerInstructions(InstructionSet& is) {
    std::shared_ptr<Instruction> i;
    i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) {
            cpu.setCCRegister(cpu.popWord());
            cpu.setPC(cpu.popLong());
            return 20;
        },
        [this](int32_t address, int32_t opcode) {
            return DisassembledInstruction(address, opcode, "rtr");
        });
    is.addInstruction(0x4e77, i);
}

} // namespace m68k::cpu::instructions
