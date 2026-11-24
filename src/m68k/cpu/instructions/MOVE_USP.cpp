#include "m68k/cpu/instructions/MOVE_USP.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

MOVE_USP::MOVE_USP(Cpu& cpu) : cpu(cpu) {}

void MOVE_USP::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t t = 0; t < 2; t++) {
        if (t == 0) {
            // move to usp
            base = 0x4e60;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return move_to_usp(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, true);
                });
        } else {
            //move from usp
            base = 0x4e68;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return move_from_usp(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, false);
                });
        }

        for (int32_t reg = 0; reg < 8; reg++) {
            is.addInstruction(base + reg, i);
        }
    }
}

int32_t MOVE_USP::move_to_usp(int32_t opcode) {
    if (!cpu.isSupervisorMode()) {
        cpu.raiseSRException();
        return 34;
    }

    cpu.setUSP(cpu.getAddrRegisterLong(opcode & 0x07));
    return 4;
}

int32_t MOVE_USP::move_from_usp(int32_t opcode) {
    if (!cpu.isSupervisorMode()) {
        cpu.raiseSRException();
        return 34;
    }

    cpu.setAddrRegisterLong(opcode & 0x07, cpu.getUSP());
    return 4;
}

DisassembledInstruction MOVE_USP::disassembleOp(int32_t address, int32_t opcode, bool move_to_usp) {
    if (move_to_usp) {
        //move to usp
        DisassembledOperand src("a" + std::to_string(opcode & 0x07));
        DisassembledOperand dst("usp");
        return DisassembledInstruction(address, opcode, "move", src, dst);
    }

    // move from usp
    DisassembledOperand src("usp");
    DisassembledOperand dst("a" + std::to_string(opcode & 0x07));

    return DisassembledInstruction(address, opcode, "move", src, dst);
}

} // namespace m68k::cpu::instructions
