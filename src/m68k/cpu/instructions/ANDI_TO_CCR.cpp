#include "m68k/cpu/instructions/ANDI_TO_CCR.h"

#include <stdexcept>

#include "m68k/cpu/LambdaInstruction.h"
#include "m68k/util/Format.h"

namespace m68k::cpu::instructions {

ANDI_TO_CCR::ANDI_TO_CCR(Cpu& cpu) : cpu(cpu) {}

void ANDI_TO_CCR::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;
    base = 0x023c;
    i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return andi_word(opcode); },
        [this](int32_t address, int32_t opcode) {
            return disassembleOp(address, opcode, Size::Byte);
        });
    is.addInstruction(base, i);
}

int32_t ANDI_TO_CCR::andi_word(int32_t opcode) {
    //the three upper bytes of the ccr must always be 0
    const int32_t s = cpu.fetchPCWord() & 0x0031; // mask out all but 5 lower bits
    cpu.setSR(cpu.getSR() & (s | 0xff00));
    return 8; // i'm NOT sure about this timing
}

DisassembledInstruction ANDI_TO_CCR::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    int32_t imm_bytes;
    int32_t imm;
    std::string is;

    switch (sz) {
        case Size::Byte: {
            imm = cpu.readMemoryWord(address + 2);
            is = m68k::util::format("#$%02x", static_cast<uint8_t>(imm & 0x00ff));
            imm_bytes = 2;
            break;
        }
        case Size::Word: {
            imm = cpu.readMemoryWord(address + 2);
            is = m68k::util::format("#$%04x", static_cast<uint16_t>(imm));
            imm_bytes = 2;
            break;
        }
        case Size::Long: {
            imm = cpu.readMemoryLong(address + 2);
            is = m68k::util::format("#$%08x", static_cast<uint32_t>(imm));
            imm_bytes = 4;
            break;
        }
        default: {
            throw std::invalid_argument("Size unsized for ANDI");
        }
    }

    DisassembledOperand src(is, imm_bytes, imm);
    DisassembledOperand dst = cpu.disassembleDstEA(address + 2 + imm_bytes, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    return DisassembledInstruction(address, opcode, std::string("andi") + ext(sz), src, dst);
}

} // namespace m68k::cpu::instructions
