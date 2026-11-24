#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "m68k/cpu/CpuCore.h"
#include "m68k/cpu/InstructionHandler.h"
#include "m68k/cpu/InstructionSet.h"

namespace m68k::cpu {

/*
 * C++ port of m68k.cpu.MC68000.
 *
 * The 64K opcode table owns its instructions via shared_ptr, which replaces the
 * role the Java garbage collector played for the anonymous Instruction objects.
 *
 * Two lifetimes need explicit care in C++:
 *  - every Instruction produced by a handler captures that handler object, so
 *    the handlers are owned here for as long as the table lives (in Java they
 *    stayed alive because the inner classes referenced them);
 *  - handlers are declared before i_table so that i_table (which holds the
 *    lambdas) is destroyed first.
 */
class MC68000 : public CpuCore, public InstructionSet {
public:
    MC68000();
    ~MC68000() override;

    int32_t execute() override;

    void addInstruction(int32_t opcode, std::shared_ptr<Instruction> i) override;

    Instruction* getInstructionFor(int32_t opcode) override;

    Instruction* getInstructionAt(int32_t address) override;

protected:
    std::vector<std::unique_ptr<InstructionHandler>> handlers;

    std::array<std::shared_ptr<Instruction>, 65536> i_table;
    std::shared_ptr<Instruction> unknown;
    int32_t loaded_ops = 0;

    /// Java: "new T(this).register(this)", keeping the handler alive.
    template <typename T>
    void registerHandler();

    void loadInstructionSet();
};

} // namespace m68k::cpu
