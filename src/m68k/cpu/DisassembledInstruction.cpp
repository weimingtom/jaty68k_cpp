#include "m68k/cpu/DisassembledInstruction.h"

#include <cstdint>

#include "m68k/util/Format.h"

namespace m68k::cpu {

DisassembledInstruction::DisassembledInstruction(int32_t address, int32_t opcode,
                                                 const std::string& instruction)
    : address(address),
      opcode(opcode),
      num_operands(0),
      instruction(instruction),
      op1(""),
      op2("") {}

DisassembledInstruction::DisassembledInstruction(int32_t address, int32_t opcode,
                                                 const std::string& instruction,
                                                 const DisassembledOperand& dop)
    : address(address),
      opcode(opcode),
      num_operands(1),
      instruction(instruction),
      op1(dop),
      op2("") {}

DisassembledInstruction::DisassembledInstruction(int32_t address, int32_t opcode,
                                                 const std::string& instruction,
                                                 const DisassembledOperand& dop1,
                                                 const DisassembledOperand& dop2)
    : address(address),
      opcode(opcode),
      num_operands(2),
      instruction(instruction),
      op1(dop1),
      op2(dop2) {}

int32_t DisassembledInstruction::size() const {
    int32_t size = 2;
    if (num_operands == 2) {
        size += op1.bytes + op2.bytes;
    } else if (num_operands == 1) {
        size += op1.bytes;
    }

    return size;
}

void DisassembledInstruction::shortFormat(std::string& buffer) const {
    buffer += m68k::util::format("%08x   ", static_cast<uint32_t>(address));
    switch (num_operands) {
        case 0: {
            buffer += instruction;
            break;
        }
        case 1: {
            int32_t ilen = static_cast<int32_t>(instruction.length());
            buffer += instruction;
            while (ilen < 9) {
                buffer += " ";
                ilen++;
            }
            buffer += op1.operand;
            break;
        }
        case 2: {
            int32_t ilen = static_cast<int32_t>(instruction.length());
            buffer += instruction;
            while (ilen < 9) {
                buffer += " ";
                ilen++;
            }
            buffer += op1.operand;
            buffer += ",";
            buffer += op2.operand;
            break;
        }
        default:
            break;
    }
}

void DisassembledInstruction::formatInstruction(std::string& buffer) const {
    buffer += m68k::util::format("%08x   %04x", static_cast<uint32_t>(address),
                                 static_cast<uint16_t>(opcode));

    switch (num_operands) {
        case 0: {
            // 20 spaces
            buffer += "                    ";
            buffer += instruction;
            break;
        }
        case 1: {
            if (op1.bytes == 2) {
                buffer += m68k::util::format(" %04x               ",
                                             static_cast<uint16_t>(op1.memory_read));
            } else if (op1.bytes == 4) {
                buffer += m68k::util::format(" %08x           ", static_cast<uint32_t>(op1.memory_read));
            } else {
                // 20 spaces
                buffer += "                    ";
            }

            int32_t ilen = static_cast<int32_t>(instruction.length());
            buffer += instruction;
            while (ilen < 9) {
                buffer += " ";
                ilen++;
            }
            buffer += op1.operand;
            break;
        }
        case 2: {
            int32_t len = 0;

            if (op1.bytes == 2) {
                buffer += m68k::util::format(" %04x", static_cast<uint16_t>(op1.memory_read));
                len += 5;
            } else if (op1.bytes == 4) {
                buffer += m68k::util::format(" %08x", static_cast<uint32_t>(op1.memory_read));
                len += 9;
            }

            if (op2.bytes == 2) {
                buffer += m68k::util::format(" %04x", static_cast<uint16_t>(op2.memory_read));
                len += 5;
            } else if (op2.bytes == 4) {
                buffer += m68k::util::format(" %08x", static_cast<uint32_t>(op2.memory_read));
                len += 9;
            }

            while (len < 21) {
                buffer += " ";
                len++;
            }
            int32_t ilen = static_cast<int32_t>(instruction.length());
            buffer += instruction;
            while (ilen < 9) {
                buffer += " ";
                ilen++;
            }

            buffer += op1.operand;
            buffer += ",";
            buffer += op2.operand;
            break;
        }
        default:
            break;
    }
}

std::string DisassembledInstruction::toString() const {
    std::string buffer;
    buffer.reserve(80);
    formatInstruction(buffer);
    return buffer;
}

} // namespace m68k::cpu
