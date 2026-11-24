#include "m68k/Console.h"

#include <iostream>

namespace m68k {

Console::Console(m68k::cpu::Cpu& cpu, m68k::memory::AddressSpace& memory) : cpu(cpu), memory(memory) {}

void Console::keyTyped(int32_t character) {
    memory.writeByte(0x7c000, 0x0);
    memory.writeByte(0x78000, character);
    cpu.raiseSafeInterrupt(2);
}

void Console::run() {
    char c = 0;
    while (std::cin.get(c)) {
        const unsigned char uc = static_cast<unsigned char>(c);

        if (uc == '\r') {
            continue;
        }
        if (uc == '\n') {
            // Java KeyEvent.VK_ENTER was delivered as 0x0d
            keyTyped(0x0d);
        } else if (uc == 0x08 || uc == 0x7f) {
            // Java KeyEvent.VK_BACK_SPACE was delivered as 0x08
            keyTyped(0x08);
        } else {
            keyTyped(uc);
        }
    }
}

} // namespace m68k
