#include "m68k/cpu/instructions/Scc.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

const std::array<std::string, 16> Scc::names = { "st", "sf", "shi", "sls", "scc", "scs", "sne", "seq",
                                                 "svc", "svs", "spl", "smi", "sge", "slt", "sgt", "sle" };

Scc::Scc(Cpu& cpu) : cpu(cpu) {}

void Scc::registerInstructions(InstructionSet& is) {
    int32_t base = 0x50c0;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return sxx(opcode); },
        [this](int32_t address, int32_t opcode) { return disassembleOp(address, opcode); });

    for (int32_t ea_mode = 0; ea_mode < 8; ea_mode++) {
        if (ea_mode == 1)
            continue;
        for (int32_t ea_reg = 0; ea_reg < 8; ea_reg++) {
            if (ea_mode == 7 && ea_reg > 1)
                break;

            for (int32_t cc = 0; cc < 16; cc++) {
                is.addInstruction(base + (cc << 8) + (ea_mode << 3) + ea_reg, i);
            }
        }
    }
}

int32_t Scc::sxx(int32_t opcode) {
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Byte);
    const int32_t cc = (opcode >> 8) & 0x0f;
    int32_t time;

    if (cpu.testCC(cc)) {
        op->setByte(0xff);
        time = (op->isRegisterMode() ? 6 : 8 + op->getTiming());
    } else {
        // condition failed
        op->setByte(0);
        time = (op->isRegisterMode() ? 4 : 8 + op->getTiming());
    }

    return time;
}

DisassembledInstruction Scc::disassembleOp(int32_t address, int32_t opcode) {
    DisassembledOperand op = cpu.disassembleDstEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), Size::Byte);
    return DisassembledInstruction(address, opcode, names[(opcode >> 8) & 0x0f], op);
}

} // namespace m68k::cpu::instructions
