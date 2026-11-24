#include "m68k/cpu/MC68000.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <typeinfo>

#include "m68k/cpu/instructions/ABCD.h"
#include "m68k/cpu/instructions/ADD.h"
#include "m68k/cpu/instructions/ADDA.h"
#include "m68k/cpu/instructions/ADDI.h"
#include "m68k/cpu/instructions/ADDQ.h"
#include "m68k/cpu/instructions/ADDX.h"
#include "m68k/cpu/instructions/AND.h"
#include "m68k/cpu/instructions/ANDI.h"
#include "m68k/cpu/instructions/ANDI_TO_CCR.h"
#include "m68k/cpu/instructions/ANDI_TO_SR.h"
#include "m68k/cpu/instructions/ASL.h"
#include "m68k/cpu/instructions/ASR.h"
#include "m68k/cpu/instructions/Bcc.h"
#include "m68k/cpu/instructions/BCHG.h"
#include "m68k/cpu/instructions/BCLR.h"
#include "m68k/cpu/instructions/BSET.h"
#include "m68k/cpu/instructions/BTST.h"
#include "m68k/cpu/instructions/CHK.h"
#include "m68k/cpu/instructions/CLR.h"
#include "m68k/cpu/instructions/CMP.h"
#include "m68k/cpu/instructions/CMPA.h"
#include "m68k/cpu/instructions/CMPI.h"
#include "m68k/cpu/instructions/CMPM.h"
#include "m68k/cpu/instructions/DBcc.h"
#include "m68k/cpu/instructions/DIVS.h"
#include "m68k/cpu/instructions/DIVU.h"
#include "m68k/cpu/instructions/EOR.h"
#include "m68k/cpu/instructions/EORI.h"
#include "m68k/cpu/instructions/EORI_TO_CCR.h"
#include "m68k/cpu/instructions/EORI_TO_SR.h"
#include "m68k/cpu/instructions/EXG.h"
#include "m68k/cpu/instructions/EXT.h"
#include "m68k/cpu/instructions/ILLEGAL.h"
#include "m68k/cpu/instructions/JMP.h"
#include "m68k/cpu/instructions/JSR.h"
#include "m68k/cpu/instructions/LEA.h"
#include "m68k/cpu/instructions/LINK.h"
#include "m68k/cpu/instructions/LSL.h"
#include "m68k/cpu/instructions/LSR.h"
#include "m68k/cpu/instructions/MOVE.h"
#include "m68k/cpu/instructions/MOVE_FROM_SR.h"
#include "m68k/cpu/instructions/MOVE_TO_CCR.h"
#include "m68k/cpu/instructions/MOVE_TO_SR.h"
#include "m68k/cpu/instructions/MOVE_USP.h"
#include "m68k/cpu/instructions/MOVEA.h"
#include "m68k/cpu/instructions/MOVEM.h"
#include "m68k/cpu/instructions/MOVEP.h"
#include "m68k/cpu/instructions/MOVEQ.h"
#include "m68k/cpu/instructions/MULS.h"
#include "m68k/cpu/instructions/MULU.h"
#include "m68k/cpu/instructions/NBCD.h"
#include "m68k/cpu/instructions/NEG.h"
#include "m68k/cpu/instructions/NEGX.h"
#include "m68k/cpu/instructions/NOP.h"
#include "m68k/cpu/instructions/NOT.h"
#include "m68k/cpu/instructions/OR.h"
#include "m68k/cpu/instructions/ORI.h"
#include "m68k/cpu/instructions/ORI_TO_CCR.h"
#include "m68k/cpu/instructions/ORI_TO_SR.h"
#include "m68k/cpu/instructions/PEA.h"
#include "m68k/cpu/instructions/RESET.h"
#include "m68k/cpu/instructions/ROL.h"
#include "m68k/cpu/instructions/ROR.h"
#include "m68k/cpu/instructions/ROXL.h"
#include "m68k/cpu/instructions/ROXR.h"
#include "m68k/cpu/instructions/RTE.h"
#include "m68k/cpu/instructions/RTR.h"
#include "m68k/cpu/instructions/RTS.h"
#include "m68k/cpu/instructions/SBCD.h"
#include "m68k/cpu/instructions/Scc.h"
#include "m68k/cpu/instructions/STOP.h"
#include "m68k/cpu/instructions/SUB.h"
#include "m68k/cpu/instructions/SUBA.h"
#include "m68k/cpu/instructions/SUBI.h"
#include "m68k/cpu/instructions/SUBQ.h"
#include "m68k/cpu/instructions/SUBX.h"
#include "m68k/cpu/instructions/SWAP.h"
#include "m68k/cpu/instructions/TAS.h"
#include "m68k/cpu/instructions/TRAP.h"
#include "m68k/cpu/instructions/TRAPV.h"
#include "m68k/cpu/instructions/TST.h"
#include "m68k/cpu/instructions/UNKNOWN.h"
#include "m68k/cpu/instructions/UNLK.h"
#include "m68k/util/Format.h"

namespace m68k::cpu {

// Java: import m68k.cpu.instructions.*;
using namespace m68k::cpu::instructions;

MC68000::MC68000() {
    i_table.fill(nullptr);

    unknown = std::make_shared<UNKNOWN>(*this);
    loaded_ops = 0;
    loadInstructionSet();

    // TODO: change this to logging
    std::cout << "MC68000: Loaded " << loaded_ops << " instructions" << std::endl;
}

MC68000::~MC68000() = default;

int32_t MC68000::execute() {
    // save the PC address
    currentInstructionAddress = reg_pc;
    const int32_t opcode = fetchPCWord();

    Instruction* i = i_table[static_cast<size_t>(opcode & 0xffff)].get();
    if (i != nullptr) {
        try {
            return i->execute(opcode);
        } catch (const std::exception&) {
            throw std::invalid_argument("Invalid opcode: 0x" + util::toHexString(opcode));
        }
    }

    // Illegal Instruction
    debug = false;
    std::cout << util::format("Illegal instruction $%04x at $%x\n", static_cast<uint16_t>(opcode),
                              static_cast<uint32_t>(currentInstructionAddress))
              << std::endl;
    throw std::invalid_argument("Invalid opcode: 0x" + util::toHexString(opcode));
}

template <typename T>
void MC68000::registerHandler() {
    // Java: new T(this).register(this) -- the handler must outlive the
    // instructions it registers, because their lambdas capture it.
    auto handler = std::make_unique<T>(*this);
    handler->registerInstructions(*this);
    handlers.push_back(std::move(handler));
}

void MC68000::loadInstructionSet() {
    registerHandler<ABCD>();
    registerHandler<ADD>();
    registerHandler<ADDA>();
    registerHandler<ADDI>();
    registerHandler<ADDQ>();
    registerHandler<ADDX>();
    registerHandler<AND>();
    registerHandler<ANDI>();
    registerHandler<ANDI_TO_SR>();
    registerHandler<ANDI_TO_CCR>();
    registerHandler<ASL>();
    registerHandler<ASR>();
    registerHandler<Bcc>();
    registerHandler<BCHG>();
    registerHandler<BCLR>();
    registerHandler<BSET>();
    registerHandler<BTST>();
    registerHandler<CHK>();
    registerHandler<CLR>();
    registerHandler<CMP>();
    registerHandler<CMPA>();
    registerHandler<CMPI>();
    registerHandler<CMPM>();
    registerHandler<DBcc>();
    registerHandler<DIVS>();
    registerHandler<DIVU>();
    registerHandler<EOR>();
    registerHandler<EORI>();
    registerHandler<EORI_TO_CCR>();
    registerHandler<EORI_TO_SR>();
    registerHandler<EXG>();
    registerHandler<EXT>();
    registerHandler<ILLEGAL>();
    registerHandler<JMP>();
    registerHandler<JSR>();
    registerHandler<LEA>();
    registerHandler<LINK>();
    registerHandler<LSL>();
    registerHandler<LSR>();
    registerHandler<MOVE>();
    registerHandler<MOVE_TO_CCR>();
    registerHandler<MOVE_TO_SR>();
    registerHandler<MOVE_FROM_SR>();
    registerHandler<MOVE_USP>();
    registerHandler<MOVEA>();
    registerHandler<MOVEM>();
    registerHandler<MOVEP>();
    registerHandler<MOVEQ>();
    registerHandler<MULS>();
    registerHandler<MULU>();
    registerHandler<NBCD>();
    registerHandler<NEG>();
    registerHandler<NEGX>();
    registerHandler<NOP>();
    registerHandler<NOT>();
    registerHandler<OR>();
    registerHandler<ORI>();
    registerHandler<ORI_TO_SR>();
    registerHandler<ORI_TO_CCR>();
    registerHandler<PEA>();
    registerHandler<RESET>();
    registerHandler<ROL>();
    registerHandler<ROR>();
    registerHandler<ROXL>();
    registerHandler<ROXR>();
    registerHandler<RTE>();
    registerHandler<RTR>();
    registerHandler<RTS>();
    registerHandler<SBCD>();
    registerHandler<Scc>();
    registerHandler<STOP>();
    registerHandler<SUB>();
    registerHandler<SUBA>();
    registerHandler<SUBI>();
    registerHandler<SUBQ>();
    registerHandler<SUBX>();
    registerHandler<SWAP>();
    registerHandler<TAS>();
    registerHandler<TRAP>();
    registerHandler<TRAPV>();
    registerHandler<TST>();
    registerHandler<UNLK>();
}

void MC68000::addInstruction(int32_t opcode, std::shared_ptr<Instruction> i) {
    // build the instruction table
    const size_t index = static_cast<size_t>(opcode & 0xffff);
    const std::shared_ptr<Instruction>& current = i_table[index];

    if (current == nullptr) {
        i_table[index] = std::move(i);
        loaded_ops++;
    } else {
        throw std::invalid_argument(
            std::string("Attempted to overwrite existing instruction [") + typeid(*current).name() +
            "] at 0x" + util::format("%04x", static_cast<uint16_t>(opcode)) + " with [" +
            typeid(*i).name() + "]");
    }
}

Instruction* MC68000::getInstructionFor(int32_t opcode) {
    Instruction* i = i_table[static_cast<size_t>(opcode & 0xffff)].get();
    if (i == nullptr) {
        i = unknown.get();
    }

    return i;
}

Instruction* MC68000::getInstructionAt(int32_t address) {
    // read opcode at address
    const int32_t opcode = readMemoryWord(address);
    return getInstructionFor(opcode);
}

} // namespace m68k::cpu
