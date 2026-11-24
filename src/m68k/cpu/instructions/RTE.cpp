#include "m68k/cpu/instructions/RTE.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

RTE::RTE(Cpu& cpu) : cpu(cpu) {}

void RTE::registerInstructions(InstructionSet& is) {
    is.addInstruction(0x4e73,
                      std::make_shared<LambdaInstruction>(
                          [this](int32_t opcode) {
                              if (cpu.isSupervisorMode()) {
                                  int32_t newsr = cpu.popWord();  // SR before the exception
                                  cpu.setPC(cpu.popLong());       // pop this BEFORE setting the SR which may change A7
                                  cpu.setSR2(newsr);

                                  return 20;
                              } else {
                                  // privilege violation
                                  cpu.raiseException(8);
                                  return 34;
                              }
                          },
                          [this](int32_t address, int32_t opcode) {
                              return DisassembledInstruction(address, opcode, "rte");
                          }));
}

} // namespace m68k::cpu::instructions
