#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "m68k/cpu/DisassembledInstruction.h"
#include "m68k/cpu/InstructionHandler.h"
#include "m68k/cpu/assemble/Binary.h"
#include "m68k/cpu/assemble/Labels.h"
#include "m68k/memory/AddressSpace.h"

namespace m68k::cpu::assemble {

/*
 * C++ port of m68k.cpu.assemble.Assembler.
 *
 * http://goldencrystal.free.fr/M68kOpcodes.pdf
 * http://www.raidenii.net/files/datasheets/cpu/68000_opcodes.pdf
 * http://oldwww.nvg.ntnu.no/amiga/MC680x0_Sections/index.HTML
 *
 * Two Java constructs have no direct C++ counterpart:
 *
 *  - every handler in commandMapping was created with a null Cpu, e.g.
 *    "new EXG(null)".  The map is only used to recognise a mnemonic (assemble()
 *    was still a TODO in the original and never runs a handler), so the handlers
 *    are bound to a null Cpu reference - see nullCpu() in Assembler.cpp.
 *  - parseLine() returns null while assembling is unfinished.  A
 *    DisassembledInstruction is returned by value here, so that result becomes
 *    std::nullopt; every other branch still returns a DisassembledInstruction.
 *
 * The Java debug "public static void main(String[] args)" self-test (it only
 * disassembled test.asm from the command line) is not part of this port.
 */
class Assembler {
public:
    explicit Assembler(m68k::memory::AddressSpace* mmu);

    std::set<Binary> assemble(const std::vector<std::string>& source);

    int32_t getPc();
    void setPc(int32_t pc);

    std::optional<DisassembledInstruction> parseLine(const std::string& line);

    std::optional<DisassembledInstruction> parseLine(int32_t lineNumber, const std::string& line);

    void printSymbols();

private:
    static std::unordered_map<std::string, std::shared_ptr<InstructionHandler>> commandMapping;

    int32_t pc = 0;
    m68k::memory::AddressSpace* mmu = nullptr;
    Labels labels;

    std::vector<std::string> splitLine(const std::string& line);
};

} // namespace m68k::cpu::assemble
