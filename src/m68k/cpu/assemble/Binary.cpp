#include "m68k/cpu/assemble/Binary.h"

namespace m68k::cpu::assemble {

Binary::Binary(int32_t startAddress, const std::vector<uint8_t>& data)
    : startAddress(startAddress), data(data) {}

int32_t Binary::getStartAddress() const {
    return startAddress;
}

const std::vector<uint8_t>& Binary::getData() const {
    return data;
}

int32_t Binary::compare(const Binary& o1, const Binary& o2) const {
    return o1.startAddress < o2.startAddress ? -1 :
            o1.startAddress > o2.startAddress ? 1 : 0;
}

} // namespace m68k::cpu::assemble
