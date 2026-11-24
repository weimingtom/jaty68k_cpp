#pragma once

#include <cstdint>
#include <vector>

namespace m68k::cpu::assemble {

/*
 * C++ port of m68k.cpu.assemble.Binary.
 *
 * Java "implements Comparator<Binary>" has no C++ counterpart to inherit from:
 * the port maps java.util.Comparator to a std::sort/ordered-container predicate
 * lambda (see CONVENTIONS.md), so compare() stays an ordinary member function
 * with the Comparator contract's two operands.
 *
 * The Java "long startAddress" is an int32_t here, like every other address in
 * the port (AddressSpace and MemorySpace take int32_t addresses).
 *
 * The Java field is a java.nio.ByteBuffer; the port stores byte buffers as
 * std::vector<uint8_t> (see m68k/memory/MemorySpace.h for the same mapping).
 * Nothing in the sources mutates a Binary's buffer after construction, so the
 * Java reference is modelled as a copied value here.
 */
class Binary {
public:
    Binary(int32_t startAddress, const std::vector<uint8_t>& data);

    int32_t getStartAddress() const;

    const std::vector<uint8_t>& getData() const;

    int32_t compare(const Binary& o1, const Binary& o2) const;

private:
    int32_t startAddress;
    std::vector<uint8_t> data;
};

} // namespace m68k::cpu::assemble
