#pragma once

#include <stdexcept>

#include "m68k/cpu/Cpu.h"

namespace m68k::cpu {

/*
 * A Cpu that refuses every call, used where the Java code passed a null Cpu.
 *
 * java/src/m68k/cpu/assemble/Assembler.java builds its mnemonic table with
 * "new EXG(null)" and friends: the handlers only ever need to be constructed,
 * never run. Binding a C++ reference to a null pointer would be undefined
 * behaviour, so those handlers are bound to this object instead - the same
 * "there is no Cpu here" state, but well defined: touching it throws.
 */
class NullCpu : public Cpu {
public:
    void setAddressSpace(m68k::memory::AddressSpace* memory) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void reset() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void stop() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t execute() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t getDataRegisterByte(int32_t reg) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t getDataRegisterByteSigned(int32_t reg) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t getDataRegisterWord(int32_t reg) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t getDataRegisterWordSigned(int32_t reg) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t getDataRegisterLong(int32_t reg) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void setDataRegisterByte(int32_t reg, int32_t value) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void setDataRegisterWord(int32_t reg, int32_t value) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void setDataRegisterLong(int32_t reg, int32_t value) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t getAddrRegisterByte(int32_t reg) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t getAddrRegisterByteSigned(int32_t reg) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t getAddrRegisterWord(int32_t reg) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t getAddrRegisterWordSigned(int32_t reg) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t getAddrRegisterLong(int32_t reg) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void setAddrRegisterByte(int32_t reg, int32_t value) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void setAddrRegisterWord(int32_t reg, int32_t value) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void setAddrRegisterLong(int32_t reg, int32_t value) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t readMemoryByte(int32_t addr) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t readMemoryByteSigned(int32_t addr) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t readMemoryWord(int32_t addr) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t readMemoryWordSigned(int32_t addr) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t readMemoryLong(int32_t addr) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void writeMemoryByte(int32_t addr, int32_t value) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void writeMemoryWord(int32_t addr, int32_t value) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void writeMemoryLong(int32_t addr, int32_t value) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void incrementAddrRegister(int32_t reg, int32_t numBytes) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void decrementAddrRegister(int32_t reg, int32_t numBytes) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t getPC() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void setPC(int32_t address) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t fetchPCWord() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t fetchPCWordSigned() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t fetchPCLong() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    bool isSupervisorMode() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t getCCRegister() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t getSR() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void setCCRegister(int32_t value) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void setSR(int32_t value) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void setSR2(int32_t value) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void setFlags(int32_t flags) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void clrFlags(int32_t flags) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    bool isFlagSet(int32_t flag) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void calcFlags(InstructionType type, int32_t s, int32_t d, int32_t r, Size sz) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }    void calcFlagsParam(InstructionType type, int32_t s, int32_t d, int32_t r, int32_t extraParam, Size sz) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    bool testCC(int32_t cc) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t getUSP() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void setUSP(int32_t address) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t getSSP() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void setSSP(int32_t address) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void pushWord(int32_t value) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void pushLong(int32_t value) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t popWord() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t popLong() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void raiseException(int32_t vector) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void raiseSRException() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void raiseInterrupt(int32_t priority) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void raiseSafeInterrupt(int32_t priority) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    void executeSafeInterrupt() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    int32_t getInterruptLevel() override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    Operand* resolveSrcEA(int32_t mode, int32_t reg, Size sz) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    Operand* resolveDstEA(int32_t mode, int32_t reg, Size sz) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    Instruction* getInstructionAt(int32_t address) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    Instruction* getInstructionFor(int32_t opcode) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    DisassembledOperand disassembleSrcEA(int32_t address, int32_t mode, int32_t reg, Size sz) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
    DisassembledOperand disassembleDstEA(int32_t address, int32_t mode, int32_t reg, Size sz) override { throw std::logic_error("NullCpu: no Cpu is attached to this assembler handler"); }
};

} // namespace m68k::cpu
