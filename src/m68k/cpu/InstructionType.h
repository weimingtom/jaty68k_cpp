#pragma once

namespace m68k::cpu {

/*
 * C++ port of m68k.cpu.InstructionType (Java enum).
 * Java: InstructionType.ADD   ->   C++: InstructionType::ADD
 */
enum class InstructionType {
    ADD, ADDX, AND, ASL, ASR, CMP, EOR, LSL, LSR, MOVE, NEG, NEGX, NOT, OR,
    ROL, ROR, ROXL, ROXR, SUB, SUBX, SWAP
};

} // namespace m68k::cpu
