#include "m68k/cpu/instructions/ORI_TO_SR.h"

#include "m68k/cpu/LambdaInstruction.h"
#include "m68k/util/Format.h"

namespace m68k::cpu::instructions {

ORI_TO_SR::ORI_TO_SR(Cpu& cpu) : cpu(cpu) {}

void ORI_TO_SR::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    base = 0x007c;
    i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return ori_word(opcode); },
        [this](int32_t address, int32_t opcode) {
            return disassembleOp(address, opcode, Size::Word);
        });
    is.addInstruction(base, i);
}

int32_t ORI_TO_SR::ori_word(int32_t opcode) {
    // mask out bits 5,6,7 & 11 : they are always 0!;
    int32_t s = cpu.fetchPCWordSigned() & 0xf71f;
    if (cpu.isSupervisorMode()) {
        cpu.setSR(cpu.getSR() | s);
    } else {
        cpu.raiseSRException();
        return 34;
    }
    return 8;                                   // i'm not sure this is true
}

DisassembledInstruction ORI_TO_SR::disassembleOp(int32_t address, int32_t opcode, Size sz) {
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

DisassembledInstruction ORI_TO_SR::bdisassembleOp(int32_t address, int32_t opcode, Size sz) {
    std::string is = m68k::util::format("#$%04x", static_cast<uint16_t>(cpu.readMemoryWord(address + 2)));

    return DisassembledInstruction(address, opcode, std::string("ori.w  ") + is + ",SR");
}

} // namespace m68k::cpu::instructions
