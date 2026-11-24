#pragma once

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "m68k/cpu/Size.h"
#include "m68k/memory/AddressSpace.h"

namespace m68k::cpu::assemble {

/*
 * C++ port of m68k.cpu.assemble.Labels.
 *
 * The private static nested class ExpectedLabel stays nested here.  Java's
 * HashSet<ExpectedLabel> hashes and compares by object identity, because the
 * class overrides neither hashCode nor equals, so the set holds shared_ptr
 * objects, which keeps exactly that identity based behaviour.
 */
class Labels {
private:
    class ExpectedLabel {
    public:
        std::string name;
        int32_t address;
        bool relative;
        Size size;

        ExpectedLabel(int32_t address, const std::string& name, bool relative, Size size);
    };

    m68k::memory::AddressSpace* mmu;
    std::unordered_map<std::string, int32_t> labels;
    std::unordered_map<std::string, std::unordered_set<std::shared_ptr<ExpectedLabel>>> expected;

public:
    explicit Labels(m68k::memory::AddressSpace* mmu);

    void addLabel(const std::string& name, int32_t address);

    bool hasLabel(const std::string& name);

    int32_t getLabel(const std::string& name, int32_t pc, bool relative, Size size);

    void printLabels(std::ostream& out);
};

} // namespace m68k::cpu::assemble
