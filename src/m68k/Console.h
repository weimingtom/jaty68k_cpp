#pragma once

#include "m68k/cpu/Cpu.h"
#include "m68k/memory/AddressSpace.h"

namespace m68k {

/*
 * C++ port of m68k.Console.
 *
 * IMPORTANT DEVIATION: the Java class was a Swing window (JFrame/JTextArea) that
 * fed typed characters to the emulated machine through the same two memory
 * mapped ports the guest polls:
 *
 *     memory.writeByte(0x7c000, 0x0);          // clear the "data ready" flag
 *     memory.writeByte(0x78000, character);    // deliver the character
 *     cpu.raiseSafeInterrupt(2);               // let the guest pick it up
 *
 * Swing has no C++ counterpart in a dependency-free port, so the window is
 * replaced by a plain stdin -> memory-port bridge.  The guest-visible behaviour
 * is identical: one character per key, Enter delivered as 0x0d and Backspace as
 * 0x08, exactly the codes the Java KeyEventDispatcher wrote.
 *
 * Guest output is not redirected into a text area either: KatyMemorySpace
 * already writes the 0x7a000 output port straight to std::cout.
 */
class Console {
public:
    Console(m68k::cpu::Cpu& cpu, m68k::memory::AddressSpace& memory);

    /// Blocking loop: forward standard input to the emulated console ports.
    void run();

    /// Push a single character (the Java keyTyped path).
    void keyTyped(int32_t character);

private:
    m68k::cpu::Cpu& cpu;
    m68k::memory::AddressSpace& memory;
};

} // namespace m68k
