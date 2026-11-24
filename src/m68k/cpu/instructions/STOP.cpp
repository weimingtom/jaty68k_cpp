#include "m68k/cpu/instructions/STOP.h"

#include "m68k/cpu/LambdaInstruction.h"
#include "m68k/util/Format.h"

namespace m68k::cpu::instructions {

STOP::STOP(Cpu& cpu) : cpu(cpu) {}

void STOP::registerInstructions(InstructionSet& is) {
    is.addInstruction(0x4e72,
                      std::make_shared<LambdaInstruction>(
                          [this](int32_t opcode) {
                              const int32_t data = cpu.fetchPCWord();

                              if (!cpu.isSupervisorMode() || (data & SUPERVISOR_FLAG) == 0) {
                                  // privilege violation
                                  cpu.raiseException(8);
                                  return 34;
                              } else {
                                  cpu.setSR(data);
                                  cpu.stop();
                                  return 4;
                              }
                          },
                          [this](int32_t address, int32_t opcode) {
                              const int32_t data = cpu.readMemoryWord(address + 2);
                              DisassembledOperand op(
                                  m68k::util::format("#$%04x", static_cast<uint16_t>(data)), 2, data);
                              return DisassembledInstruction(address, opcode, "stop", op);
                          }));
}

} // namespace m68k::cpu::instructions
