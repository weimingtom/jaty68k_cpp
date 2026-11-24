#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <utility>

#include "m68k/cpu/Instruction.h"

namespace m68k::cpu {

/*
 * Replacement for the Java anonymous inner classes that implement Instruction
 * inside each instruction handler, e.g.
 *
 *   Java:
 *       i = new Instruction() {
 *               public int execute(int opcode) { return add_byte_dn_dest(opcode); }
 *               public DisassembledInstruction disassemble(int address, int opcode) {
 *                   return disassembleOp(address, opcode, Size.Byte);
 *               }
 *           };
 *
 *   C++:
 *       auto i = std::make_shared<LambdaInstruction>(
 *           [this](int32_t opcode) { return add_byte_dn_dest(opcode); },
 *           [this](int32_t address, int32_t opcode) { return disassembleOp(address, opcode, Size::Byte); });
 *
 *   is.addInstruction(base + (reg << 9) + (ea_mode << 3) + ea_reg, i);
 */
class LambdaInstruction : public Instruction {
public:
    using ExecuteFn = std::function<int32_t(int32_t)>;
    using DisassembleFn = std::function<DisassembledInstruction(int32_t, int32_t)>;

    LambdaInstruction(ExecuteFn executeFn, DisassembleFn disassembleFn)
        : execute_(std::move(executeFn)), disassemble_(std::move(disassembleFn)) {}

    int32_t execute(int32_t opcode) override { return execute_(opcode); }

    DisassembledInstruction disassemble(int32_t address, int32_t opcode) override {
        return disassemble_(address, opcode);
    }

private:
    ExecuteFn execute_;
    DisassembleFn disassemble_;
};

} // namespace m68k::cpu
