#pragma once

#include <cstdint>
#include <string>

#include "m68k/cpu/Size.h"

namespace m68k::cpu {

/*
 * C++ port of m68k.cpu.Operand (Java interface).
 */
class Operand {
public:
    virtual ~Operand() = default;

    virtual void init(int32_t param, Size size) = 0;

    virtual int32_t getByte() = 0;
    virtual int32_t getWord() = 0;
    virtual int32_t getLong() = 0;

    virtual int32_t getByteSigned() = 0;
    virtual int32_t getWordSigned() = 0;

    virtual void setByte(int32_t value) = 0;
    virtual void setWord(int32_t value) = 0;
    virtual void setLong(int32_t value) = 0;

    virtual bool isRegisterMode() = 0;
    virtual bool isSR() = 0;
    virtual int32_t getTiming() = 0;

    virtual int32_t getComputedAddress() = 0;
    virtual int32_t index() = 0;

    virtual std::string toString() const = 0;
};

} // namespace m68k::cpu
