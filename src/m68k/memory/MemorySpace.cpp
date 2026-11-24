#include "m68k/memory/MemorySpace.h"

#include <stdexcept>

namespace m68k::memory {

MemorySpace::MemorySpace(int32_t sizeKb) {
    size_ = sizeKb * 1024;
    buffer.resize(static_cast<size_t>(size_));
}

void MemorySpace::reset() {}

int32_t MemorySpace::getStartAddress() {
    return 0;
}

int32_t MemorySpace::getEndAddress() {
    return size_;
}

int32_t MemorySpace::readByte(int32_t addr) {
    const int32_t v = buffer.at(static_cast<size_t>(addr));
    return v & 0x00ff;
}

int32_t MemorySpace::readWord(int32_t addr) {
    // big-endian, matching java.nio.ByteBuffer.getShort()
    const int32_t hi = buffer.at(static_cast<size_t>(addr));
    const int32_t lo = buffer.at(static_cast<size_t>(addr) + 1);
    const int32_t v = (hi << 8) | lo;
    return v & 0x0000ffff;
}

int32_t MemorySpace::readLong(int32_t addr) {
    // big-endian, matching java.nio.ByteBuffer.getInt()
    const uint32_t b0 = buffer.at(static_cast<size_t>(addr));
    const uint32_t b1 = buffer.at(static_cast<size_t>(addr) + 1);
    const uint32_t b2 = buffer.at(static_cast<size_t>(addr) + 2);
    const uint32_t b3 = buffer.at(static_cast<size_t>(addr) + 3);
    return static_cast<int32_t>((b0 << 24) | (b1 << 16) | (b2 << 8) | b3);
}

void MemorySpace::writeByte(int32_t addr, int32_t value) {
    buffer.at(static_cast<size_t>(addr)) = static_cast<uint8_t>(value & 0x00ff);
}

void MemorySpace::writeWord(int32_t addr, int32_t value) {
    buffer.at(static_cast<size_t>(addr)) = static_cast<uint8_t>((value >> 8) & 0x00ff);
    buffer.at(static_cast<size_t>(addr) + 1) = static_cast<uint8_t>(value & 0x00ff);
}

void MemorySpace::writeLong(int32_t addr, int32_t value) {
    buffer.at(static_cast<size_t>(addr)) = static_cast<uint8_t>((value >> 24) & 0x00ff);
    buffer.at(static_cast<size_t>(addr) + 1) = static_cast<uint8_t>((value >> 16) & 0x00ff);
    buffer.at(static_cast<size_t>(addr) + 2) = static_cast<uint8_t>((value >> 8) & 0x00ff);
    buffer.at(static_cast<size_t>(addr) + 3) = static_cast<uint8_t>(value & 0x00ff);
}

int32_t MemorySpace::internalReadByte(int32_t addr) {
    return readByte(addr);
}

int32_t MemorySpace::internalReadWord(int32_t addr) {
    return readWord(addr);
}

int32_t MemorySpace::internalReadLong(int32_t addr) {
    return readLong(addr);
}

void MemorySpace::internalWriteByte(int32_t addr, int32_t value) {
    writeByte(addr, value);
}

void MemorySpace::internalWriteWord(int32_t addr, int32_t value) {
    writeWord(addr, value);
}

void MemorySpace::internalWriteLong(int32_t addr, int32_t value) {
    writeLong(addr, value);
}

int32_t MemorySpace::size() {
    return size_;
}

} // namespace m68k::memory
