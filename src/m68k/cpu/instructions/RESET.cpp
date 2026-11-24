#include "m68k/cpu/instructions/RESET.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

RESET::RESET(Cpu& cpu) : cpu(cpu) {}

void RESET::registerInstructions(InstructionSet& is) {
    is.addInstruction(0x4e70,
                      std::make_shared<LambdaInstruction>(
                          [this](int32_t opcode) {
                              if (cpu.isSupervisorMode()) {
                                  cpu.reset();
                                  return 132;
                              } else {
                                  // privilege violation
                                  cpu.raiseException(8);
                                  return 34;
                              }
                          },
                          [this](int32_t address, int32_t opcode) {
                              return DisassembledInstruction(address, opcode, "reset");
                          }));
}

} // namespace m68k::cpu::instructions
