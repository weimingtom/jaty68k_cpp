#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "m68k/cpu/InstructionHandler.h"

namespace m68k::cpu::assemble {

/*
 * C++ port of m68k.cpu.assemble.Mnemonics.
 *
 * The Java "public static class Mnemonic" stays a nested class.  Its Java field
 * "handler" is an InstructionHandler reference, so it becomes a shared_ptr; the
 * Java static Map<String, Mnemonic> becomes the static std::unordered_map
 * "mapping", filled by the static initialiser block of the Java class.
 */
class Mnemonics {
public:
    class Mnemonic {
    public:
        const std::shared_ptr<InstructionHandler> handler;
        const int32_t src;
        const int32_t dst;

        Mnemonic(std::shared_ptr<InstructionHandler> handler, int32_t src, int32_t dst);
    };

    static int32_t DATA_REGISTER;
    static int32_t ADDRESS_REGISTER;

private:
    static std::unordered_map<std::string, Mnemonic> mapping;
};

} // namespace m68k::cpu::assemble
