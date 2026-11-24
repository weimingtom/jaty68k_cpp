#include "m68k/cpu/instructions/CMPM.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

CMPM::CMPM(Cpu& cpu) : cpu(cpu) {}

void CMPM::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            base = 0xb108;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return cmpm_byte(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            base = 0xb148;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return cmpm_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            base = 0xb188;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return cmpm_long(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }

        for (int32_t ax = 0; ax < 8; ax++) {
            for (int32_t ay = 0; ay < 8; ay++) {
                is.addInstruction(base + (ax << 9) + ay, i);
            }
        }
    }
}

int32_t CMPM::cmpm_byte(int32_t opcode) {
    int32_t ax = (opcode >> 9) & 0x07;
    int32_t ay = (opcode & 0x07);

    int32_t s = cpu.readMemoryByteSigned(cpu.getAddrRegisterLong(ay));
    cpu.incrementAddrRegister(ay, 1);
    int32_t d = cpu.readMemoryByteSigned(cpu.getAddrRegisterLong(ax));
    cpu.incrementAddrRegister(ax, 1);

    int32_t r = d - s;

    cpu.calcFlags(InstructionType::CMP, s, d, r, Size::Byte);

    return 12;
}

int32_t CMPM::cmpm_word(int32_t opcode) {
    int32_t ax = (opcode >> 9) & 0x07;
    int32_t ay = (opcode & 0x07);

    int32_t s = cpu.readMemoryWordSigned(cpu.getAddrRegisterLong(ay));
    cpu.incrementAddrRegister(ay, 2);
    int32_t d = cpu.readMemoryWordSigned(cpu.getAddrRegisterLong(ax));
    cpu.incrementAddrRegister(ax, 2);

    int32_t r = d - s;

    cpu.calcFlags(InstructionType::CMP, s, d, r, Size::Word);

    return 12;
}

int32_t CMPM::cmpm_long(int32_t opcode) {
    int32_t ax = (opcode >> 9) & 0x07;
    int32_t ay = (opcode & 0x07);

    int32_t s = cpu.readMemoryLong(cpu.getAddrRegisterLong(ay));
    cpu.incrementAddrRegister(ay, 4);
    int32_t d = cpu.readMemoryLong(cpu.getAddrRegisterLong(ax));
    cpu.incrementAddrRegister(ax, 4);

    int32_t r = d - s;

    cpu.calcFlags(InstructionType::CMP, s, d, r, Size::Long);

    return 20;
}

DisassembledInstruction CMPM::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    DisassembledOperand src("(a" + std::to_string(opcode & 0x07) + ")+");
    DisassembledOperand dst("(a" + std::to_string((opcode >> 9) & 0x07) + ")+");

    return DisassembledInstruction(address, opcode, std::string("cmpm") + ext(sz), src, dst);
}

} // namespace m68k::cpu::instructions
