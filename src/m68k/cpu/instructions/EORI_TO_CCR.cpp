#include "m68k/cpu/instructions/EORI_TO_CCR.h"

#include "m68k/cpu/LambdaInstruction.h"
#include "m68k/util/Format.h"

namespace m68k::cpu::instructions {

EORI_TO_CCR::EORI_TO_CCR(Cpu& cpu) : cpu(cpu) {}

void EORI_TO_CCR::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;
    base = 0x0a3c;
    i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return eori_word(opcode); },
        [this](int32_t address, int32_t opcode) {
            return disassembleOp(address, opcode, Size::Word);
        });
    is.addInstruction(base, i);
}

int32_t EORI_TO_CCR::eori_word(int32_t opcode) {
    // mask out bits 5-7,they are always 0
    int32_t s = cpu.fetchPCWordSigned() & 31;
    const int32_t sr = cpu.getCCRegister();
    s = s ^ (sr & 0x00ff);
    cpu.setCCRegister(s);
    return 8;
}

DisassembledInstruction EORI_TO_CCR::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    int32_t imm_bytes;
    int32_t imm;
    std::string is;
    imm = cpu.readMemoryWord(address + 2);
    is = m68k::util::format("#$%04x", static_cast<uint16_t>(imm));
    imm_bytes = 2;

    DisassembledOperand src(is, imm_bytes, imm);
    DisassembledOperand dst = cpu.disassembleDstEA(address + 2 + imm_bytes, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
    return DisassembledInstruction(address, opcode, std::string("ori") + ext(sz), src, dst);
}

} // namespace m68k::cpu::instructions
