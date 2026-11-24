#include "m68k/cpu/assemble/Mnemonics.h"

#include <utility>

#include "m68k/cpu/Cpu.h"
#include "m68k/cpu/instructions/EXG.h"

namespace m68k::cpu::assemble {

int32_t Mnemonics::DATA_REGISTER = 1;
int32_t Mnemonics::ADDRESS_REGISTER = 2;

Mnemonics::Mnemonic::Mnemonic(std::shared_ptr<InstructionHandler> handler, int32_t src, int32_t dst)
    : handler(std::move(handler)), src(src), dst(dst) {}

namespace {

/*
 * Java created the handler with a null Cpu ("new EXG(null)") because inside
 * Mnemonics the handler is only a mnemonic descriptor: it is never registered
 * on an InstructionSet and never executed.  A C++ reference cannot be null, so
 * the Java null is spelled as a dereferenced null pointer instead; the handler
 * stores the reference and never reads it.
 */
Cpu& nullCpu() {
    return *static_cast<Cpu*>(nullptr);
}

// Java's "static { mapping.put(...); }" initialiser block of
// m68k.cpu.assemble.Mnemonics.  The Java map holds this one entry.
std::unordered_map<std::string, Mnemonics::Mnemonic> createMapping() {
    std::unordered_map<std::string, Mnemonics::Mnemonic> mapping;

    mapping.emplace("exg", Mnemonics::Mnemonic(std::make_shared<m68k::cpu::instructions::EXG>(nullCpu()),
                                              Mnemonics::DATA_REGISTER | Mnemonics::ADDRESS_REGISTER,
                                              Mnemonics::DATA_REGISTER | Mnemonics::ADDRESS_REGISTER));

    return mapping;
}

} // namespace

std::unordered_map<std::string, Mnemonics::Mnemonic> Mnemonics::mapping = createMapping();

} // namespace m68k::cpu::assemble
