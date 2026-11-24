#include "m68k/cpu/instructions/Bcc.h"

#include "m68k/cpu/CpuUtils.h"
#include "m68k/cpu/LambdaInstruction.h"
#include "m68k/util/Format.h"

namespace m68k::cpu::instructions {

const std::array<std::string, 16> Bcc::names = { "bra", "bsr", "bhi", "bls", "bcc", "bcs", "bne", "beq",
                                                 "bvc", "bvs", "bpl", "bmi", "bge", "blt", "bgt", "ble" };

Bcc::Bcc(Cpu& cpu) : cpu(cpu) {}

void Bcc::registerInstructions(InstructionSet& is) {
    int32_t base = 0x6000;
    std::shared_ptr<Instruction> ib = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return bxx_byte(opcode); },
        [this](int32_t address, int32_t opcode) { return disassembleOp(address, opcode); });

    std::shared_ptr<Instruction> iw = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return bxx_word(opcode); },
        [this](int32_t address, int32_t opcode) { return disassembleOp(address, opcode); });

    for (int32_t cc = 0; cc < 16; cc++) {
        is.addInstruction(base + (cc << 8), iw);

        for (int32_t dis = 1; dis < 256; dis++) {
            is.addInstruction(base + (cc << 8) + dis, ib);
        }
    }
}

int32_t Bcc::bxx_byte(int32_t opcode) {
    int32_t dis = signExtendByte(opcode & 0xff);
    int32_t cc = (opcode >> 8) & 0x0f;
    int32_t pc = cpu.getPC();
    int32_t time;

    if (cc == 1) {
        //bsr
        cpu.pushLong(pc);
        cpu.setPC(pc + dis);
        time = 18;
    } else {
        if (cpu.testCC(cc)) {
            cpu.setPC(pc + dis);
            time = 10;
        } else {
            // condition failed
            time = 8;
        }
    }
    return time;
}

int32_t Bcc::bxx_word(int32_t opcode) {
    int32_t pc = cpu.getPC();
    int32_t dis = cpu.readMemoryWordSigned(pc);
    int32_t cc = (opcode >> 8) & 0x0f;
    int32_t time;

    if (cc == 1) {
        //bsr
        cpu.pushLong(pc + 2);
        cpu.setPC(pc + dis);
        time = 18;
    } else {
        if (cpu.testCC(cc)) {
            cpu.setPC(pc + dis);
            time = 10;
        } else {
            // condition failed
            time = 12;
            cpu.setPC(pc + 2);
        }
    }
    return time;
}

DisassembledInstruction Bcc::disassembleOp(int32_t address, int32_t opcode) {
    int32_t cc = (opcode >> 8) & 0x0f;
    int32_t dis = signExtendByte(opcode & 0xff);
    std::string name;

    if (dis != 0) {
        DisassembledOperand op(m68k::util::format("$%08x", dis + address + 2));
        name = names[cc] + ".s";
        return DisassembledInstruction(address, opcode, name, op);
    } else {
        //word displacement
        dis = cpu.readMemoryWordSigned(address + 2);
        DisassembledOperand op(m68k::util::format("$%08x", dis + address + 2), 2, dis);
        name = names[cc] + ".w";
        return DisassembledInstruction(address, opcode, name, op);
    }
}

} // namespace m68k::cpu::instructions
