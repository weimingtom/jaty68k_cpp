#include "m68k/cpu/CpuCore.h"

#include <stdexcept>
#include <utility>

#include "m68k/cpu/CpuException.h"
#include "m68k/cpu/CpuUtils.h"
#include "m68k/util/Format.h"

namespace m68k::cpu {

namespace {

const char* instructionTypeName(InstructionType type) {
    switch (type) {
        case InstructionType::ADD: return "ADD";
        case InstructionType::ADDX: return "ADDX";
        case InstructionType::AND: return "AND";
        case InstructionType::ASL: return "ASL";
        case InstructionType::ASR: return "ASR";
        case InstructionType::CMP: return "CMP";
        case InstructionType::EOR: return "EOR";
        case InstructionType::LSL: return "LSL";
        case InstructionType::LSR: return "LSR";
        case InstructionType::MOVE: return "MOVE";
        case InstructionType::NEG: return "NEG";
        case InstructionType::NEGX: return "NEGX";
        case InstructionType::NOT: return "NOT";
        case InstructionType::OR: return "OR";
        case InstructionType::ROL: return "ROL";
        case InstructionType::ROR: return "ROR";
        case InstructionType::ROXL: return "ROXL";
        case InstructionType::ROXR: return "ROXR";
        case InstructionType::SUB: return "SUB";
        case InstructionType::SUBX: return "SUBX";
        case InstructionType::SWAP: return "SWAP";
    }
    return "?";
}

/* ------------------------------------------------------------------------- */
/* Effective address handlers                                                */
/*                                                                           */
/* These are the Java inner classes of CpuCore, moved out of the header.     */
/* Every one of them only uses CpuCore's public interface.                   */
/* ------------------------------------------------------------------------- */

class DataRegisterOperand : public Operand {
public:
    explicit DataRegisterOperand(CpuCore& cpu) : cpu_(cpu) {}

    void init(int32_t param, Size sz) override {
        regNumber = param;
        size = sz;
    }

    int32_t getByte() override { return cpu_.getDataRegisterByte(regNumber); }
    int32_t getWord() override { return cpu_.getDataRegisterWord(regNumber); }
    int32_t getLong() override { return cpu_.getDataRegisterLong(regNumber); }
    int32_t getByteSigned() override { return cpu_.getDataRegisterByteSigned(regNumber); }
    int32_t getWordSigned() override { return cpu_.getDataRegisterWordSigned(regNumber); }

    void setByte(int32_t value) override { cpu_.setDataRegisterByte(regNumber, value); }
    void setWord(int32_t value) override { cpu_.setDataRegisterWord(regNumber, value); }
    void setLong(int32_t value) override { cpu_.setDataRegisterLong(regNumber, value); }

    bool isRegisterMode() override { return true; }
    bool isSR() override { return false; }

    int32_t getTiming() override {
        // always zero
        return 0;
    }

    // used for jmp and jsr
    int32_t getComputedAddress() override {
        throw std::runtime_error("Data Register has no computed address");
    }

    int32_t index() override { return indexValue; }

    std::string toString() const override { return std::string("d") + std::to_string(regNumber); }

private:
    CpuCore& cpu_;
    int32_t regNumber = 0;
    Size size = Size::Unsized;
    static constexpr int32_t indexValue = 0;
};

class AddressRegisterOperand : public Operand {
public:
    explicit AddressRegisterOperand(CpuCore& cpu) : cpu_(cpu) {}

    void init(int32_t param, Size sz) override {
        regNumber = param;
        size = sz;
    }

    int32_t getByte() override { return cpu_.getAddrRegisterByte(regNumber); }
    int32_t getWord() override { return cpu_.getAddrRegisterWord(regNumber); }
    int32_t getLong() override { return cpu_.getAddrRegisterLong(regNumber); }
    int32_t getByteSigned() override { return cpu_.getAddrRegisterByteSigned(regNumber); }
    int32_t getWordSigned() override { return cpu_.getAddrRegisterWordSigned(regNumber); }

    void setByte(int32_t value) override { cpu_.setAddrRegisterByte(regNumber, value); }
    void setWord(int32_t value) override { cpu_.setAddrRegisterWord(regNumber, value); }
    void setLong(int32_t value) override { cpu_.setAddrRegisterLong(regNumber, value); }

    bool isRegisterMode() override { return true; }
    bool isSR() override { return false; }

    int32_t getTiming() override {
        // always zero
        return 0;
    }

    // used for jmp and jsr
    int32_t getComputedAddress() override {
        throw std::runtime_error("Address Register direct has no computed address");
    }

    int32_t index() override { return indexValue; }

    std::string toString() const override { return std::string("a") + std::to_string(regNumber); }

private:
    CpuCore& cpu_;
    int32_t regNumber = 0;
    Size size = Size::Unsized;
    static constexpr int32_t indexValue = 1;
};

class AddressRegisterIndirectOperand : public Operand {
public:
    explicit AddressRegisterIndirectOperand(CpuCore& cpu) : cpu_(cpu) {}

    void init(int32_t param, Size sz) override {
        regNumber = param;
        size = sz;
        address = cpu_.getAddrRegisterLong(regNumber);
    }

    int32_t getByte() override { return cpu_.readMemoryByte(address); }
    int32_t getWord() override { return cpu_.readMemoryWord(address); }
    int32_t getLong() override { return cpu_.readMemoryLong(address); }
    int32_t getByteSigned() override { return cpu_.readMemoryByteSigned(address); }
    int32_t getWordSigned() override { return cpu_.readMemoryWordSigned(address); }

    void setByte(int32_t value) override { cpu_.writeMemoryByte(address, value); }
    void setWord(int32_t value) override { cpu_.writeMemoryWord(address, value); }
    void setLong(int32_t value) override { cpu_.writeMemoryLong(address, value); }

    bool isRegisterMode() override { return false; }
    bool isSR() override { return false; }

    int32_t getTiming() override { return size == Size::Long ? 8 : 4; }

    // used for jmp and jsr
    int32_t getComputedAddress() override { return address; }

    int32_t index() override { return indexValue; }

    std::string toString() const override {
        return std::string("(a") + std::to_string(regNumber) + ")";
    }

private:
    CpuCore& cpu_;
    int32_t regNumber = 0;
    Size size = Size::Unsized;
    int32_t address = 0;
    static constexpr int32_t indexValue = 2;
};

class AddressRegisterPostIncOperand : public Operand {
public:
    explicit AddressRegisterPostIncOperand(CpuCore& cpu) : cpu_(cpu) {}

    void init(int32_t param, Size sz) override {
        regNumber = param;
        size = sz;
        address = cpu_.getAddrRegisterLong(regNumber);

        // in the 68008 At LEAST, moving bytes to the stack will change the stack pointer by 2, not 1
        if (param == 7 && byteCount(size) == 1) {
            cpu_.incrementAddrRegister(regNumber, 2);
        } else {
            cpu_.incrementAddrRegister(regNumber, byteCount(size));
        }
    }

    int32_t getByte() override { return cpu_.readMemoryByte(address); }
    int32_t getWord() override { return cpu_.readMemoryWord(address); }
    int32_t getLong() override { return cpu_.readMemoryLong(address); }
    int32_t getByteSigned() override { return cpu_.readMemoryByteSigned(address); }
    int32_t getWordSigned() override { return cpu_.readMemoryWordSigned(address); }

    void setByte(int32_t value) override { cpu_.writeMemoryByte(address, value); }
    void setWord(int32_t value) override { cpu_.writeMemoryWord(address, value); }
    void setLong(int32_t value) override { cpu_.writeMemoryLong(address, value); }

    bool isRegisterMode() override { return false; }
    bool isSR() override { return false; }

    int32_t getTiming() override { return size == Size::Long ? 8 : 4; }

    // used for jmp and jsr
    int32_t getComputedAddress() override { return address; }

    int32_t index() override { return indexValue; }

    std::string toString() const override {
        return std::string("(a") + std::to_string(regNumber) + ")+";
    }

private:
    CpuCore& cpu_;
    int32_t regNumber = 0;
    Size size = Size::Unsized;
    int32_t address = 0;
    static constexpr int32_t indexValue = 3;
};

class AddressRegisterPreDecOperand : public Operand {
public:
    explicit AddressRegisterPreDecOperand(CpuCore& cpu) : cpu_(cpu) {}

    void init(int32_t param, Size sz) override {
        regNumber = param;
        size = sz;

        // in the 68008 At LEAST, moving bytes to the stack will change the stack pointer by 2, not 1
        if (param == 7 && byteCount(size) == 1) {
            cpu_.decrementAddrRegister(regNumber, 2);
        } else {
            cpu_.decrementAddrRegister(regNumber, byteCount(size));
        }

        address = cpu_.getAddrRegisterLong(regNumber);
    }

    int32_t getByte() override { return cpu_.readMemoryByte(address); }
    int32_t getWord() override { return cpu_.readMemoryWord(address); }
    int32_t getLong() override { return cpu_.readMemoryLong(address); }
    int32_t getByteSigned() override { return cpu_.readMemoryByteSigned(address); }
    int32_t getWordSigned() override { return cpu_.readMemoryWordSigned(address); }

    void setByte(int32_t value) override { cpu_.writeMemoryByte(address, value); }
    void setWord(int32_t value) override { cpu_.writeMemoryWord(address, value); }
    void setLong(int32_t value) override { cpu_.writeMemoryLong(address, value); }

    bool isSR() override { return false; }
    bool isRegisterMode() override { return false; }

    int32_t getTiming() override { return size == Size::Long ? 10 : 6; }

    // used for jmp and jsr, lea, movem
    int32_t getComputedAddress() override { return address; }

    int32_t index() override { return indexValue; }

    std::string toString() const override {
        return std::string("-(a") + std::to_string(regNumber) + ")";
    }

private:
    CpuCore& cpu_;
    int32_t regNumber = 0;
    Size size = Size::Unsized;
    int32_t address = 0;
    static constexpr int32_t indexValue = 4;
};

class AddressRegisterWithDisplacementOperand : public Operand {
public:
    explicit AddressRegisterWithDisplacementOperand(CpuCore& cpu) : cpu_(cpu) {}

    void init(int32_t param, Size sz) override {
        regNumber = param;
        size = sz;
        displacement = cpu_.fetchPCWordSigned();
        address = cpu_.getAddrRegisterLong(regNumber) + displacement;
    }

    int32_t getByte() override { return cpu_.readMemoryByte(address); }
    int32_t getWord() override { return cpu_.readMemoryWord(address); }
    int32_t getLong() override { return cpu_.readMemoryLong(address); }
    int32_t getByteSigned() override { return cpu_.readMemoryByteSigned(address); }
    int32_t getWordSigned() override { return cpu_.readMemoryWordSigned(address); }

    void setByte(int32_t value) override { cpu_.writeMemoryByte(address, value); }
    void setWord(int32_t value) override { cpu_.writeMemoryWord(address, value); }
    void setLong(int32_t value) override { cpu_.writeMemoryLong(address, value); }

    bool isRegisterMode() override { return false; }
    bool isSR() override { return false; }

    int32_t getTiming() override { return size == Size::Long ? 12 : 8; }

    // used for jmp and jsr
    int32_t getComputedAddress() override { return address; }

    int32_t index() override { return indexValue; }

    std::string toString() const override {
        return m68k::util::format("$%x", static_cast<uint32_t>(displacement)) + "(a" +
               std::to_string(regNumber) + ")";
    }

private:
    CpuCore& cpu_;
    int32_t regNumber = 0;
    Size size = Size::Unsized;
    int32_t address = 0;
    int32_t displacement = 0;
    static constexpr int32_t indexValue = 5;
};

class AddressRegisterWithIndexOperand : public Operand {
public:
    explicit AddressRegisterWithIndexOperand(CpuCore& cpu) : cpu_(cpu) {}

    void init(int32_t param, Size sz) override {
        regNumber = param;
        size = sz;
        const int32_t ext = cpu_.fetchPCWordSigned();
        displacement = signExtendByte(ext);
        idxRegNumber = (ext >> 12) & 0x07;
        idxSize = ((ext & 0x0800) == 0x0800 ? Size::Long : Size::Word);
        idxIsAddressReg = ((ext & 0x8000) == 0x8000);
        int32_t idxVal;
        if (idxIsAddressReg) {
            if (idxSize == Size::Word) {
                idxVal = cpu_.getAddrRegisterWordSigned(idxRegNumber);
            } else {
                idxVal = cpu_.getAddrRegisterLong(idxRegNumber);
            }
        } else {
            if (idxSize == Size::Word) {
                idxVal = cpu_.getDataRegisterWordSigned(idxRegNumber);
            } else {
                idxVal = cpu_.getDataRegisterLong(idxRegNumber);
            }
        }
        address = cpu_.getAddrRegisterLong(regNumber) + displacement + idxVal;
    }

    int32_t getByte() override { return cpu_.readMemoryByte(address); }
    int32_t getWord() override { return cpu_.readMemoryWord(address); }
    int32_t getLong() override { return cpu_.readMemoryLong(address); }
    int32_t getByteSigned() override { return cpu_.readMemoryByteSigned(address); }
    int32_t getWordSigned() override { return cpu_.readMemoryWordSigned(address); }

    void setByte(int32_t value) override { cpu_.writeMemoryByte(address, value); }
    void setWord(int32_t value) override { cpu_.writeMemoryWord(address, value); }
    void setLong(int32_t value) override { cpu_.writeMemoryLong(address, value); }

    bool isSR() override { return false; }
    bool isRegisterMode() override { return false; }

    int32_t getTiming() override { return size == Size::Long ? 14 : 10; }

    // used for jmp and jsr
    int32_t getComputedAddress() override { return address; }

    int32_t index() override { return indexValue; }

    std::string toString() const override {
        std::string sb;
        sb += std::to_string(displacement);
        sb += "(a";
        sb += std::to_string(regNumber);
        sb += ",";
        sb += (idxIsAddressReg ? "a" : "d");
        sb += std::to_string(idxRegNumber);
        sb += ext(idxSize);
        sb += ")";
        return sb;
    }

private:
    CpuCore& cpu_;
    int32_t regNumber = 0;
    Size size = Size::Unsized;
    int32_t address = 0;
    int32_t displacement = 0;
    int32_t idxRegNumber = 0;
    Size idxSize = Size::Unsized;
    bool idxIsAddressReg = false;
    static constexpr int32_t indexValue = 6;
};

class AbsoluteShortOperand : public Operand {
public:
    explicit AbsoluteShortOperand(CpuCore& cpu) : cpu_(cpu) {}

    void init(int32_t /*param*/, Size sz) override {
        // don't need the param
        size = sz;
        // yes it is sign extended (the high byte is discarded in 68000 addressing)
        address = cpu_.fetchPCWordSigned();
    }

    int32_t getByte() override { return cpu_.readMemoryByte(address); }
    int32_t getWord() override { return cpu_.readMemoryWord(address); }
    int32_t getLong() override { return cpu_.readMemoryLong(address); }
    int32_t getByteSigned() override { return cpu_.readMemoryByteSigned(address); }
    int32_t getWordSigned() override { return cpu_.readMemoryWordSigned(address); }

    void setByte(int32_t value) override { cpu_.writeMemoryByte(address, value); }
    void setWord(int32_t value) override { cpu_.writeMemoryWord(address, value); }
    void setLong(int32_t value) override { cpu_.writeMemoryLong(address, value); }

    bool isRegisterMode() override { return false; }
    bool isSR() override { return false; }

    int32_t getTiming() override { return size == Size::Long ? 12 : 8; }

    // used for jmp and jsr
    int32_t getComputedAddress() override { return address; }

    int32_t index() override { return indexValue; }

    std::string toString() const override {
        return "$" + m68k::util::toHexString(address) + ".w";
    }

private:
    CpuCore& cpu_;
    Size size = Size::Unsized;
    int32_t address = 0;
    static constexpr int32_t indexValue = 7;
};

class AbsoluteLongOperand : public Operand {
public:
    explicit AbsoluteLongOperand(CpuCore& cpu) : cpu_(cpu) {}

    void init(int32_t /*param*/, Size sz) override {
        // don't need the param
        size = sz;
        address = cpu_.fetchPCLong();
    }

    int32_t getByte() override { return cpu_.readMemoryByte(address); }
    int32_t getWord() override { return cpu_.readMemoryWord(address); }
    int32_t getLong() override { return cpu_.readMemoryLong(address); }
    int32_t getByteSigned() override { return cpu_.readMemoryByteSigned(address); }
    int32_t getWordSigned() override { return cpu_.readMemoryWordSigned(address); }

    void setByte(int32_t value) override { cpu_.writeMemoryByte(address, value); }
    void setWord(int32_t value) override { cpu_.writeMemoryWord(address, value); }
    void setLong(int32_t value) override { cpu_.writeMemoryLong(address, value); }

    bool isRegisterMode() override { return false; }
    bool isSR() override { return false; }

    int32_t getTiming() override { return size == Size::Long ? 16 : 12; }

    // used for jmp and jsr
    int32_t getComputedAddress() override { return address; }

    int32_t index() override { return indexValue; }

    std::string toString() const override {
        return "$" + m68k::util::toHexString(address) + ".l";
    }

private:
    CpuCore& cpu_;
    Size size = Size::Unsized;
    int32_t address = 0;
    static constexpr int32_t indexValue = 8;
};

class PCWithDisplacementOperand : public Operand {
public:
    explicit PCWithDisplacementOperand(CpuCore& cpu) : cpu_(cpu) {}

    void init(int32_t /*param*/, Size sz) override {
        // param not used
        size = sz;
        // the address of pc before displacement read is used
        address = cpu_.getPC();
        displacement = cpu_.fetchPCWordSigned();
        address += displacement;
    }

    int32_t getByte() override { return cpu_.readMemoryByte(address); }
    int32_t getWord() override { return cpu_.readMemoryWord(address); }
    int32_t getLong() override { return cpu_.readMemoryLong(address); }
    int32_t getByteSigned() override { return cpu_.readMemoryByteSigned(address); }
    int32_t getWordSigned() override { return cpu_.readMemoryWordSigned(address); }

    void setByte(int32_t value) override { cpu_.writeMemoryByte(address, value); }
    void setWord(int32_t value) override { cpu_.writeMemoryWord(address, value); }
    void setLong(int32_t value) override { cpu_.writeMemoryLong(address, value); }

    bool isRegisterMode() override { return false; }
    bool isSR() override { return false; }

    int32_t getTiming() override { return size == Size::Long ? 12 : 8; }

    // used for jmp and jsr
    int32_t getComputedAddress() override { return address; }

    int32_t index() override { return indexValue; }

    std::string toString() const override {
        return std::to_string(displacement) + "(pc)";
    }

private:
    CpuCore& cpu_;
    Size size = Size::Unsized;
    int32_t address = 0;
    int32_t displacement = 0;
    static constexpr int32_t indexValue = 9;
};

class PCWithIndexOperand : public Operand {
public:
    explicit PCWithIndexOperand(CpuCore& cpu) : cpu_(cpu) {}

    void init(int32_t /*param*/, Size sz) override {
        // param not used
        size = sz;
        // get pc before it's incremented by the fetch
        address = cpu_.getPC();
        const int32_t xt = cpu_.fetchPCWordSigned();
        displacement = signExtendByte(xt);
        idxRegNumber = (xt >> 12) & 0x07;
        idxSize = ((xt & 0x0800) == 0x0800 ? Size::Long : Size::Word);
        idxIsAddressReg = ((xt & 0x8000) == 0x8000);
        int32_t idxVal;
        if (idxIsAddressReg) {
            if (idxSize == Size::Word) {
                idxVal = cpu_.getAddrRegisterWordSigned(idxRegNumber);
            } else {
                idxVal = cpu_.getAddrRegisterLong(idxRegNumber);
            }
        } else {
            if (idxSize == Size::Word) {
                idxVal = cpu_.getDataRegisterWordSigned(idxRegNumber);
            } else {
                idxVal = cpu_.getDataRegisterLong(idxRegNumber);
            }
        }
        address += displacement + idxVal;
    }

    int32_t getByte() override { return cpu_.readMemoryByte(address); }
    int32_t getWord() override { return cpu_.readMemoryWord(address); }
    int32_t getLong() override { return cpu_.readMemoryLong(address); }
    int32_t getByteSigned() override { return cpu_.readMemoryByteSigned(address); }
    int32_t getWordSigned() override { return cpu_.readMemoryWordSigned(address); }

    void setByte(int32_t value) override { cpu_.writeMemoryByte(address, value); }
    void setWord(int32_t value) override { cpu_.writeMemoryWord(address, value); }
    void setLong(int32_t value) override { cpu_.writeMemoryLong(address, value); }

    bool isRegisterMode() override { return false; }
    bool isSR() override { return false; }

    int32_t getTiming() override { return size == Size::Long ? 14 : 10; }

    // used for jmp and jsr
    int32_t getComputedAddress() override { return address; }

    int32_t index() override { return indexValue; }

    std::string toString() const override {
        std::string sb;
        sb += std::to_string(displacement);
        sb += "(pc,";
        sb += (idxIsAddressReg ? "a" : "d");
        sb += std::to_string(idxRegNumber);
        sb += ext(idxSize);
        sb += ")";
        return sb;
    }

private:
    CpuCore& cpu_;
    Size size = Size::Unsized;
    int32_t address = 0;
    int32_t displacement = 0;
    int32_t idxRegNumber = 0;
    Size idxSize = Size::Unsized;
    bool idxIsAddressReg = false;
    static constexpr int32_t indexValue = 10;
};

class ImmediateOperand : public Operand {
public:
    explicit ImmediateOperand(CpuCore& cpu) : cpu_(cpu) {}

    void init(int32_t /*param*/, Size sz) override {
        // param not used
        size = sz;
        if (size == Size::Long) {
            value = cpu_.fetchPCLong();
        } else {
            // byte & word
            value = cpu_.fetchPCWord();
            if (size == Size::Byte) {
                value &= 0x00ff;
            }
        }
    }

    int32_t getByte() override { return value & 0x00ff; }
    int32_t getWord() override { return value & 0x0000ffff; }
    int32_t getLong() override { return value; }
    int32_t getByteSigned() override { return signExtendByte(value); }
    int32_t getWordSigned() override { return signExtendWord(value); }

    void setByte(int32_t /*value*/) override {
        // should never be called
        throw std::runtime_error("Cannot setByte on source only operand");
    }

    void setWord(int32_t /*value*/) override {
        // should never be called
        throw std::runtime_error("Cannot setWord on source only operand");
    }

    void setLong(int32_t /*value*/) override {
        // should never be called
        throw std::runtime_error("Cannot setLong on source only operand");
    }

    bool isRegisterMode() override { return false; }
    bool isSR() override { return false; }

    int32_t getTiming() override { return size == Size::Long ? 12 : 8; }

    // used for jmp and jsr
    int32_t getComputedAddress() override {
        throw std::runtime_error("Immediate addressing has no computed address");
    }

    int32_t index() override { return indexValue; }

    std::string toString() const override {
        return "#$" + m68k::util::toHexString(value);
    }

private:
    CpuCore& cpu_;
    Size size = Size::Unsized;
    int32_t value = 0;
    static constexpr int32_t indexValue = 11;
};

class StatusRegisterOperand : public Operand {
public:
    explicit StatusRegisterOperand(CpuCore& cpu) : cpu_(cpu) {}

    void init(int32_t /*param*/, Size sz) override {
        // param not used
        size = sz;
        switch (size) {
            case Size::Byte: {
                // condition codes - not privileged
                value = cpu_.getCCRegister();
                name = "ccr";
                break;
            }
            case Size::Word: {
                // status register - priviledged!
                value = cpu_.getSR();
                name = "sr";
                break;
            }
            default: {
                throw std::invalid_argument("Status Register is byte or word access only");
            }
        }
    }

    int32_t getByte() override { return value & 0x00ff; }
    int32_t getWord() override { return value & 0x0000ffff; }

    int32_t getLong() override {
        // should never be called
        throw std::runtime_error("Cannot getLong on status register");
    }

    int32_t getByteSigned() override { return signExtendByte(value); }
    int32_t getWordSigned() override { return signExtendWord(value); }

    void setByte(int32_t v) override { cpu_.setCCRegister(v); }
    void setWord(int32_t v) override { cpu_.setSR(v); }

    void setLong(int32_t /*value*/) override {
        // should never be called
        throw std::runtime_error("Cannot setLong on status register");
    }

    bool isRegisterMode() override { return true; }
    bool isSR() override { return true; }

    int32_t getTiming() override {
        // no ea calc time
        return 0;
    }

    // used for jmp and jsr
    int32_t getComputedAddress() override {
        throw std::runtime_error("Status Register has no computed address");
    }

    int32_t index() override { return indexValue; }

    std::string toString() const override { return name; }

private:
    CpuCore& cpu_;
    Size size = Size::Unsized;
    int32_t value = 0;
    std::string name;
    static constexpr int32_t indexValue = 12;
};

} // namespace

CpuCore::CpuCore() {
    srcEAHandler = nullptr;
    dstEAHandler = nullptr;
    memory = nullptr;
    disasmBuffer.reserve(64);
    initEAHandlers();
}

CpuCore::~CpuCore() = default;

void CpuCore::raiseSafeInterrupt(int32_t priority) {
    safeInterrupt = priority;
}

void CpuCore::executeSafeInterrupt() {
    if (safeInterrupt > this->getInterruptLevel() || safeInterruptTimer++ == 100) {
        this->raiseInterrupt(this->safeInterrupt);
        this->safeInterrupt = 0;
        this->safeInterruptTimer = 0;
    }
}

void CpuCore::setAddressSpace(m68k::memory::AddressSpace* as) {
    this->memory = as;
}

void CpuCore::reset() {
    // TODO: this has to be sent to all external devices - called by RESET instruction
    // and during initialization (is this correct ?)

    reg_ssp = memory->readLong(0);
    addr_regs[7] = reg_ssp;
    reg_pc = memory->readLong(4);
    // supervisor mode, interrupts enabled
    reg_sr = 0x2700;
}

void CpuCore::stop() {
    // TODO: called by STOP instruction - should halt cpu and wait for interrupt
}

int32_t CpuCore::getDataRegisterByte(int32_t reg) {
    return data_regs[reg] & 0x00ff;
}

int32_t CpuCore::getDataRegisterByteSigned(int32_t reg) {
    return signExtendByteValue(data_regs[reg]);
}

int32_t CpuCore::getDataRegisterWord(int32_t reg) {
    return data_regs[reg] & 0x0000ffff;
}

int32_t CpuCore::getDataRegisterWordSigned(int32_t reg) {
    return signExtendWordValue(data_regs[reg]);
}

int32_t CpuCore::getDataRegisterLong(int32_t reg) {
    return data_regs[reg];
}

void CpuCore::setDataRegisterByte(int32_t reg, int32_t value) {
    data_regs[reg] = (data_regs[reg] & static_cast<int32_t>(0xffffff00u)) | (value & 0x00ff);
}

void CpuCore::setDataRegisterWord(int32_t reg, int32_t value) {
    data_regs[reg] = (data_regs[reg] & static_cast<int32_t>(0xffff0000u)) | (value & 0x0000ffff);
}

void CpuCore::setDataRegisterLong(int32_t reg, int32_t value) {
    data_regs[reg] = value;
}

// address registers
int32_t CpuCore::getAddrRegisterByte(int32_t reg) {
    return addr_regs[reg] & 0x00ff;
}

int32_t CpuCore::getAddrRegisterByteSigned(int32_t reg) {
    return signExtendByteValue(addr_regs[reg]);
}

int32_t CpuCore::getAddrRegisterWord(int32_t reg) {
    return addr_regs[reg] & 0x0000ffff;
}

int32_t CpuCore::getAddrRegisterWordSigned(int32_t reg) {
    return signExtendWordValue(addr_regs[reg]);
}

int32_t CpuCore::getAddrRegisterLong(int32_t reg) {
    return addr_regs[reg];
}

void CpuCore::setAddrRegisterByte(int32_t reg, int32_t value) {
    addr_regs[reg] = (addr_regs[reg] & static_cast<int32_t>(0xffffff00u)) | (value & 0x00ff);
    if (reg == 7) {
        if (isSupervisorMode()) {
            reg_ssp = addr_regs[reg];
        } else {
            reg_usp = addr_regs[reg];
        }
    }
}

void CpuCore::setAddrRegisterWord(int32_t reg, int32_t value) {
    addr_regs[reg] = (addr_regs[reg] & static_cast<int32_t>(0xffff0000u)) | (value & 0x0000ffff);
    if (reg == 7) {
        if (isSupervisorMode()) {
            reg_ssp = addr_regs[reg];
        } else {
            reg_usp = addr_regs[reg];
        }
    }
}

void CpuCore::setAddrRegisterLong(int32_t reg, int32_t value) {
    addr_regs[reg] = value;
    if (reg == 7) {
        if (isSupervisorMode()) {
            reg_ssp = value;
        } else {
            reg_usp = value;
        }
    }
}

int32_t CpuCore::getPC() {
    return reg_pc;
}

void CpuCore::setPC(int32_t address) {
    reg_pc = address;
}

// pc fetches - for reading data following instructions and incrementing the PC afterwards
int32_t CpuCore::fetchPCWord() {
    const int32_t value = readMemoryWord(reg_pc);
    reg_pc += 2;
    return value;
}

int32_t CpuCore::fetchPCWordSigned() {
    const int32_t value = readMemoryWordSigned(reg_pc);
    reg_pc += 2;
    return value;
}

int32_t CpuCore::fetchPCLong() {
    const int32_t value = readMemoryLong(reg_pc);
    reg_pc += 4;
    return value;
}

// status reg
int32_t CpuCore::getCCRegister() {
    return reg_sr & 0x00ff;
}

int32_t CpuCore::getSR() {
    return reg_sr;
}

void CpuCore::setCCRegister(int32_t value) {
    reg_sr = (reg_sr & static_cast<int32_t>(0xff00u)) | (value & 0x00ff);
}

void CpuCore::setSR(int32_t value) {
    // check for supervisor bit change
    if (((reg_sr & SUPERVISOR_FLAG) ^ (value & SUPERVISOR_FLAG)) != 0) {
        // if changing via this method don't push/pop sr and pc - this is only called by andi/eori/ori

        if ((value & SUPERVISOR_FLAG) != 0) {
            reg_usp = addr_regs[7];
            addr_regs[7] = reg_ssp;
        } else {
            // switch stacks
            reg_ssp = addr_regs[7];
            addr_regs[7] = reg_usp;
        }
    }
    reg_sr = value;
}

/**
 * Set the SR when coming from an RTE.
 * We might already have been IN supervisor mode when the exception was caused (eg a Trap called in supervisor mode),
 * so we must check the S bit we get back from the stack and possibly STAY in supervisor mode even after the RTE.
 */
void CpuCore::setSR2(int32_t value) {
    // new value of SR, could be user mode or super mode
    reg_sr = value;
    if ((reg_sr & SUPERVISOR_FLAG) == 0) { // we changed back to user mode, change stack pointer
        reg_ssp = addr_regs[7];            // keep supervisor stack pointer
        addr_regs[7] = reg_usp;            // get user stack pointer
    }
}

void CpuCore::setFlags(int32_t flags) {
    // only set CC flags so clear top byte
    reg_sr |= (flags & 0x00ff);
}

void CpuCore::clrFlags(int32_t flags) {
    // we need to invert the flags and then AND them with the SR to clear
    reg_sr &= ~(flags & 0x00ff);
}

bool CpuCore::isFlagSet(int32_t flag) {
    return ((reg_sr & flag) == flag);
}

void CpuCore::calcFlags(InstructionType type, int32_t src, int32_t dst, int32_t result, Size sz) {
    calcFlagsParam(type, src, dst, result, 0, sz);
}

void CpuCore::calcFlagsParam(InstructionType type, int32_t src, int32_t dst, int32_t result, int32_t extraParam,
                             Size sz) {
    const bool Sm = (src & msb(sz)) != 0;
    const bool Dm = (dst & msb(sz)) != 0;
    const bool Rm = (result & msb(sz)) != 0;

    switch (type) {
        case InstructionType::ADD: { // ADD, ADDI, ADDQ
            if ((Sm && Dm && !Rm) || (!Sm && !Dm && Rm)) {
                reg_sr |= V_FLAG;
            } else {
                reg_sr &= ~(V_FLAG);
            }

            if ((Sm && Dm) || (!Rm && Dm) || (Sm && !Rm)) {
                reg_sr |= (C_FLAG | X_FLAG);
            } else {
                reg_sr &= ~(C_FLAG | X_FLAG);
            }

            if (result == 0) {
                reg_sr |= Z_FLAG;
            } else {
                reg_sr &= ~(Z_FLAG);
            }

            if (Rm) {
                reg_sr |= N_FLAG;
            } else {
                reg_sr &= ~(N_FLAG);
            }
            break;
        }

        case InstructionType::ADDX: {
            if ((Sm && Dm && !Rm) || (!Sm && !Dm && Rm)) {
                reg_sr |= V_FLAG;
            } else {
                reg_sr &= ~(V_FLAG);
            }

            if ((Sm && Dm) || (!Rm && Dm) || (Sm && !Rm)) {
                reg_sr |= (C_FLAG | X_FLAG);
            } else {
                reg_sr &= ~(C_FLAG | X_FLAG);
            }

            if (result != 0) {
                reg_sr &= ~(Z_FLAG);
            }

            if (Rm) {
                reg_sr |= N_FLAG;
            } else {
                reg_sr &= ~(N_FLAG);
            }
            break;
        }

        case InstructionType::ASL: {
            // params are different here!
            if (src != 0) {  // shift count
                if (dst != 0) { // last bit out
                    reg_sr |= (C_FLAG | X_FLAG);
                } else {
                    reg_sr &= ~(C_FLAG | X_FLAG);
                }
            }

            if (result == 0) {
                reg_sr |= Z_FLAG;
            } else {
                reg_sr &= ~(Z_FLAG);
            }

            if (Rm) {
                reg_sr |= N_FLAG;
            } else {
                reg_sr &= ~(N_FLAG);
            }

            if (extraParam != 0) { // msb changed
                reg_sr |= V_FLAG;
            } else {
                reg_sr &= ~V_FLAG;
            }

            break;
        }

        case InstructionType::ASR: {
            // params are different here!
            if (src != 0) {  // shift count
                if (dst != 0) { // last bit out
                    reg_sr |= (C_FLAG | X_FLAG);
                } else {
                    reg_sr &= ~(C_FLAG | X_FLAG);
                }
            }

            if (result == 0) {
                reg_sr |= Z_FLAG;
            } else {
                reg_sr &= ~(Z_FLAG);
            }

            if (Rm) {
                reg_sr |= N_FLAG;
            } else {
                reg_sr &= ~(N_FLAG);
            }

            // always cleared
            reg_sr &= ~V_FLAG;

            break;
        }

        case InstructionType::CMP: { // CMP, CMPA, CMPI CMPM
            if (result == 0) {
                reg_sr |= Z_FLAG;
            } else {
                reg_sr &= ~(Z_FLAG);
            }

            if ((!Sm && Dm && !Rm) || (Sm && !Dm && Rm)) {
                reg_sr |= V_FLAG;
            } else {
                reg_sr &= ~(V_FLAG);
            }

            if ((Sm && !Dm) || (Rm && !Dm) || (Sm && Rm)) {
                reg_sr |= C_FLAG;
            } else {
                reg_sr &= ~(C_FLAG);
            }

            if (Rm) {
                reg_sr |= N_FLAG;
            } else {
                reg_sr &= ~(N_FLAG);
            }
            break;
        }
        case InstructionType::LSL:
        case InstructionType::LSR:
        case InstructionType::ROXL:
        case InstructionType::ROXR: {
            if (src > 0) { // shift count
                if (dst != 0) { // last bit out
                    reg_sr |= (C_FLAG | X_FLAG);
                } else {
                    reg_sr &= ~(C_FLAG | X_FLAG);
                }
            }

            if (result == 0) {
                reg_sr |= Z_FLAG;
            } else {
                reg_sr &= ~(Z_FLAG);
            }

            if (Rm) {
                reg_sr |= N_FLAG;
            } else {
                reg_sr &= ~(N_FLAG);
            }

            reg_sr &= ~(V_FLAG);

            break;
        }
        case InstructionType::AND:
        case InstructionType::EOR:
        case InstructionType::MOVE:
        case InstructionType::NOT:
        case InstructionType::OR: {
            if (result == 0) {
                reg_sr |= Z_FLAG;
            } else {
                reg_sr &= ~(Z_FLAG);
            }

            if (Rm) {
                reg_sr |= N_FLAG;
            } else {
                reg_sr &= ~(N_FLAG);
            }

            reg_sr &= ~(V_FLAG | C_FLAG);
            break;
        }
        case InstructionType::NEG: {
            if (Sm && Rm) {
                reg_sr |= V_FLAG;
            } else {
                reg_sr &= ~(V_FLAG);
            }

            if (result == 0) {
                reg_sr |= Z_FLAG;
                reg_sr &= ~(X_FLAG | C_FLAG);
            } else {
                reg_sr &= ~(Z_FLAG);
                reg_sr |= (X_FLAG | C_FLAG);
            }
            if (Rm) {
                reg_sr |= N_FLAG;
            } else {
                reg_sr &= ~(N_FLAG);
            }
            break;
        }
        case InstructionType::NEGX: {
            if (Sm && Rm) {
                reg_sr |= V_FLAG;
            } else {
                reg_sr &= ~(V_FLAG);
            }
            if (Sm || Rm) {
                reg_sr |= (X_FLAG | C_FLAG);
            } else {
                reg_sr &= ~(X_FLAG | C_FLAG);
            }
            if (result == 0) {
                reg_sr |= Z_FLAG;
            } else {
                reg_sr &= ~(Z_FLAG);
            }
            if (Rm) {
                reg_sr |= N_FLAG;
            } else {
                reg_sr &= ~(N_FLAG);
            }
            break;
        }
        case InstructionType::ROL:
        case InstructionType::ROR: {
            if (src > 0) { // shift count
                if (dst != 0) { // last bit out
                    reg_sr |= C_FLAG;
                } else {
                    reg_sr &= ~(C_FLAG);
                }
            }

            if (result == 0) {
                reg_sr |= Z_FLAG;
            } else {
                reg_sr &= ~(Z_FLAG);
            }

            if (Rm) {
                reg_sr |= N_FLAG;
            } else {
                reg_sr &= ~(N_FLAG);
            }

            reg_sr &= ~(V_FLAG);

            break;
        }

        case InstructionType::SUB: {
            if (result == 0) {
                reg_sr |= Z_FLAG;
            } else {
                reg_sr &= ~(Z_FLAG);
            }

            if ((!Sm && Dm && !Rm) || (Sm && !Dm && Rm)) {
                reg_sr |= V_FLAG;
            } else {
                reg_sr &= ~(V_FLAG);
            }

            if ((Sm && !Dm) || (Rm && !Dm) || (Sm && Rm)) {
                reg_sr |= (C_FLAG | X_FLAG);
            } else {
                reg_sr &= ~(C_FLAG | X_FLAG);
            }

            if (Rm) {
                reg_sr |= N_FLAG;
            } else {
                reg_sr &= ~(N_FLAG);
            }
            break;
        }

        case InstructionType::SUBX: {
            if (result != 0) {
                reg_sr &= ~(Z_FLAG);
            }

            if ((!Sm && Dm && !Rm) || (Sm && !Dm && Rm)) {
                reg_sr |= V_FLAG;
            } else {
                reg_sr &= ~(V_FLAG);
            }

            if ((Sm && !Dm) || (Rm && !Dm) || (Sm && Rm)) {
                reg_sr |= (C_FLAG | X_FLAG);
            } else {
                reg_sr &= ~(C_FLAG | X_FLAG);
            }

            if (Rm) {
                reg_sr |= N_FLAG;
            } else {
                reg_sr &= ~(N_FLAG);
            }
            break;
        }

        // swap also affects the SR
        case InstructionType::SWAP: {
            if (result == 0) {
                reg_sr |= Z_FLAG;
            } else {
                reg_sr &= ~(Z_FLAG);
            }

            if (Rm) {
                reg_sr |= N_FLAG;
            } else {
                reg_sr &= ~(N_FLAG);
            }
            reg_sr &= ~(V_FLAG); // these are always set to 0
            reg_sr &= ~(C_FLAG);
            break;
        }
        default: {
            throw std::invalid_argument(std::string("No flags handled for ") + instructionTypeName(type));
        }
    }
}

bool CpuCore::testCC(int32_t cc) {
    const int32_t ccr = reg_sr & 0x001f;

    switch (cc) {
        case 0: // T
            return true;
        case 1: // F
            return false;
        case 2: // HI:
            return ((ccr & (C_FLAG | Z_FLAG)) == 0);
        case 3: // LS:
            return ((ccr & (C_FLAG | Z_FLAG)) != 0);
        case 4: // CC:
            return ((ccr & C_FLAG) == 0);
        case 5: // CS:
            return ((ccr & C_FLAG) != 0);
        case 6: // NE:
            return ((ccr & Z_FLAG) == 0);
        case 7: // EQ:
            return ((ccr & Z_FLAG) != 0);
        case 8: // VC:
            return ((ccr & V_FLAG) == 0);
        case 9: // VS:
            return ((ccr & V_FLAG) != 0);
        case 10: // PL:
            return ((ccr & N_FLAG) == 0);
        case 11: // MI:
            return ((ccr & N_FLAG) != 0);
        case 12: { // GE:
            const int32_t v = ccr & (N_FLAG | V_FLAG);
            return (v == 0 || v == (N_FLAG | V_FLAG));
        }
        case 13: { // LT:
            const int32_t v = ccr & (N_FLAG | V_FLAG);
            return (v == N_FLAG || v == V_FLAG);
        }
        case 14: { // GT:
            const int32_t v = ccr & (N_FLAG | V_FLAG | Z_FLAG);
            return (v == 0 || v == (N_FLAG | V_FLAG));
        }
        case 15: { // LE:
            const int32_t v = ccr & (N_FLAG | V_FLAG | Z_FLAG);
            return ((v & Z_FLAG) != 0 || (v == N_FLAG) || (v == V_FLAG));
        }
        default:
            break;
    }
    throw std::invalid_argument("Invalid Condition Code value!");
}

bool CpuCore::isSupervisorMode() {
    return (reg_sr & SUPERVISOR_FLAG) == SUPERVISOR_FLAG;
}

void CpuCore::pushWord(int32_t value) {
    // do we need to keep the usp or ssp up to date ?
    addr_regs[7] -= 2;
    writeMemoryWord(addr_regs[7], value);
}

void CpuCore::pushLong(int32_t value) {
    // do we need to keep the usp or ssp up to date ?
    addr_regs[7] -= 4;
    writeMemoryLong(addr_regs[7], value);
}

int32_t CpuCore::popWord() {
    // do we need to keep the usp or ssp up to date ?
    const int32_t val = readMemoryWord(addr_regs[7]);
    addr_regs[7] += 2;
    return val;
}

int32_t CpuCore::popLong() {
    // do we need to keep the usp or ssp up to date ?
    const int32_t val = readMemoryLong(addr_regs[7]);
    addr_regs[7] += 4;
    return val;
}

int32_t CpuCore::getUSP() {
    return reg_usp;
}

void CpuCore::setUSP(int32_t address) {
    reg_usp = address;
    if (!isSupervisorMode()) {
        addr_regs[7] = reg_usp;
    }
}

int32_t CpuCore::getSSP() {
    return reg_ssp;
}

void CpuCore::setSSP(int32_t address) {
    reg_ssp = address;

    if (isSupervisorMode()) {
        addr_regs[7] = reg_ssp;
    }
}

void CpuCore::setSupervisorMode(bool enable) {
    if (enable) {
        const int32_t old_sr = reg_sr;
        if ((reg_sr & SUPERVISOR_FLAG) == 0) { // were we in supervisor mode already?....
            reg_sr |= SUPERVISOR_FLAG;         // ...no, so set supervisor bit
            reg_usp = addr_regs[7];            // and change stack pointers
            addr_regs[7] = reg_ssp;
        }

        // save pc and status regs
        pushLong(reg_pc);
        pushWord(old_sr);
    } else {
        // switch back to user mode
        if ((reg_sr & SUPERVISOR_FLAG) != 0) {
            // restore PC and status regs
            reg_sr = popWord();
            reg_pc = popLong();

            // switch stacks
            reg_ssp = addr_regs[7];
            addr_regs[7] = reg_usp;
        }
    }
}

void CpuCore::raiseException(int32_t vector) {
    const int32_t address = (vector & 0x00ff) << 2;

    // don't call setSupervisorMode, do it directly
    const int32_t old_sr = reg_sr; // SR BEFORE the exception

    if ((reg_sr & SUPERVISOR_FLAG) == 0) { // were we in supervisor mode already?....
        reg_sr |= SUPERVISOR_FLAG;         // ...no, so set supervisor bit
        reg_usp = addr_regs[7];            // and change stack pointers
        addr_regs[7] = reg_ssp;
    }

    // save pc and status regs
    pushLong(reg_pc);
    pushWord(old_sr);
    reg_sr &= ~(TRACE_FLAG); // exceptions unset the trace flag

    int32_t xaddress = readMemoryLong(address);
    if (xaddress == 0) {
        // interrupt vector is uninitialised
        // raise a uninitialised interrupt vector exception instead
        // vector 15 == 0x003c
        xaddress = readMemoryLong(0x003c);
        // if this is zero as well the CPU should halt
        if (xaddress == 0) {
            throw CpuException(
                "Interrupt vector not set for uninitialised interrupt vector while trapping uninitialised vector " +
                std::to_string(vector));
        }
    }

    reg_pc = xaddress;
}

void CpuCore::raiseSRException() {
    // always a privilege violation - vector 8
    const int32_t address = 32;

    // switch to supervisor mode
    const int32_t old_sr = reg_sr;

    if ((reg_sr & SUPERVISOR_FLAG) == 0) {
        reg_sr |= SUPERVISOR_FLAG; // set supervisor bit
        // switch stacks
        reg_usp = addr_regs[7];
        addr_regs[7] = reg_ssp;
    }

    // subtly different in that the address of the instruction is pushed rather than the address of the next instruction
    // save pc and status regs - operands fetched in supervisor mode so PC at current address
    pushLong(currentInstructionAddress);
    pushWord(old_sr);

    // todo: handle special exception cases & build stack info

    int32_t xaddress = readMemoryLong(address);
    if (xaddress == 0) {
        // interrupt vector is uninitialised
        // raise a uninitialised interrupt vector exception instead
        // vector 15 == 0x003c
        xaddress = readMemoryLong(0x003c);
        // if this is zero as well the CPU should halt
        if (xaddress == 0) {
            throw CpuException(
                "Interrupt vector not set for uninitialised interrupt vector while trapping uninitialised vector 8");
        }
    }

    reg_pc = xaddress;
}

int32_t CpuCore::getInterruptLevel() {
    return (reg_sr >> 8) & 0x07;
}

void CpuCore::setInterruptLevel(int32_t level) {
    reg_sr &= ~(INTERRUPT_FLAGS_MASK);
    reg_sr |= (level & 0x07) << 8;
}

void CpuCore::raiseInterrupt(int32_t priority) {
    if (priority == 0) {
        return;
    }

    priority &= 0x07;

    // is it higher than the current interrupt mask ?
    if (priority > getInterruptLevel()) {
        // make it an autovectored interrupt
        raiseException(priority + 24);
        setInterruptLevel(priority);
    }
}

// memory interface
int32_t CpuCore::readMemoryByte(int32_t addr) {
    return memory->readByte(addr);
}

int32_t CpuCore::readMemoryByteSigned(int32_t addr) {
    return signExtendByteValue(memory->readByte(addr));
}

int32_t CpuCore::readMemoryWord(int32_t addr) {
    return memory->readWord(addr);
}

int32_t CpuCore::readMemoryWordSigned(int32_t addr) {
    return signExtendWordValue(memory->readWord(addr));
}

int32_t CpuCore::readMemoryLong(int32_t addr) {
    return memory->readLong(addr);
}

void CpuCore::writeMemoryByte(int32_t addr, int32_t value) {
    memory->writeByte(addr, value);
}

void CpuCore::writeMemoryWord(int32_t addr, int32_t value) {
    memory->writeWord(addr, value);
}

void CpuCore::writeMemoryLong(int32_t addr, int32_t value) {
    memory->writeLong(addr, value);
}

Operand* CpuCore::resolveSrcEA(int32_t mode, int32_t reg, Size size) {
    if (mode < 7) {
        srcEAHandler = srcHandlers[static_cast<size_t>(mode)];
    } else {
        srcEAHandler = srcHandlers[static_cast<size_t>(mode + reg)];
    }

    srcEAHandler->init(reg, size);
    return srcEAHandler;
}

// destination EA
Operand* CpuCore::resolveDstEA(int32_t mode, int32_t reg, Size size) {
    if (mode < 7) {
        dstEAHandler = dstHandlers[static_cast<size_t>(mode)];
    } else {
        dstEAHandler = dstHandlers[static_cast<size_t>(mode + reg)];
    }

    dstEAHandler->init(reg, size);
    return dstEAHandler;
}

DisassembledOperand CpuCore::disassembleSrcEA(int32_t address, int32_t mode, int32_t reg, Size sz) {
    return disassembleEA(address, mode, reg, sz, true);
}

DisassembledOperand CpuCore::disassembleDstEA(int32_t address, int32_t mode, int32_t reg, Size sz) {
    return disassembleEA(address, mode, reg, sz, false);
}

DisassembledOperand CpuCore::disassembleEA(int32_t address, int32_t mode, int32_t reg, Size sz, bool is_src) {
    int32_t bytes_read = 0;
    int32_t mem = 0;
    disasmBuffer.clear();

    switch (mode) {
        case 0: {
            disasmBuffer += "d";
            disasmBuffer += std::to_string(reg);
            break;
        }
        case 1: {
            disasmBuffer += "a";
            disasmBuffer += std::to_string(reg);
            break;
        }
        case 2: {
            disasmBuffer += "(a";
            disasmBuffer += std::to_string(reg);
            disasmBuffer += ")";
            break;
        }
        case 3: {
            disasmBuffer += "(a";
            disasmBuffer += std::to_string(reg);
            disasmBuffer += ")+";
            break;
        }
        case 4: {
            disasmBuffer += "-(a";
            disasmBuffer += std::to_string(reg);
            disasmBuffer += ")";
            break;
        }
        case 5: {
            mem = readMemoryWordSigned(address);
            disasmBuffer += m68k::util::format("$%04x", static_cast<uint16_t>(mem));
            disasmBuffer += "(a";
            disasmBuffer += std::to_string(reg);
            disasmBuffer += ")";
            bytes_read = 2;
            break;
        }
        case 6: {
            mem = readMemoryWord(address);
            const int32_t dis = signExtendByteValue(mem);
            disasmBuffer += m68k::util::format("$%02x", static_cast<uint8_t>(dis));
            disasmBuffer += "(a";
            disasmBuffer += std::to_string(reg);
            disasmBuffer += ",";
            disasmBuffer += ((mem & 0x8000) != 0 ? "a" : "d");
            disasmBuffer += std::to_string((mem >> 12) & 0x07);
            disasmBuffer += ((mem & 0x0800) != 0 ? ".l" : ".w");
            disasmBuffer += ")";
            bytes_read = 2;
            break;
        }
        case 7: {
            switch (reg) {
                case 0: {
                    mem = readMemoryWord(address);
                    disasmBuffer += m68k::util::format("$%04x", static_cast<uint16_t>(mem));
                    bytes_read = 2;
                    break;
                }
                case 1: {
                    mem = readMemoryLong(address);
                    disasmBuffer += m68k::util::format("$%08x", static_cast<uint32_t>(mem));
                    bytes_read = 4;
                    break;
                }
                case 2: {
                    mem = readMemoryWordSigned(address);
                    disasmBuffer += m68k::util::format("$%04x", static_cast<uint16_t>(mem));
                    disasmBuffer += "(pc)";
                    bytes_read = 2;
                    break;
                }
                case 3: {
                    mem = readMemoryWord(address);
                    const int32_t dis = signExtendByteValue(mem);
                    disasmBuffer += m68k::util::format("$%02x", static_cast<uint8_t>(dis));
                    disasmBuffer += "(pc,";
                    disasmBuffer += ((mem & 0x8000) != 0 ? "a" : "d");
                    disasmBuffer += std::to_string((mem >> 12) & 0x07);
                    disasmBuffer += ((mem & 0x0800) != 0 ? ".l" : ".w");
                    disasmBuffer += ")";
                    bytes_read = 2;
                    break;
                }
                case 4: {
                    if (is_src) {
                        if (sz == Size::Long) {
                            mem = readMemoryLong(address);
                            bytes_read = 4;
                            disasmBuffer += m68k::util::format("#$%08x", static_cast<uint32_t>(mem));
                        } else {
                            mem = readMemoryWord(address);
                            bytes_read = 2;
                            disasmBuffer += m68k::util::format("#$%04x", static_cast<uint16_t>(mem));

                            if (sz == Size::Byte) {
                                mem &= 0x00ff;
                            }
                        }
                    } else {
                        if (sz == Size::Byte) {
                            disasmBuffer += "ccr";
                        } else {
                            disasmBuffer += "sr";
                        }
                    }
                    break;
                }
                default: {
                    throw std::invalid_argument("Invalid reg specified for mode 7: " + std::to_string(reg));
                }
            }
            break;
        }
        default: {
            throw std::invalid_argument("Invalid mode specified: " + std::to_string(mode));
        }
    }
    return DisassembledOperand(disasmBuffer, bytes_read, mem);
}

// effective address handling
void CpuCore::incrementAddrRegister(int32_t reg, int32_t numBytes) {
    addr_regs[reg] += numBytes;
}

void CpuCore::decrementAddrRegister(int32_t reg, int32_t numBytes) {
    addr_regs[reg] -= numBytes;
}

int32_t CpuCore::signExtendByteValue(int32_t value) const {
    if ((value & 0x80) == 0x80) {
        value |= static_cast<int32_t>(0xffffff00u);
    } else {
        value &= 0x000000ff;
    }
    return value;
}

int32_t CpuCore::signExtendWordValue(int32_t value) const {
    if ((value & 0x8000) == 0x8000) {
        value |= static_cast<int32_t>(0xffff0000u);
    } else {
        value &= 0x0000ffff;
    }
    return value;
}

void CpuCore::initEAHandlers() {
    srcHandlerStore_[0] = std::make_unique<DataRegisterOperand>(*this);
    srcHandlerStore_[1] = std::make_unique<AddressRegisterOperand>(*this);
    srcHandlerStore_[2] = std::make_unique<AddressRegisterIndirectOperand>(*this);
    srcHandlerStore_[3] = std::make_unique<AddressRegisterPostIncOperand>(*this);
    srcHandlerStore_[4] = std::make_unique<AddressRegisterPreDecOperand>(*this);
    srcHandlerStore_[5] = std::make_unique<AddressRegisterWithDisplacementOperand>(*this);
    srcHandlerStore_[6] = std::make_unique<AddressRegisterWithIndexOperand>(*this);
    srcHandlerStore_[7] = std::make_unique<AbsoluteShortOperand>(*this);
    srcHandlerStore_[8] = std::make_unique<AbsoluteLongOperand>(*this);
    srcHandlerStore_[9] = std::make_unique<PCWithDisplacementOperand>(*this);
    srcHandlerStore_[10] = std::make_unique<PCWithIndexOperand>(*this);
    srcHandlerStore_[11] = std::make_unique<ImmediateOperand>(*this);

    dstHandlerStore_[0] = std::make_unique<DataRegisterOperand>(*this);
    dstHandlerStore_[1] = std::make_unique<AddressRegisterOperand>(*this);
    dstHandlerStore_[2] = std::make_unique<AddressRegisterIndirectOperand>(*this);
    dstHandlerStore_[3] = std::make_unique<AddressRegisterPostIncOperand>(*this);
    dstHandlerStore_[4] = std::make_unique<AddressRegisterPreDecOperand>(*this);
    dstHandlerStore_[5] = std::make_unique<AddressRegisterWithDisplacementOperand>(*this);
    dstHandlerStore_[6] = std::make_unique<AddressRegisterWithIndexOperand>(*this);
    dstHandlerStore_[7] = std::make_unique<AbsoluteShortOperand>(*this);
    dstHandlerStore_[8] = std::make_unique<AbsoluteLongOperand>(*this);
    dstHandlerStore_[9] = std::make_unique<PCWithDisplacementOperand>(*this);
    dstHandlerStore_[10] = std::make_unique<PCWithIndexOperand>(*this);
    dstHandlerStore_[11] = std::make_unique<StatusRegisterOperand>(*this);

    for (size_t i = 0; i < srcHandlers.size(); ++i) {
        srcHandlers[i] = srcHandlerStore_[i].get();
        dstHandlers[i] = dstHandlerStore_[i].get();
    }
}

} // namespace m68k::cpu
