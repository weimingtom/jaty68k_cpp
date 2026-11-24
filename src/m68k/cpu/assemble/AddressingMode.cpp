#include "m68k/cpu/assemble/AddressingMode.h"

namespace m68k::cpu::assemble {

int32_t getRequiredOperants(AddressingMode mode) {
    switch (mode) {
        case AddressingMode::NA: return 0;
        case AddressingMode::IMMEDIATE: return 1;
        case AddressingMode::IMMEDIATE_DATA: return 1;
        case AddressingMode::IMMEDIATE_ADDRESS: return 1;
        case AddressingMode::INDIRECT: return 1;
        case AddressingMode::INDIRECT_POST: return 2;
        case AddressingMode::INDIRECT_PRE: return 2;
        case AddressingMode::INDIRECT_DISP: return 2;
        case AddressingMode::INDIRECT_INDEX: return 3;
        case AddressingMode::ABSOLUTE_NEAR: return 2;
        case AddressingMode::ABSOLUTE_FAR: return 2;
        case AddressingMode::PC_DISP: return 2;
        case AddressingMode::PC_INDEX: return 3;
        case AddressingMode::SR: return 1;
        case AddressingMode::CCR: return 1;
        case AddressingMode::REGISTER_LIST: return 1;
        case AddressingMode::LABEL: return 1;
    }
    return 0;
}

int32_t bits(AddressingMode mode) {
    switch (mode) {
        case AddressingMode::NA: return 0;
        case AddressingMode::IMMEDIATE: return 7;
        case AddressingMode::IMMEDIATE_DATA: return 0;
        case AddressingMode::IMMEDIATE_ADDRESS: return 1;
        case AddressingMode::INDIRECT: return 2;
        case AddressingMode::INDIRECT_POST: return 3;
        case AddressingMode::INDIRECT_PRE: return 4;
        case AddressingMode::INDIRECT_DISP: return 5;
        case AddressingMode::INDIRECT_INDEX: return 6;
        case AddressingMode::ABSOLUTE_NEAR: return 7;
        case AddressingMode::ABSOLUTE_FAR: return 7;
        case AddressingMode::PC_DISP: return 7;
        case AddressingMode::PC_INDEX: return 7;
        case AddressingMode::SR: return 0;
        case AddressingMode::CCR: return 0;
        case AddressingMode::REGISTER_LIST: return 0;
        case AddressingMode::LABEL: return 0;
    }
    return 0;
}

} // namespace m68k::cpu::assemble
