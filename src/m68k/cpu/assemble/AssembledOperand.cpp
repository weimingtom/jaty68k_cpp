#include "m68k/cpu/assemble/AssembledOperand.h"

#include <stdexcept>
#include <string>

namespace m68k::cpu::assemble {

namespace {

// Java appends the enums to a string, which prints their constant name.
const char* addressingModeName(AddressingMode mode) {
    switch (mode) {
        case AddressingMode::NA: return "NA";
        case AddressingMode::IMMEDIATE: return "IMMEDIATE";
        case AddressingMode::IMMEDIATE_DATA: return "IMMEDIATE_DATA";
        case AddressingMode::IMMEDIATE_ADDRESS: return "IMMEDIATE_ADDRESS";
        case AddressingMode::INDIRECT: return "INDIRECT";
        case AddressingMode::INDIRECT_POST: return "INDIRECT_POST";
        case AddressingMode::INDIRECT_PRE: return "INDIRECT_PRE";
        case AddressingMode::INDIRECT_DISP: return "INDIRECT_DISP";
        case AddressingMode::INDIRECT_INDEX: return "INDIRECT_INDEX";
        case AddressingMode::ABSOLUTE_NEAR: return "ABSOLUTE_NEAR";
        case AddressingMode::ABSOLUTE_FAR: return "ABSOLUTE_FAR";
        case AddressingMode::PC_DISP: return "PC_DISP";
        case AddressingMode::PC_INDEX: return "PC_INDEX";
        case AddressingMode::SR: return "SR";
        case AddressingMode::CCR: return "CCR";
        case AddressingMode::REGISTER_LIST: return "REGISTER_LIST";
        case AddressingMode::LABEL: return "LABEL";
    }
    return "?";
}

const char* conditionalName(Conditional conditional) {
    switch (conditional) {
        case Conditional::NA: return "NA";
        case Conditional::T: return "T";
        case Conditional::F: return "F";
        case Conditional::HI: return "HI";
        case Conditional::LS: return "LS";
        case Conditional::CC: return "CC";
        case Conditional::CS: return "CS";
        case Conditional::NE: return "NE";
        case Conditional::EQ: return "EQ";
        case Conditional::VC: return "VC";
        case Conditional::VS: return "VS";
        case Conditional::PL: return "PL";
        case Conditional::MI: return "MI";
        case Conditional::GE: return "GE";
        case Conditional::LT: return "LT";
        case Conditional::GT: return "GT";
        case Conditional::LE: return "LE";
    }
    return "?";
}

} // namespace

std::string AssembledOperand::toString() const {
    return std::string("DisassembledOperand{") +
           "bytes=" + std::to_string(bytes) +
           ", memory_read=" + std::to_string(memory_read) +
           ", operand='" + operand + '\'' +
           ", mode=" + addressingModeName(mode) +
           ", conditional=" + conditionalName(conditional) +
           '}';
}

AssembledOperand::AssembledOperand(const std::string& operand)
    : DisassembledOperand(operand),
      mode(AddressingMode::NA),
      register_(0),
      ext_register(0),
      ext_data(0),
      ext_size(Size::Unsized),
      conditional(Conditional::NA) {}

AssembledOperand::AssembledOperand(const std::string& operand, int32_t bytes, int32_t memory_read)
    : DisassembledOperand(operand, bytes, memory_read),
      mode(AddressingMode::NA),
      register_(0),
      ext_register(0),
      ext_data(0),
      ext_size(Size::Unsized),
      conditional(Conditional::NA) {
    if (bytes > 8) {
        throw std::invalid_argument("Are these the wrong way around ?");
    }
}

AssembledOperand::AssembledOperand(const std::string& operand, int32_t bytes, int32_t memory_read,
                                   AddressingMode mode, Conditional conditional, int32_t register_)
    : DisassembledOperand(operand, bytes, memory_read),
      mode(mode),
      register_(register_),
      ext_register(0),
      ext_data(0),
      ext_size(Size::Unsized),
      conditional(conditional) {
    if (bytes > 8) {
        throw std::invalid_argument("Are these the wrong way around ?");
    }
}

AssembledOperand::AssembledOperand(const std::string& operand, int32_t bytes, int32_t memory_read,
                                   AddressingMode mode, Conditional conditional, int32_t register_,
                                   int32_t ext_register, int32_t ext_data, Size ext_size)
    : DisassembledOperand(operand, bytes, memory_read),
      mode(mode),
      register_(register_),
      ext_register(ext_register),
      ext_data(ext_data),
      ext_size(ext_size),
      conditional(conditional) {
    if (bytes > 8) {
        throw std::invalid_argument("Are these the wrong way around ?");
    }
}

} // namespace m68k::cpu::assemble
