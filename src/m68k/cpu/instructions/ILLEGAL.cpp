#include "m68k/cpu/instructions/ILLEGAL.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

ILLEGAL::ILLEGAL(Cpu& cpu) : cpu(cpu) {}

void ILLEGAL::registerInstructions(InstructionSet& is) {
    is.addInstruction(0x4afc,
                      std::make_shared<LambdaInstruction>(
                          [this](int32_t opcode) {
                              // vector 4
                              cpu.raiseException(4);
                              return 34;
                          },
                          [this](int32_t address, int32_t opcode) {
                              return DisassembledInstruction(address, opcode, "illegal");
                          }));
}

} // namespace m68k::cpu::instructions
