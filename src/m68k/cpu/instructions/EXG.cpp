#include "m68k/cpu/instructions/EXG.h"

#include <stdexcept>

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

EXG::EXG(Cpu& cpu) : cpu(cpu) {}

void EXG::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    // destination ea
    for (int32_t mode = 0; mode < 3; mode++) {
        if (mode == 0) {
            // exg data regs
            base = 0xc140;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return exg_dd(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, ExgMode::EXG_DATA);
                });
        } else if (mode == 1) {
            // exg address regs
            base = 0xc148;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return exg_aa(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, ExgMode::EXG_ADDR);
                });
        } else {
            // exg data addr regs
            base = 0xc188;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return exg_da(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, ExgMode::EXG_DATA_ADDR);
                });
        }
        for (int32_t rx = 0; rx < 8; rx++) {
            for (int32_t ry = 0; ry < 8; ry++) {
                is.addInstruction(base + (rx << 9) + ry, i);
            }
        }
    }
}

int32_t EXG::exg_dd(int32_t opcode) {
    const int32_t rx = (opcode >> 9) & 0x07;
    const int32_t ry = (opcode & 0x07);

    const int32_t x = cpu.getDataRegisterLong(rx);
    const int32_t y = cpu.getDataRegisterLong(ry);
    cpu.setDataRegisterLong(rx, y);
    cpu.setDataRegisterLong(ry, x);
    return 6;
}

int32_t EXG::exg_aa(int32_t opcode) {
    const int32_t rx = (opcode >> 9) & 0x07;
    const int32_t ry = (opcode & 0x07);

    const int32_t x = cpu.getAddrRegisterLong(rx);
    const int32_t y = cpu.getAddrRegisterLong(ry);
    cpu.setAddrRegisterLong(rx, y);
    cpu.setAddrRegisterLong(ry, x);
    return 6;
}

int32_t EXG::exg_da(int32_t opcode) {
    const int32_t rx = (opcode >> 9) & 0x07;
    const int32_t ry = (opcode & 0x07);

    const int32_t x = cpu.getDataRegisterLong(rx);
    const int32_t y = cpu.getAddrRegisterLong(ry);
    cpu.setDataRegisterLong(rx, y);
    cpu.setAddrRegisterLong(ry, x);
    return 6;
}

DisassembledInstruction EXG::disassembleOp(int32_t address, int32_t opcode, ExgMode mode) {
    switch (mode) {
        case ExgMode::EXG_DATA: {
            DisassembledOperand src("d" + std::to_string((opcode >> 9) & 0x07));
            DisassembledOperand dst("d" + std::to_string(opcode & 0x07));
            return DisassembledInstruction(address, opcode, "exg", src, dst);
        }
        case ExgMode::EXG_ADDR: {
            DisassembledOperand src("a" + std::to_string((opcode >> 9) & 0x07));
            DisassembledOperand dst("a" + std::to_string(opcode & 0x07));
            return DisassembledInstruction(address, opcode, "exg", src, dst);
        }
        case ExgMode::EXG_DATA_ADDR: {
            DisassembledOperand src("d" + std::to_string((opcode >> 9) & 0x07));
            DisassembledOperand dst("a" + std::to_string(opcode & 0x07));
            return DisassembledInstruction(address, opcode, "exg", src, dst);
        }
        default: {
            throw std::invalid_argument("Invalid exg type specified");
        }
    }
}

} // namespace m68k::cpu::instructions
