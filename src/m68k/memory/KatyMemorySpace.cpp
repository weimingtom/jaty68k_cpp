#include "m68k/memory/KatyMemorySpace.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace m68k::memory {

KatyMemorySpace::KatyMemorySpace(int32_t size) {
    aWriter.open("a.log", std::ios::out | std::ios::trunc);
    if (!aWriter.is_open()) {
        throw std::runtime_error("Unable to open a.log");
    }

    this->size_ = size + 1;
    buffer.resize(static_cast<size_t>(size) + 1);
    writeByte(OUTPUT_TXE, 0x2);
}

void KatyMemorySpace::debug(const std::string& s) {
    if (debug_) {
        aWriter << s;
        aWriter << '\n';
    }
}

void KatyMemorySpace::reset() {}

int32_t KatyMemorySpace::getStartAddress() {
    return 0;
}

int32_t KatyMemorySpace::getEndAddress() {
    return size_;
}

int32_t KatyMemorySpace::readByte(int32_t addr) {
    try {
        const int32_t v = buffer.at(static_cast<size_t>(addr));
        const int32_t b = v & 0x00ff;
        if (addr == INPUT_ADDRESS) {
            writeByte(INPUT_RDF, 0x1);
        }
        return b;
    } catch (const std::exception& e) {
        std::cout << "Invalid read byte at address " << std::hex << static_cast<uint32_t>(addr) << std::dec
                  << "\n";
        throw std::runtime_error(e.what());
    }
}

int32_t KatyMemorySpace::readWord(int32_t addr) {
    try {
        const int32_t hi = buffer.at(static_cast<size_t>(addr));
        const int32_t lo = buffer.at(static_cast<size_t>(addr) + 1);
        const int32_t v = (hi << 8) | lo;
        return v & 0x0000ffff;
    } catch (const std::exception& e) {
        std::cout << "Invalid read word at address " << std::hex << static_cast<uint32_t>(addr) << std::dec
                  << "\n";
        throw std::runtime_error(e.what());
    }
}

int32_t KatyMemorySpace::readLong(int32_t addr) {
    try {
        const uint32_t b0 = buffer.at(static_cast<size_t>(addr));
        const uint32_t b1 = buffer.at(static_cast<size_t>(addr) + 1);
        const uint32_t b2 = buffer.at(static_cast<size_t>(addr) + 2);
        const uint32_t b3 = buffer.at(static_cast<size_t>(addr) + 3);
        const int32_t l = static_cast<int32_t>((b0 << 24) | (b1 << 16) | (b2 << 8) | b3);

        if (addr == INPUT_ADDRESS) {
            writeByte(INPUT_RDF, 0x1);
        }
        return l;
    } catch (const std::exception& e) {
        std::cout << "Invalid read long at address " << std::hex << static_cast<uint32_t>(addr) << std::dec
                  << "\n";
        throw std::runtime_error(e.what());
    }
}

void KatyMemorySpace::writeByte(int32_t addr, int32_t value) {
    const int8_t b = static_cast<int8_t>(value & 0x00ff);
    buffer.at(static_cast<size_t>(addr)) = static_cast<uint8_t>(b);

    if (addr == OUTPUT_ADDRESS) {
        // The Java GUI appended every character to the JTextArea as it arrived,
        // so the emulated program's prompt was visible immediately. Flush here
        // to keep that behaviour: the guest may print a prompt without a
        // trailing newline and then wait for input.
        std::cout << static_cast<char>(b) << std::flush;
    }
}

void KatyMemorySpace::writeWord(int32_t addr, int32_t value) {
    buffer.at(static_cast<size_t>(addr)) = static_cast<uint8_t>((value >> 8) & 0x00ff);
    buffer.at(static_cast<size_t>(addr) + 1) = static_cast<uint8_t>(value & 0x00ff);
}

void KatyMemorySpace::writeLong(int32_t addr, int32_t value) {
    buffer.at(static_cast<size_t>(addr)) = static_cast<uint8_t>((value >> 24) & 0x00ff);
    buffer.at(static_cast<size_t>(addr) + 1) = static_cast<uint8_t>((value >> 16) & 0x00ff);
    buffer.at(static_cast<size_t>(addr) + 2) = static_cast<uint8_t>((value >> 8) & 0x00ff);
    buffer.at(static_cast<size_t>(addr) + 3) = static_cast<uint8_t>(value & 0x00ff);
}

int32_t KatyMemorySpace::internalReadByte(int32_t addr) {
    return readByte(addr);
}

int32_t KatyMemorySpace::internalReadWord(int32_t addr) {
    return readWord(addr);
}

int32_t KatyMemorySpace::internalReadLong(int32_t addr) {
    return readLong(addr);
}

void KatyMemorySpace::internalWriteByte(int32_t addr, int32_t value) {
    writeByte(addr, value);
}

void KatyMemorySpace::internalWriteWord(int32_t addr, int32_t value) {
    writeWord(addr, value);
}

void KatyMemorySpace::internalWriteLong(int32_t addr, int32_t value) {
    writeLong(addr, value);
}

int32_t KatyMemorySpace::size() {
    return size_;
}

} // namespace m68k::memory
