/*
 * Functional smoke test for the C++ port.
 *
 * This is not part of the shipped program: it drives the ported CPU, memory and
 * assembler directly to prove that the conversion behaves like the Java original
 * (opcode dispatch, effective-address resolution, flag calculation, big-endian
 * memory, disassembly and the assembler front end).
 *
 * Build (the source glob excludes src/m68k/main.cpp, which defines another main):
 */

#include <cstdint>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "m68k/cpu/MC68000.h"
#include "m68k/cpu/assemble/Assembler.h"
#include "m68k/cpu/assemble/ParseException.h"
#include "m68k/memory/MemorySpace.h"

namespace {

int checks = 0;
int failures = 0;

void check(bool condition, const std::string& what) {
    checks++;
    if (!condition) {
        failures++;
        std::cout << "FAIL  " << what << "\n";
    } else {
        std::cout << "ok    " << what << "\n";
    }
}

void checkEq(int32_t actual, int32_t expected, const std::string& what) {
    checks++;
    if (actual != expected) {
        failures++;
        std::cout << "FAIL  " << what << "  expected $";
        std::cout << std::hex << static_cast<uint32_t>(expected) << " got $"
                  << static_cast<uint32_t>(actual) << std::dec << "\n";
    } else {
        std::cout << "ok    " << what << "\n";
    }
}

} // namespace

int main() {
    m68k::cpu::MC68000 cpu;
    m68k::memory::MemorySpace mem(1024); // 1 MB
    cpu.setAddressSpace(&mem);

    // ---------------------------------------------------------------------
    // reset() must load the initial SSP and PC from the vector table
    // ---------------------------------------------------------------------
    mem.writeLong(0, 0x0007fff0);
    mem.writeLong(4, 0x00001000);
    cpu.reset();
    checkEq(cpu.getSSP(), 0x0007fff0, "reset reads SSP from vector 0");
    checkEq(cpu.getPC(), 0x00001000, "reset reads PC from vector 4");
    checkEq(cpu.getSR(), 0x2700, "reset sets SR = 0x2700 (supervisor, ints masked)");
    check(cpu.isSupervisorMode(), "reset leaves the CPU in supervisor mode");

    // ---------------------------------------------------------------------
    // memory is big-endian, like java.nio.ByteBuffer
    // ---------------------------------------------------------------------
    mem.writeLong(0x2000, 0x12345678);
    checkEq(mem.readByte(0x2000), 0x12, "writeLong/readByte is big-endian");
    checkEq(mem.readWord(0x2000), 0x1234, "writeLong/readWord is big-endian");
    checkEq(mem.readLong(0x2000), 0x12345678, "writeLong/readLong round-trips");
    mem.writeWord(0x2010, 0xabcd);
    checkEq(mem.readByte(0x2010), 0xab, "writeWord stores the high byte first");

    // ---------------------------------------------------------------------
    // execution: moveq #5,d0 / moveq #7,d1 / add.l d1,d0 / swap d0 / nop
    // ---------------------------------------------------------------------
    mem.writeWord(0x1000, 0x7005); // moveq #5,d0
    mem.writeWord(0x1002, 0x7207); // moveq #7,d1
    mem.writeWord(0x1004, 0xd081); // add.l  d1,d0
    mem.writeWord(0x1006, 0x4840); // swap   d0
    mem.writeWord(0x1008, 0x4e71); // nop

    cpu.setPC(0x1000);

    cpu.execute();
    checkEq(cpu.getDataRegisterLong(0), 5, "moveq #5,d0");
    checkEq(cpu.getPC(), 0x1002, "pc advanced past moveq");

    cpu.execute();
    checkEq(cpu.getDataRegisterLong(1), 7, "moveq #7,d1");

    cpu.execute();
    checkEq(cpu.getDataRegisterLong(0), 12, "add.l d1,d0");
    checkEq(cpu.getSR() & m68k::cpu::Z_FLAG, 0, "add clears Z");
    checkEq(cpu.getSR() & m68k::cpu::N_FLAG, 0, "add clears N");
    checkEq(cpu.getSR() & m68k::cpu::C_FLAG, 0, "add clears C");
    checkEq(cpu.getSR() & m68k::cpu::V_FLAG, 0, "add clears V");

    cpu.execute();
    checkEq(cpu.getDataRegisterLong(0), 0x000c0000, "swap d0");

    cpu.setPC(0x1008);
    const int32_t nopTime = cpu.execute();
    checkEq(nopTime, 4, "nop reports 4 cycles");

    // ---------------------------------------------------------------------
    // signed overflow: 0x7fffffff + 1 must set V and N, and leave C clear
    // ---------------------------------------------------------------------
    mem.writeWord(0x1010, 0xd081); // add.l d1,d0
    cpu.setDataRegisterLong(0, 0x7fffffff);
    cpu.setDataRegisterLong(1, 1);
    cpu.setPC(0x1010);
    cpu.execute();
    checkEq(cpu.getDataRegisterLong(0), static_cast<int32_t>(0x80000000u), "add.l overflow wraps");
    check((cpu.getSR() & m68k::cpu::V_FLAG) != 0, "add.l sets V on signed overflow");
    check((cpu.getSR() & m68k::cpu::N_FLAG) != 0, "add.l sets N on negative result");
    check((cpu.getSR() & m68k::cpu::C_FLAG) == 0, "add.l leaves C clear on no carry");

    // ---------------------------------------------------------------------
    // effective address: move.b (a0)+,d0  reads then post-increments a0
    // ---------------------------------------------------------------------
    mem.writeWord(0x1020, 0x1018); // move.b (a0)+,d0
    mem.writeByte(0x3000, 0xab);
    cpu.setAddrRegisterLong(0, 0x3000);
    cpu.setDataRegisterLong(0, 0xffff0000);
    cpu.setPC(0x1020);
    cpu.execute();
    checkEq(cpu.getDataRegisterByte(0), 0xab, "move.b (a0)+,d0 reads the byte");
    checkEq(cpu.getAddrRegisterLong(0), 0x3001, "move.b (a0)+,d0 post-increments a0");
    checkEq(cpu.getDataRegisterLong(0), 0xffff00ab, "move.b only replaces the low byte");

    // ---------------------------------------------------------------------
    // effective address: move.w d1,(a2)  write path
    // ---------------------------------------------------------------------
    mem.writeWord(0x1030, 0x3481); // move.w d1,(a2)
    cpu.setDataRegisterLong(1, 0x11223344);
    cpu.setAddrRegisterLong(2, 0x3010);
    cpu.setPC(0x1030);
    cpu.execute();
    checkEq(mem.readWord(0x3010), 0x3344, "move.w d1,(a2) writes the low word");

    // ---------------------------------------------------------------------
    // stacks: pushLong/popLong are pre-decrement/post-increment on a7
    // ---------------------------------------------------------------------
    cpu.setSSP(0x4000);
    cpu.pushLong(0xdeadbeef);
    checkEq(cpu.getAddrRegisterLong(7), 0x3ffc, "pushLong lowers the stack pointer by 4");
    checkEq(mem.readLong(0x3ffc), static_cast<int32_t>(0xdeadbeefu), "pushLong stores the value");
    checkEq(cpu.popLong(), static_cast<int32_t>(0xdeadbeefu), "popLong returns the value");
    checkEq(cpu.getAddrRegisterLong(7), 0x4000, "popLong restores the stack pointer");

    // ---------------------------------------------------------------------
    // condition codes
    // ---------------------------------------------------------------------
    cpu.setCCRegister(m68k::cpu::Z_FLAG);
    check(cpu.testCC(7), "testCC: EQ is true when Z is set");
    check(!cpu.testCC(6), "testCC: NE is false when Z is set");
    cpu.setCCRegister(0x00);
    check(!cpu.testCC(7), "testCC: EQ is false when Z is clear");
    check(cpu.testCC(6), "testCC: NE is true when Z is clear");
    cpu.setCCRegister(m68k::cpu::N_FLAG | m68k::cpu::V_FLAG);
    check(cpu.testCC(12), "testCC: GE true when N and V both set");
    check(!cpu.testCC(13), "testCC: LT false when N and V both set");

    // ---------------------------------------------------------------------
    // disassembly
    // ---------------------------------------------------------------------
    m68k::cpu::Instruction* instr = cpu.getInstructionAt(0x1000);
    const std::string text = instr->disassemble(0x1000, 0x7005).toString();
    check(text.find("moveq") != std::string::npos, "disassembles moveq");
    check(text.find("#$05") != std::string::npos, "disassembly renders the immediate operand");
    check(text.find("d0") != std::string::npos, "disassembly renders the destination register");
    check(text.find("00001000") != std::string::npos, "disassembly renders the address");

    // every opcode must resolve to something (registered or the UNKNOWN fallback)
    bool allResolve = true;
    for (int32_t op = 0; op < 0x10000; op++) {
        if (cpu.getInstructionFor(op) == nullptr) {
            allResolve = false;
            break;
        }
    }
    check(allResolve, "all 65536 opcodes resolve to an Instruction");

    // ---------------------------------------------------------------------
    // assembler front end (exercises the 13 ported assemble/* classes)
    //
    // Note the Java Assembler never finished its assemble() step: for a real
    // instruction parseLine() returns null (here std::nullopt), while comment
    // and label lines return a DisassembledInstruction and an unknown mnemonic
    // throws ParseException. The assertions below pin down exactly that.
    // ---------------------------------------------------------------------
    try {
        m68k::cpu::assemble::Assembler assembler(&mem);

        const auto label = assembler.parseLine(1, "start:");
        check(label.has_value(), "assembler parseLine accepts a label definition");

        const auto comment = assembler.parseLine(2, " ; just a comment");
        check(comment.has_value(), "assembler parseLine accepts a comment");

        const auto instruction = assembler.parseLine(3, " moveq #5,d0");
        check(!instruction.has_value(),
              "assembler parseLine returns nullopt for an instruction (Java returns null)");

        bool threw = false;
        try {
            assembler.parseLine(4, " not_an_instruction");
        } catch (const m68k::cpu::assemble::ParseException&) {
            threw = true;
        }
        check(threw, "assembler parseLine throws ParseException for an unknown mnemonic");
    } catch (const std::exception& e) {
        check(false, std::string("assembler front end threw unexpectedly: ") + e.what());
    }

    std::cout << "\n" << (checks - failures) << "/" << checks << " checks passed\n";
    return failures == 0 ? 0 : 1;
}
