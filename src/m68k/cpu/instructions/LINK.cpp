#include "m68k/cpu/instructions/LINK.h"

#include "m68k/cpu/LambdaInstruction.h"
#include "m68k/util/Format.h"

namespace m68k::cpu::instructions {

LINK::LINK(Cpu& cpu) : cpu(cpu) {}

void LINK::registerInstructions(InstructionSet& is) {
    int32_t base = 0x4e50;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return link(opcode); },
        [this](int32_t address, int32_t opcode) { return disassembleOp(address, opcode); });

    for (int32_t reg = 0; reg < 8; reg++) {
        is.addInstruction(base + reg, i);
    }
}

int32_t LINK::link(int32_t opcode) {
    const int32_t sreg = (opcode & 0x007);

    //signed displacement
    const int32_t displacement = cpu.fetchPCWordSigned();

    //push the address reg
    cpu.pushLong(cpu.getAddrRegisterLong(sreg));

    //copy the stack pointer to the address reg
    const int32_t sp = cpu.getAddrRegisterLong(7);
    cpu.setAddrRegisterLong(sreg, sp);

    //add the displacement to the stack pointer
    cpu.setAddrRegisterLong(7, sp + displacement);
    return 16;
}

DisassembledInstruction LINK::disassembleOp(int32_t address, int32_t opcode) {
    DisassembledOperand src("a" + std::to_string((opcode & 0x07)));
    const int32_t dis = cpu.readMemoryWordSigned(address + 2);
    DisassembledOperand dst(m68k::util::format("#$%04x", dis), 2, dis);

    return DisassembledInstruction(address, opcode, "link", src, dst);
}

} // namespace m68k::cpu::instructions
