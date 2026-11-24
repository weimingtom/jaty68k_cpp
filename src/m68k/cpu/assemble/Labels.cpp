#include "m68k/cpu/assemble/Labels.h"

#include <set>

#include "m68k/util/Format.h"

namespace m68k::cpu::assemble {

Labels::ExpectedLabel::ExpectedLabel(int32_t address, const std::string& name, bool relative, Size size)
    : name(name), address(address), relative(relative), size(size) {}

Labels::Labels(m68k::memory::AddressSpace* mmu) : mmu(mmu) {}

void Labels::addLabel(const std::string& name, int32_t address) {
    // check exists
    // update in expected
    labels[name] = address;

    // from Assembler
    auto usages = expected.find(name);

    if (usages != expected.end()) {
        for (const auto& usage : usages->second) {
            if (usage->relative) {
                switch (usage->size) {
                    case Size::Byte:
                        mmu->writeByte(usage->address, address - usage->address - 1);
                        break;
                    case Size::Unsized:
                    case Size::Word:
                        mmu->writeWord(usage->address, address - usage->address);
                        break;
                    case Size::Long:
                        mmu->writeLong(usage->address, address - usage->address);
                        break;
                }
            } else {
                mmu->writeLong(usage->address, address);
            }
        }

        expected.erase(name);
    }
}

bool Labels::hasLabel(const std::string& name) {
    return labels.find(name) != labels.end();
}

int32_t Labels::getLabel(const std::string& name, int32_t pc, bool relative, Size size) {
    auto found = labels.find(name);

    int32_t result;
    if (found == labels.end()) {
        result = -1;

        auto expectedLabels = expected.find(name);

        if (expectedLabels == expected.end()) {
            expectedLabels =
                expected.emplace(name, std::unordered_set<std::shared_ptr<ExpectedLabel>>()).first;
        }

        expectedLabels->second.insert(std::make_shared<ExpectedLabel>(pc, name, relative, size));
    } else {
        result = found->second;
    }

    return result;
}

void Labels::printLabels(std::ostream& out) {
    std::set<std::string> sortedLabels;

    for (const auto& entry : labels) {
        sortedLabels.insert(entry.first);
    }
    for (const std::string& label : sortedLabels) {
        int32_t address = labels.at(label);
        out << m68k::util::format("%32s  %08x", label, static_cast<uint32_t>(address)) << std::endl;
    }
    out << "Expected:" << std::endl;
    for (const auto& entry : expected) {
        for (const auto& label : entry.second) {
            out << m68k::util::format("%32s  %08x", entry.first, static_cast<uint32_t>(label->address))
                << std::endl;
        }
    }
}

} // namespace m68k::cpu::assemble
