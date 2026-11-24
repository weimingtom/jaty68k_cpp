#pragma once

#include <cstdint>

#include "m68k/cpu/DisassembledInstruction.h"
#include "m68k/cpu/DisassembledOperand.h"
#include "m68k/cpu/Instruction.h"
#include "m68k/cpu/InstructionType.h"
#include "m68k/cpu/Operand.h"
#include "m68k/cpu/Size.h"
#include "m68k/memory/AddressSpace.h"

namespace m68k::cpu {

/*
 * Status register / condition code bits, C++ port of the constants that the
 * Java code declared inside the Cpu interface.  They stay at namespace scope,
 * so Java's "cpu.C_FLAG" becomes simply "C_FLAG".
 */
inline constexpr int32_t C_FLAG = 1;
inline constexpr int32_t V_FLAG = 2;
inline constexpr int32_t Z_FLAG = 4;
inline constexpr int32_t N_FLAG = 8;
inline constexpr int32_t X_FLAG = 16;
inline constexpr int32_t INTERRUPT_FLAGS_MASK = 0x0700;
inline constexpr int32_t SUPERVISOR_FLAG = 0x2000;
inline constexpr int32_t TRACE_FLAG = static_cast<int32_t>(0x8000u);

/*
 * C++ port of m68k.cpu.Cpu (Java interface).
 */
class Cpu {
public:
    virtual ~Cpu() = default;

    virtual void setAddressSpace(m68k::memory::AddressSpace* memory) = 0;
    virtual void reset() = 0;
    virtual void stop() = 0;
    virtual int32_t execute() = 0;

    // data registers
    virtual int32_t getDataRegisterByte(int32_t reg) = 0;
    virtual int32_t getDataRegisterByteSigned(int32_t reg) = 0;
    virtual int32_t getDataRegisterWord(int32_t reg) = 0;
    virtual int32_t getDataRegisterWordSigned(int32_t reg) = 0;
    virtual int32_t getDataRegisterLong(int32_t reg) = 0;
    virtual void setDataRegisterByte(int32_t reg, int32_t value) = 0;
    virtual void setDataRegisterWord(int32_t reg, int32_t value) = 0;
    virtual void setDataRegisterLong(int32_t reg, int32_t value) = 0;

    // address registers
    virtual int32_t getAddrRegisterByte(int32_t reg) = 0;
    virtual int32_t getAddrRegisterByteSigned(int32_t reg) = 0;
    virtual int32_t getAddrRegisterWord(int32_t reg) = 0;
    virtual int32_t getAddrRegisterWordSigned(int32_t reg) = 0;
    virtual int32_t getAddrRegisterLong(int32_t reg) = 0;
    virtual void setAddrRegisterByte(int32_t reg, int32_t value) = 0;
    virtual void setAddrRegisterWord(int32_t reg, int32_t value) = 0;
    virtual void setAddrRegisterLong(int32_t reg, int32_t value) = 0;

    // memory interface
    virtual int32_t readMemoryByte(int32_t addr) = 0;
    virtual int32_t readMemoryByteSigned(int32_t addr) = 0;
    virtual int32_t readMemoryWord(int32_t addr) = 0;
    virtual int32_t readMemoryWordSigned(int32_t addr) = 0;
    virtual int32_t readMemoryLong(int32_t addr) = 0;
    virtual void writeMemoryByte(int32_t addr, int32_t value) = 0;
    virtual void writeMemoryWord(int32_t addr, int32_t value) = 0;
    virtual void writeMemoryLong(int32_t addr, int32_t value) = 0;

    // addr reg helpers
    virtual void incrementAddrRegister(int32_t reg, int32_t numBytes) = 0;
    virtual void decrementAddrRegister(int32_t reg, int32_t numBytes) = 0;

    // PC reg
    virtual int32_t getPC() = 0;
    virtual void setPC(int32_t address) = 0;

    // pc fetches - for reading data following instructions and incrementing the PC afterwards
    virtual int32_t fetchPCWord() = 0;
    virtual int32_t fetchPCWordSigned() = 0;
    virtual int32_t fetchPCLong() = 0;

    // status reg
    virtual bool isSupervisorMode() = 0;
    virtual int32_t getCCRegister() = 0;
    virtual int32_t getSR() = 0;
    virtual void setCCRegister(int32_t value) = 0;
    virtual void setSR(int32_t value) = 0;
    virtual void setSR2(int32_t value) = 0;

    // flags
    virtual void setFlags(int32_t flags) = 0;
    virtual void clrFlags(int32_t flags) = 0;
    virtual bool isFlagSet(int32_t flag) = 0;
    virtual void calcFlags(InstructionType type, int32_t s, int32_t d, int32_t r, Size sz) = 0;
    virtual void calcFlagsParam(InstructionType type, int32_t s, int32_t d, int32_t r, int32_t extraParam,
                                Size sz) = 0;
    virtual bool testCC(int32_t cc) = 0;

    // stacks
    virtual int32_t getUSP() = 0;
    virtual void setUSP(int32_t address) = 0;
    virtual int32_t getSSP() = 0;
    virtual void setSSP(int32_t address) = 0;
    virtual void pushWord(int32_t value) = 0;
    virtual void pushLong(int32_t value) = 0;
    virtual int32_t popWord() = 0;
    virtual int32_t popLong() = 0;

    // exceptions & interrupts
    virtual void raiseException(int32_t vector) = 0;
    virtual void raiseSRException() = 0;
    virtual void raiseInterrupt(int32_t priority) = 0;
    virtual void raiseSafeInterrupt(int32_t priority) = 0;
    virtual void executeSafeInterrupt() = 0;
    virtual int32_t getInterruptLevel() = 0;

    // source EA
    virtual Operand* resolveSrcEA(int32_t mode, int32_t reg, Size sz) = 0;
    // destination EA
    virtual Operand* resolveDstEA(int32_t mode, int32_t reg, Size sz) = 0;

    // disassembling
    virtual Instruction* getInstructionAt(int32_t address) = 0;
    virtual Instruction* getInstructionFor(int32_t opcode) = 0;
    virtual DisassembledOperand disassembleSrcEA(int32_t address, int32_t mode, int32_t reg, Size sz) = 0;
    virtual DisassembledOperand disassembleDstEA(int32_t address, int32_t mode, int32_t reg, Size sz) = 0;
};

} // namespace m68k::cpu
