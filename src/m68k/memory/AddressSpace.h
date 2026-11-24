#pragma once

#include <cstdint>

namespace m68k::memory {

/*
 * C++ port of m68k.memory.AddressSpace (Java interface).
 */
class AddressSpace {
public:
    virtual ~AddressSpace() = default;

    virtual void reset() = 0;
    virtual int32_t getStartAddress() = 0;
    virtual int32_t getEndAddress() = 0;

    virtual int32_t readByte(int32_t addr) = 0;
    virtual int32_t readWord(int32_t addr) = 0;
    virtual int32_t readLong(int32_t addr) = 0;

    virtual void writeByte(int32_t addr, int32_t value) = 0;
    virtual void writeWord(int32_t addr, int32_t value) = 0;
    virtual void writeLong(int32_t addr, int32_t value) = 0;

    virtual int32_t internalReadByte(int32_t addr) = 0;
    virtual int32_t internalReadWord(int32_t addr) = 0;
    virtual int32_t internalReadLong(int32_t addr) = 0;

    virtual void internalWriteByte(int32_t addr, int32_t value) = 0;
    virtual void internalWriteWord(int32_t addr, int32_t value) = 0;
    virtual void internalWriteLong(int32_t addr, int32_t value) = 0;

    virtual int32_t size() = 0;
};

} // namespace m68k::memory
