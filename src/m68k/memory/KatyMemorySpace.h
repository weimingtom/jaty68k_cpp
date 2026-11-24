#pragma once

#include <cstdint>
#include <fstream>
#include <vector>

#include "m68k/memory/AddressSpace.h"

namespace m68k::memory {

/*
 * C++ port of m68k.memory.KatyMemorySpace.
 *
 * Mirrors the original exactly: a byte-addressable big-endian buffer with a
 * debug log file ("a.log") and the two memory-mapped ports the Java class
 * emulated by hand.
 */
class KatyMemorySpace : public AddressSpace {
public:
    explicit KatyMemorySpace(int32_t size);

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
    void debug(const std::string& s);

    bool debug_ = false;
    std::vector<uint8_t> buffer;
    int32_t size_ = 0;

    const int32_t INPUT_RDF = 0x7C000;
    const int32_t INPUT_ADDRESS = 0x78000;
    const int32_t OUTPUT_TXE = 0x7D000;
    const int32_t OUTPUT_ADDRESS = 0x7A000;

    std::ofstream aWriter;
};

} // namespace m68k::memory
