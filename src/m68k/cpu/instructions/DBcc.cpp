#include "m68k/cpu/instructions/DBcc.h"

#include "m68k/cpu/LambdaInstruction.h"
#include "m68k/util/Format.h"

namespace m68k::cpu::instructions {

const std::array<std::string, 16> DBcc::names = {
    "dbt", "dbra", "dbhi", "dbls", "dbcc", "dbcs", "dbne", "dbeq",
    "dbvc", "dbvs", "dbpl", "dbmi", "dbge", "dblt", "dbgt", "dble"};

DBcc::DBcc(Cpu& cpu) : cpu(cpu) {}

void DBcc::registerInstructions(InstructionSet& is) {
    int32_t base = 0x50c8;
    std::shared_ptr<Instruction> i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return dbxx(opcode); },
        [this](int32_t address, int32_t opcode) { return disassembleOp(address, opcode); });

    for (int32_t cc = 0; cc < 16; cc++) {
        for (int32_t r = 0; r < 8; r++) {
            is.addInstruction(base + (cc << 8) + r, i);
        }
    }
}

int32_t DBcc::dbxx(int32_t opcode) {
    int32_t reg = (opcode & 0x07);
    int32_t pc = cpu.getPC();
    int32_t dis = cpu.fetchPCWordSigned();
    int32_t time;
    int32_t count = cpu.getDataRegisterWordSigned(reg) - 1;

    if (cpu.testCC((opcode >> 8) & 0x0f)) {
        // condition met
        time = 12;
    } else {
        // only decrease the reg is the condition is not met!!!!!
        cpu.setDataRegisterWord(reg, count);
        if (count == -1) {
            // counter expired
            time = 14;
        } else {
            // loop
            cpu.setPC(pc + dis);
            time = 10;
        }
    }
    return time;
}

DisassembledInstruction DBcc::disassembleOp(int32_t address, int32_t opcode) {
    int32_t cc = (opcode >> 8) & 0x0f;
    int32_t dis = cpu.readMemoryWordSigned(address + 2);

    DisassembledOperand reg(m68k::util::format("d%d", (opcode & 0x07)));
    //word displacement
    DisassembledOperand where(m68k::util::format("$%08x", dis + address + 2), 2, dis);

    return DisassembledInstruction(address, opcode, names[cc], reg, where);
}

} // namespace m68k::cpu::instructions
