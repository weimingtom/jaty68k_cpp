#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>

#include "m68k/cpu/Cpu.h"

namespace m68k::cpu {

/*
 * C++ port of m68k.cpu.CpuCore (Java abstract class).
 *
 * The Java version declared its effective-address operands as inner classes in
 * the same compilation unit.  Here they live in CpuCore.cpp as ordinary classes
 * (see initEAHandlers()) - they only ever use CpuCore's public interface, which
 * keeps this header readable.
 *
 * Java int fields are int32_t, and the build enables -fwrapv so that arithmetic
 * wraps exactly like Java's defined two's-complement overflow.
 */
class CpuCore : public Cpu {
public:
    CpuCore();
    ~CpuCore() override;

    void setAddressSpace(m68k::memory::AddressSpace* as) override;
    void reset() override;
    void stop() override;

    void raiseSafeInterrupt(int32_t priority) override;
    void executeSafeInterrupt() override;

    // data registers
    int32_t getDataRegisterByte(int32_t reg) override;
    int32_t getDataRegisterByteSigned(int32_t reg) override;
    int32_t getDataRegisterWord(int32_t reg) override;
    int32_t getDataRegisterWordSigned(int32_t reg) override;
    int32_t getDataRegisterLong(int32_t reg) override;
    void setDataRegisterByte(int32_t reg, int32_t value) override;
    void setDataRegisterWord(int32_t reg, int32_t value) override;
    void setDataRegisterLong(int32_t reg, int32_t value) override;

    // address registers
    int32_t getAddrRegisterByte(int32_t reg) override;
    int32_t getAddrRegisterByteSigned(int32_t reg) override;
    int32_t getAddrRegisterWord(int32_t reg) override;
    int32_t getAddrRegisterWordSigned(int32_t reg) override;
    int32_t getAddrRegisterLong(int32_t reg) override;
    void setAddrRegisterByte(int32_t reg, int32_t value) override;
    void setAddrRegisterWord(int32_t reg, int32_t value) override;
    void setAddrRegisterLong(int32_t reg, int32_t value) override;

    int32_t getPC() override;
    void setPC(int32_t address) override;

    // pc fetches - for reading data following instructions and incrementing the PC afterwards
    int32_t fetchPCWord() override;
    int32_t fetchPCWordSigned() override;
    int32_t fetchPCLong() override;

    // status reg
    int32_t getCCRegister() override;
    int32_t getSR() override;
    void setCCRegister(int32_t value) override;
    void setSR(int32_t value) override;
    void setSR2(int32_t value) override;

    void setFlags(int32_t flags) override;
    void clrFlags(int32_t flags) override;
    bool isFlagSet(int32_t flag) override;
    void calcFlags(InstructionType type, int32_t src, int32_t dst, int32_t result, Size sz) override;
    void calcFlagsParam(InstructionType type, int32_t src, int32_t dst, int32_t result, int32_t extraParam,
                        Size sz) override;
    bool testCC(int32_t cc) override;

    bool isSupervisorMode() override;

    void pushWord(int32_t value) override;
    void pushLong(int32_t value) override;
    int32_t popWord() override;
    int32_t popLong() override;

    int32_t getUSP() override;
    void setUSP(int32_t address) override;
    int32_t getSSP() override;
    void setSSP(int32_t address) override;

    void setSupervisorMode(bool enable);

    void raiseException(int32_t vector) override;
    void raiseSRException() override;

    int32_t getInterruptLevel() override;
    void raiseInterrupt(int32_t priority) override;

    // memory interface
    int32_t readMemoryByte(int32_t addr) override;
    int32_t readMemoryByteSigned(int32_t addr) override;
    int32_t readMemoryWord(int32_t addr) override;
    int32_t readMemoryWordSigned(int32_t addr) override;
    int32_t readMemoryLong(int32_t addr) override;
    void writeMemoryByte(int32_t addr, int32_t value) override;
    void writeMemoryWord(int32_t addr, int32_t value) override;
    void writeMemoryLong(int32_t addr, int32_t value) override;

    void incrementAddrRegister(int32_t reg, int32_t numBytes) override;
    void decrementAddrRegister(int32_t reg, int32_t numBytes) override;

    Operand* resolveSrcEA(int32_t mode, int32_t reg, Size size) override;
    Operand* resolveDstEA(int32_t mode, int32_t reg, Size size) override;

    DisassembledOperand disassembleSrcEA(int32_t address, int32_t mode, int32_t reg, Size sz) override;
    DisassembledOperand disassembleDstEA(int32_t address, int32_t mode, int32_t reg, Size sz) override;

protected:
    m68k::memory::AddressSpace* memory = nullptr;
    std::array<int32_t, 8> data_regs{};
    std::array<int32_t, 8> addr_regs{};
    int32_t reg_pc = 0;
    int32_t reg_sr = 0;
    int32_t reg_usp = 0;
    int32_t reg_ssp = 0;

    // owning storage for the operand handlers, plus the raw view used at run time
    std::array<std::unique_ptr<Operand>, 12> srcHandlerStore_{};
    std::array<std::unique_ptr<Operand>, 12> dstHandlerStore_{};
    std::array<Operand*, 12> srcHandlers{};
    std::array<Operand*, 12> dstHandlers{};

    Operand* srcEAHandler = nullptr;
    Operand* dstEAHandler = nullptr;

    int32_t currentInstructionAddress = 0;
    std::string disasmBuffer;

    int32_t safeInterrupt = 0;
    int32_t safeInterruptTimer = 0;

    bool debug = false;

    void setInterruptLevel(int32_t level);

    DisassembledOperand disassembleEA(int32_t address, int32_t mode, int32_t reg, Size sz, bool is_src);

    int32_t signExtendByteValue(int32_t value) const;
    int32_t signExtendWordValue(int32_t value) const;

    void initEAHandlers();
};

} // namespace m68k::cpu
