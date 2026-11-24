#include "m68k/cpu/instructions/MOVEP.h"

#include "m68k/cpu/LambdaInstruction.h"
#include "m68k/util/Format.h"

namespace m68k::cpu::instructions {

MOVEP::MOVEP(Cpu& cpu) : cpu(cpu) {}

void MOVEP::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    // r2m
    for (int32_t sz = 0; sz < 2; sz++) {
        if (sz == 0) {
            //word
            base = 0x0188;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return r2m_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word, true);
                });
        } else {
            // long
            base = 0x01c8;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return r2m_long(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long, true);
                });
        }

        for (int32_t dreg = 0; dreg < 8; dreg++) {
            for (int32_t areg = 0; areg < 8; areg++) {
                is.addInstruction(base + (dreg << 9) + areg, i);
            }
        }
    }

    // m2r
    for (int32_t sz = 0; sz < 2; sz++) {
        if (sz == 0) {
            //word
            base = 0x0108;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return m2r_word(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word, false);
                });
        } else {
            // long
            base = 0x0148;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return m2r_long(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long, false);
                });
        }

        for (int32_t dreg = 0; dreg < 8; dreg++) {
            for (int32_t areg = 0; areg < 8; areg++) {
                is.addInstruction(base + (dreg << 9) + areg, i);
            }
        }
    }
}

int32_t MOVEP::r2m_word(int32_t opcode) {
    int32_t dis = cpu.fetchPCWordSigned();
    int32_t address = cpu.getAddrRegisterLong(opcode & 0x07) + dis;
    int32_t val = cpu.getDataRegisterWord((opcode >> 9) & 0x07);
    cpu.writeMemoryByte(address, (static_cast<uint32_t>(val) >> 8) & 0xff);
    cpu.writeMemoryByte(address + 2, val & 0xff);
    return 16;
}

int32_t MOVEP::r2m_long(int32_t opcode) {
    int32_t dis = cpu.fetchPCWordSigned();
    int32_t address = cpu.getAddrRegisterLong(opcode & 0x07) + dis;
    int32_t val = cpu.getDataRegisterWord((opcode >> 9) & 0x07);
    cpu.writeMemoryByte(address, (static_cast<uint32_t>(val) >> 24) & 0xff);
    cpu.writeMemoryByte(address + 2, (static_cast<uint32_t>(val) >> 16) & 0xff);
    cpu.writeMemoryByte(address + 4, (static_cast<uint32_t>(val) >> 8) & 0xff);
    cpu.writeMemoryByte(address + 6, val & 0xff);
    return 24;
}

int32_t MOVEP::m2r_word(int32_t opcode) {
    int32_t dis = cpu.fetchPCWordSigned();
    int32_t address = cpu.getAddrRegisterLong(opcode & 0x07) + dis;
    int32_t val = 0;

    val = cpu.readMemoryByte(address) << 8;
    val |= cpu.readMemoryByte(address + 2);

    cpu.setDataRegisterWord((opcode >> 9) & 0x07, val);
    return 16;
}

int32_t MOVEP::m2r_long(int32_t opcode) {
    int32_t dis = cpu.fetchPCWordSigned();
    int32_t address = cpu.getAddrRegisterLong(opcode & 0x07) + dis;
    int32_t val = 0;

    val = cpu.readMemoryByte(address) << 24;
    val |= cpu.readMemoryByte(address + 2) << 16;
    val |= cpu.readMemoryByte(address + 4) << 8;
    val |= cpu.readMemoryByte(address + 6);

    cpu.setDataRegisterLong((opcode >> 9) & 0x07, val);
    return 24;
}

DisassembledInstruction MOVEP::disassembleOp(int32_t address, int32_t opcode, Size sz, bool r2m) {
    int32_t dis = cpu.readMemoryWordSigned(address + 2);

    if (r2m) {
        DisassembledOperand src("d" + std::to_string((opcode >> 9) & 0x07));
        DisassembledOperand dst(m68k::util::format("#$%04x(a%d)", dis, (opcode & 0x07)), 2, dis);
        return DisassembledInstruction(address, opcode, std::string("movep") + ext(sz), src, dst);
    } else {
        DisassembledOperand src(m68k::util::format("#$%04x(a%d)", dis, (opcode & 0x07)), 2, dis);
        DisassembledOperand dst("d" + std::to_string((opcode >> 9) & 0x07));
        return DisassembledInstruction(address, opcode, std::string("movep") + ext(sz), src, dst);
    }
}

} // namespace m68k::cpu::instructions
