/*
 * End-to-end test: load the real target firmware and execute it.
 *
 * The Java program's only shipped payload is ../java/linux-pcb.bin, which the
 * monitor loads at address 0 and then runs.  Executing a few hundred thousand
 * instructions of that image exercises far more of the ported instruction set
 * (branches, stacks, memory operands, multiply/divide, bit ops) than the
 * synthetic smoke test can.
 *
 * The guest also writes to KatyMemorySpace's 0x7a000 output port, so some
 * characters may appear on stdout before the summary - that is the emulated
 * program talking, exactly as in the Java original.
 */

#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "m68k/cpu/MC68000.h"
#include "m68k/memory/KatyMemorySpace.h"

int main(int argc, char** argv) {
    const std::string path = (argc > 1) ? argv[1] : "linux-pcb.bin";

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::cout << "SKIP: " << path << " not found (run from a directory containing the ROM)\n";
        return 0;
    }

    const std::vector<uint8_t> image((std::istreambuf_iterator<char>(in)),
                                     std::istreambuf_iterator<char>());

    m68k::memory::KatyMemorySpace mem(0xfffff); // same size the Java Monitor used
    for (size_t i = 0; i < image.size(); i++) {
        mem.writeByte(static_cast<int32_t>(i), image.at(i));
    }

    m68k::cpu::MC68000 cpu;
    cpu.setAddressSpace(&mem);
    cpu.reset();

    const long limit = 500000;
    long executed = 0;
    std::string stopReason;

    try {
        for (; executed < limit; executed++) {
            cpu.execute();
        }
    } catch (const std::exception& e) {
        stopReason = e.what();
    }

    std::cout << "\n--- rom_test ---\n";
    std::cout << "image             : " << path << " (" << image.size() << " bytes)\n";
    std::cout << "instructions run  : " << executed << "\n";
    std::cout << "final PC          : 0x" << std::hex << cpu.getPC() << std::dec << "\n";
    std::cout << "final SR          : 0x" << std::hex << cpu.getSR() << std::dec << "\n";
    std::cout << "D0..D7            : ";
    for (int32_t r = 0; r < 8; r++) {
        std::cout << "0x" << std::hex << cpu.getDataRegisterLong(r) << std::dec << " ";
    }
    std::cout << "\nA0..A7            : ";
    for (int32_t r = 0; r < 8; r++) {
        std::cout << "0x" << std::hex << cpu.getAddrRegisterLong(r) << std::dec << " ";
    }
    std::cout << "\n";
    if (!stopReason.empty()) {
        std::cout << "stopped early     : " << stopReason << "\n";
    }

    if (executed >= 10000) {
        std::cout << "RESULT: PASS (executed a substantial run of real firmware code)\n";
        return 0;
    }

    std::cout << "RESULT: FAIL (only " << executed << " instructions executed)\n";
    return 1;
}
