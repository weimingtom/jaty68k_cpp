#include "m68k/cpu/instructions/EORI_TO_SR.h"

#include "m68k/cpu/LambdaInstruction.h"
#include "m68k/util/Format.h"

namespace m68k::cpu::instructions {

EORI_TO_SR::EORI_TO_SR(Cpu& cpu) : cpu(cpu) {}

void EORI_TO_SR::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    base = 0x0a7c;
    i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) {
            int32_t s = cpu.fetchPCWordSigned();
            if (cpu.isSupervisorMode()) {
                s &= 0xf71f; // mask out bits 5,6,7 & 11 : they are always 0!
                cpu.setSR(cpu.getSR() ^ s);
            } else {
                cpu.raiseSRException();
                return 34;
            }

            return 8; // i'm NOT sure about this timing
        },
        [this](int32_t address, int32_t opcode) {
            return disassembleOp(address, opcode, Size::Word);
        });
    is.addInstruction(base, i);
}

DisassembledInstruction EORI_TO_SR::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    int32_t imm_bytes;
    int32_t imm;
    std::string is;

    imm = cpu.readMemoryWord(address + 2);
    is = m68k::util::format("#$%04x", static_cast<uint16_t>(imm));
    imm_bytes = 2;
    DisassembledOperand src(is, imm_bytes, imm);
    DisassembledOperand dst = cpu.disassembleDstEA(address + 2 + imm_bytes, (opcode >> 3) & 0x07, (opcode & 0x07), sz);

    return DisassembledInstruction(address, opcode, std::string("EORI") + ext(sz), src, dst);
}

} // namespace m68k::cpu::instructions
