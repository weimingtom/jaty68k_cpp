#pragma once

#include <cstdint>
#include <vector>

#include "m68k/memory/AddressSpace.h"

namespace m68k::memory {

/*
 * C++ port of m68k.memory.MemorySpace.
 *
 * The Java code used a direct java.nio.ByteBuffer, which is big-endian and
 * bounds-checked; std::vector<uint8_t> reproduces both behaviours (bounds
 * violations throw std::out_of_range, as ByteBuffer threw IndexOutOfBounds).
 */
class MemorySpace : public AddressSpace {
public:
    explicit MemorySpace(int32_t sizeKb);

    void reset() override;
    int32_t getStartAddress() override;
    int32_t getEndAddress() override;

    int32_t readByte(int32_t addr) override;
    int32_t readWord(int32_t addr) override;
    int32_t readLong(int32_t addr) override;

    void writeByte(int32_t addr, int32_t value) override;
    void writeWord(int32_t addr, int32_t value) override;
    void writeLong(int32_t addr, int32_t value) override;

    int32_t internalReadByte(int32_t addr) override;
    int32_t internalReadWord(int32_t addr) override;
    int32_t internalReadLong(int32_t addr) override;

    void internalWriteByte(int32_t addr, int32_t value) override;
    void internalWriteWord(int32_t addr, int32_t value) override;
    void internalWriteLong(int32_t addr, int32_t value) override;

    int32_t size() override;

private:
    std::vector<uint8_t> buffer;
    int32_t size_;
};

} // namespace m68k::memory
