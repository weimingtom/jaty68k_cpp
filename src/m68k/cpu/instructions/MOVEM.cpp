#include "m68k/cpu/instructions/MOVEM.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

MOVEM::MOVEM(Cpu& cpu) : cpu(cpu) {}

void MOVEM::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    // reg to mem
    for (int32_t sz = 0; sz < 2; sz++) {
        if (sz == 0) {
            base = 0x4880;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return movem_word_r2m(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word, true);
                });
        } else {
            base = 0x48c0;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return movem_long_r2m(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long, true);
                });
        }

        for (int32_t ea_mode = 2; ea_mode < 8; ea_mode++) {
            if (ea_mode == 3)
                continue;

            for (int32_t ea_reg = 0; ea_reg < 8; ea_reg++) {
                if (ea_mode == 7 && ea_reg > 1)
                    break;

                is.addInstruction(base + (ea_mode << 3) + ea_reg, i);
            }
        }
    }

    // mem to reg
    for (int32_t sz = 0; sz < 2; sz++) {
        if (sz == 0) {
            base = 0x4c80;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return movem_word_m2r(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word, false);
                });
        } else {
            base = 0x4cc0;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return movem_long_m2r(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long, false);
                });
        }

        for (int32_t ea_mode = 2; ea_mode < 8; ea_mode++) {
            if (ea_mode == 4)
                continue;

            for (int32_t ea_reg = 0; ea_reg < 8; ea_reg++) {
                if (ea_mode == 7 && ea_reg > 3)
                    break;

                is.addInstruction(base + (ea_mode << 3) + ea_reg, i);
            }
        }
    }
}

int32_t MOVEM::movem_word_r2m(int32_t opcode) {
    int32_t reglist = cpu.fetchPCWord();
    Operand* dst = cpu.resolveDstEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Word);
    int32_t address = dst->getComputedAddress();
    int32_t count;

    //true up if -(An) addressing mode
    if (((opcode >> 3) & 0x07) == 4) {
        count = putMultipleWordPreDec(opcode & 0x07, reglist, address);
    } else {
        count = putMultipleWord(reglist, address);
    }

    return R2M_Timing[dst->index()] + (count << 2); //reg count * 4
}

int32_t MOVEM::movem_long_r2m(int32_t opcode) {
    int32_t reglist = cpu.fetchPCWord();
    Operand* dst = cpu.resolveDstEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Long);
    int32_t address = dst->getComputedAddress();
    int32_t count;

    //true up if -(An) addressing mode
    if (((opcode >> 3) & 0x07) == 4) {
        count = putMultipleLongPreDec(opcode & 0x07, reglist, address);
    } else {
        count = putMultipleLong(reglist, address);
    }

    return R2M_Timing[dst->index()] + (count << 3); //reg count * 8
}

int32_t MOVEM::movem_word_m2r(int32_t opcode) {
    int32_t reglist = cpu.fetchPCWord();
    Operand* src = cpu.resolveSrcEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Word);
    int32_t address = src->getComputedAddress();
    int32_t count;

    // true up if (An)+ addressing mode
    if (((opcode >> 3) & 0x07) == 3) {
        count = getMultipleWordPostInc(opcode & 0x07, reglist, address);
    } else {
        count = getMultipleWord(reglist, address);
    }

    return M2R_Timing[src->index()] + (count << 2); //reg count * 4
}

int32_t MOVEM::movem_long_m2r(int32_t opcode) {
    int32_t reglist = cpu.fetchPCWord();
    Operand* src = cpu.resolveSrcEA((opcode >> 3) & 0x07, opcode & 0x07, Size::Long);
    int32_t address = src->getComputedAddress();
    int32_t count;

    //true up if (An)+ addressing mode
    if (((opcode >> 3) & 0x07) == 3) {
        count = getMultipleLongPostInc(opcode & 0x07, reglist, address);
    } else {
        count = getMultipleLong(reglist, address);
    }

    return M2R_Timing[src->index()] + (count << 3); //reg count * 8
}

DisassembledInstruction MOVEM::disassembleOp(int32_t address, int32_t opcode, Size sz, bool reg_to_mem) {
    int32_t mode = (opcode >> 3) & 0x07;
    int32_t reg = (opcode & 0x07);
    int32_t reglist = cpu.readMemoryWord(address + 2);
    bool reversed = (mode == 4);

    if (reg_to_mem) {
        //registers to memory
        DisassembledOperand src(regListToString(reglist, reversed), 2, reglist);
        DisassembledOperand dst = cpu.disassembleDstEA(address + 4, mode, reg, sz);
        return DisassembledInstruction(address, opcode, std::string("movem") + ext(sz), src, dst);
    }

    // memory to registers
    DisassembledOperand src = cpu.disassembleSrcEA(address + 4, mode, reg, sz);
    DisassembledOperand dst(regListToString(reglist, reversed), 2, reglist);

    return DisassembledInstruction(address, opcode, std::string("movem") + ext(sz), src, dst);
}

std::string MOVEM::regListToString(int32_t reglist, bool reversed) {
    std::string sb;
    int32_t first = -1;
    int32_t count = 0;

    if (!reversed) {
        //normal mode lsb = d0
        char prefix = 'd';
        int32_t mask = 1;

        for (int32_t i = 0; i < 2; i++) {
            for (int32_t n = 0; n < 8; n++, mask <<= 1) {
                if ((reglist & mask) != 0) {
                    if (first != -1) {
                        count++;
                    } else {
                        first = n;
                    }
                } else {
                    if (first != -1) {
                        if (sb.size() > 0)
                            sb += '/';

                        sb += prefix;
                        sb += std::to_string(first);
                        if (count == 1) {
                            sb += '/';
                            sb += prefix;
                            sb += std::to_string(n - 1);
                        } else if (count > 1) {
                            sb += '-';
                            sb += prefix;
                            sb += std::to_string(n - 1);
                        }

                        count = 0;
                        first = -1;
                    }
                }
            }

            if (first != -1) {
                if (sb.size() > 0)
                    sb += '/';

                sb += prefix;
                sb += std::to_string(first);
                if (count == 1) {
                    sb += '/';
                    sb += prefix;
                    sb += std::to_string(7);
                } else if (count > 1) {
                    sb += '-';
                    sb += prefix;
                    sb += std::to_string(7);
                }

                count = 0;
                first = -1;
            }

            prefix = 'a';
        }
    } else {
        //reverse mode for -(an) lsb = a7
        char prefix = 'd';
        int32_t mask = 0x8000;

        for (int32_t i = 0; i < 2; i++) {
            for (int32_t n = 0; n < 8; n++, mask >>= 1) {
                if ((reglist & mask) != 0) {
                    if (first != -1) {
                        count++;
                    } else {
                        first = n;
                    }
                } else {
                    if (first != -1) {
                        if (sb.size() > 0)
                            sb += '/';

                        sb += prefix;
                        sb += std::to_string(first);
                        if (count == 1) {
                            sb += '/';
                            sb += prefix;
                            sb += std::to_string(n - 1);
                        } else if (count > 1) {
                            sb += '-';
                            sb += prefix;
                            sb += std::to_string(n - 1);
                        }

                        count = 0;
                        first = -1;
                    }
                }
            }

            if (first != -1) {
                if (sb.size() > 0)
                    sb += '/';

                sb += prefix;
                sb += std::to_string(first);
                if (count == 1) {
                    sb += '/';
                    sb += prefix;
                    sb += std::to_string(7);
                } else if (count > 1) {
                    sb += '-';
                    sb += prefix;
                    sb += std::to_string(7);
                }

                count = 0;
                first = -1;
            }

            prefix = 'a';
        }
    }

    return sb;
}

int32_t MOVEM::getMultipleWord(int32_t reglist, int32_t address) {
    int32_t bit = 1;
    int32_t regcount = 0;
    int32_t start = address;

    //assumes d0 is first bit
    for (int32_t n = 0; n < 8; n++) {
        if ((reglist & bit) != 0) {
            //ensure these are sign extended properly
            cpu.setDataRegisterLong(n, cpu.readMemoryWordSigned(start));
            start += 2;
            regcount++;
        }
        bit <<= 1;
    }

    for (int32_t n = 0; n < 8; n++) {
        if ((reglist & bit) != 0) {
            //to ensure these are sign extended properly
            cpu.setAddrRegisterLong(n, cpu.readMemoryWordSigned(start));
            start += 2;
            regcount++;
        }
        bit <<= 1;
    }

    return regcount;
}

int32_t MOVEM::getMultipleWordPostInc(int32_t reg, int32_t reglist, int32_t address) {
    int32_t bit = 1;
    int32_t regcount = 0;
    int32_t start = address;

    //assumes d0 is first bit
    for (int32_t n = 0; n < 8; n++) {
        if ((reglist & bit) != 0) {
            //ensure these are sign extended properly
            cpu.setDataRegisterLong(n, cpu.readMemoryWordSigned(start));
            start += 2;
            regcount++;
        }
        bit <<= 1;
    }

    for (int32_t n = 0; n < 8; n++) {
        if ((reglist & bit) != 0) {
            //to ensure these are sign extended properly
            cpu.setAddrRegisterLong(n, cpu.readMemoryWordSigned(start));
            start += 2;
            regcount++;
        }
        bit <<= 1;
    }

    cpu.setAddrRegisterLong(reg, start);
    return regcount;
}

int32_t MOVEM::getMultipleLong(int32_t reglist, int32_t address) {
    int32_t bit = 1;
    int32_t regcount = 0;
    int32_t start = address;

    //assumes d0 is first bit
    for (int32_t n = 0; n < 8; n++) {
        if ((reglist & bit) != 0) {
            cpu.setDataRegisterLong(n, cpu.readMemoryLong(start));
            start += 4;
            regcount++;
        }
        bit <<= 1;
    }

    for (int32_t n = 0; n < 8; n++) {
        if ((reglist & bit) != 0) {
            cpu.setAddrRegisterLong(n, cpu.readMemoryLong(start));
            start += 4;
            regcount++;
        }
        bit <<= 1;
    }

    return regcount;
}

int32_t MOVEM::getMultipleLongPostInc(int32_t reg, int32_t reglist, int32_t address) {
    int32_t bit = 1;
    int32_t regcount = 0;
    int32_t start = address;

    //assumes d0 is first bit
    for (int32_t n = 0; n < 8; n++) {
        if ((reglist & bit) != 0) {
            cpu.setDataRegisterLong(n, cpu.readMemoryLong(start));
            start += 4;
            regcount++;
        }
        bit <<= 1;
    }

    for (int32_t n = 0; n < 8; n++) {
        if ((reglist & bit) != 0) {
            cpu.setAddrRegisterLong(n, cpu.readMemoryLong(start));
            start += 4;
            regcount++;
        }
        bit <<= 1;
    }

    cpu.setAddrRegisterLong(reg, start);
    return regcount;
}

int32_t MOVEM::putMultipleWord(int32_t reglist, int32_t address) {
    int32_t bit = 1;
    int32_t regcount = 0;
    int32_t start = address;

    //assumes d0 is first bit
    for (int32_t n = 0; n < 8; n++) {
        if ((reglist & bit) != 0) {
            cpu.writeMemoryWord(start, cpu.getDataRegisterWord(n));
            start += 2;
            regcount++;
        }
        bit <<= 1;
    }

    for (int32_t n = 0; n < 8; n++) {
        if ((reglist & bit) != 0) {
            cpu.writeMemoryWord(start, cpu.getAddrRegisterWord(n));
            start += 2;
            regcount++;
        }
        bit <<= 1;
    }

    return regcount;
}

int32_t MOVEM::putMultipleWordPreDec(int32_t reg, int32_t reglist, int32_t address) {
    int32_t bit = 1;
    int32_t regcount = 0;
    int32_t start = address + 2;
    int32_t oldreg = address & 0xffff;

    //assumes a7 is first bit
    for (int32_t n = 0; n < 8; n++) {
        if ((reglist & bit) != 0) {
            start -= 2;
            if (reg == 7 - n) // if the EA register itself is also moved, use initial value
                cpu.writeMemoryWord(start, oldreg); // see comment at beginning of file
            else
                cpu.writeMemoryWord(start, cpu.getAddrRegisterWord(7 - n));
            regcount++;
        }
        bit <<= 1;
    }

    for (int32_t n = 0; n < 8; n++) {
        if ((reglist & bit) != 0) {
            start -= 2;
            cpu.writeMemoryWord(start, cpu.getDataRegisterWord(7 - n));
            regcount++;
        }
        bit <<= 1;
    }

    cpu.setAddrRegisterLong(reg, start);
    return regcount;
}

int32_t MOVEM::putMultipleLong(int32_t reglist, int32_t address) {
    int32_t bit = 1;
    int32_t regcount = 0;
    int32_t start = address;

    //assumes d0 is first bit
    for (int32_t n = 0; n < 8; n++) {
        if ((reglist & bit) != 0) {
            cpu.writeMemoryLong(start, cpu.getDataRegisterLong(n));
            start += 4;
            regcount++;
        }
        bit <<= 1;
    }

    for (int32_t n = 0; n < 8; n++) {
        if ((reglist & bit) != 0) {
            cpu.writeMemoryLong(start, cpu.getAddrRegisterLong(n));
            start += 4;
            regcount++;
        }
        bit <<= 1;
    }

    return regcount;
}

int32_t MOVEM::putMultipleLongPreDec(int32_t reg, int32_t reglist, int32_t address) {
    int32_t bit = 1;
    int32_t regcount = 0;

    int32_t start = address + 4; // avoid double decrease

    int32_t oldreg = address; // the old value of the address resister used as EA, see comment at beginning of file
    //assumes a7 is first bit
    for (int32_t n = 0; n < 8; n++) {
        if ((reglist & bit) != 0) {
            start -= 4;
            if (reg == 7 - n) // if the EA register itself is also moved, use initial value
                cpu.writeMemoryLong(start, oldreg);
            else
                cpu.writeMemoryLong(start, cpu.getAddrRegisterLong(7 - n));
            regcount++;
        }
        bit <<= 1;
    }

    for (int32_t n = 0; n < 8; n++) {
        if ((reglist & bit) != 0) {
            start -= 4;
            cpu.writeMemoryLong(start, cpu.getDataRegisterLong(7 - n));
            regcount++;
        }
        bit <<= 1;
    }

    cpu.setAddrRegisterLong(reg, start);
    return regcount;
}

} // namespace m68k::cpu::instructions
