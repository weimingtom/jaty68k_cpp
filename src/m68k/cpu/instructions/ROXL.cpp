#include "m68k/cpu/instructions/ROXL.h"

#include "m68k/cpu/LambdaInstruction.h"

namespace m68k::cpu::instructions {

ROXL::ROXL(Cpu& cpu) : cpu(cpu) {}

void ROXL::registerInstructions(InstructionSet& is) {
    int32_t base;
    std::shared_ptr<Instruction> i;

    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            // roxl byte imm
            base = 0xe110;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return roxl_byte_imm(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            // roxl word imm
            base = 0xe150;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return roxl_word_imm(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // roxl long imm
            base = 0xe190;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return roxl_long_imm(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }
        for (int32_t imm = 0; imm < 8; imm++) {
            for (int32_t reg = 0; reg < 8; reg++) {
                is.addInstruction(base + (imm << 9) + reg, i);
            }
        }
    }

    for (int32_t sz = 0; sz < 3; sz++) {
        if (sz == 0) {
            // roxl byte reg
            base = 0xe130;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return roxl_byte_reg(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Byte);
                });
        } else if (sz == 1) {
            // roxl word reg
            base = 0xe170;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return roxl_word_reg(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Word);
                });
        } else {
            // roxl long reg
            base = 0xe1b0;
            i = std::make_shared<LambdaInstruction>(
                [this](int32_t opcode) { return roxl_long_reg(opcode); },
                [this](int32_t address, int32_t opcode) {
                    return disassembleOp(address, opcode, Size::Long);
                });
        }
        for (int32_t imm = 0; imm < 8; imm++) {
            for (int32_t reg = 0; reg < 8; reg++) {
                is.addInstruction(base + (imm << 9) + reg, i);
            }
        }
    }

    // roxl word mem
    base = 0xe5c0;
    i = std::make_shared<LambdaInstruction>(
        [this](int32_t opcode) { return roxl_word_mem(opcode); },
        [this](int32_t address, int32_t opcode) {
            return disassembleOp(address, opcode, Size::Word);
        });

    for (int32_t ea_mode = 2; ea_mode < 8; ea_mode++) {
        for (int32_t ea_reg = 0; ea_reg < 8; ea_reg++) {
            if (ea_mode == 7 && ea_reg > 1) {
                break;
            }
            is.addInstruction(base + (ea_mode << 3) + ea_reg, i);
        }
    }
}

int32_t ROXL::roxl_byte_imm(int32_t opcode) {
    //  the state of the X cflag BEFORE each shift is moved into the lsb after each shift. Also, check for 0 count rotate if roxr reg (and not imm)
    //  also if roxl reg, the shift count is mod 64 not 32 finally, cond code reg is set directly here
    int32_t shift = (opcode >> 9) & 0x07;
    if (shift == 0) {
        shift = 8;
    }

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterByte(reg);

    int32_t last_out;
    bool xflag = cpu.isFlagSet(X_FLAG);             // state of the X flag
    int32_t maskFlags;                              // there is no 0 rotate count here
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x80;                    // bit rotated out before ths shift
        d <<= 1;
        if (xflag) {
            d |= 1;                             // if xflag was set before the shift, set LSB
        }
        if (last_out != 0) {                    // bit goes to xflag
            xflag = true;
        } else {
            xflag = false;
        }
    }
    d &= 0xff;
    cpu.setDataRegisterByte(reg, d);
    if (xflag) {
        maskFlags = X_FLAG + C_FLAG;            // if last bit was 1, set flags accordingly
    } else {
        maskFlags = 0;                          // these flags aren't set
    }
    if (d == 0) {
        maskFlags += Z_FLAG;                    // Z flag set if result is 0
    }
    if ((d & 0x80) != 0) {
        maskFlags += N_FLAG;                    // N flag for result (the V flag is always 0)
    }
    cpu.setCCRegister(maskFlags);
    return 6 + shift + shift;
}

int32_t ROXL::roxl_word_imm(int32_t opcode) {
    int32_t shift = (opcode >> 9) & 0x07;
    if (shift == 0) {
        shift = 8;
    }

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterWord(reg);

    int32_t last_out;
    bool xflag = cpu.isFlagSet(X_FLAG);             // state of the X flag
    int32_t maskFlags;                              // there is no 0 rotate count here
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x8000;                  // bit rotated out before ths shift
        d <<= 1;
        if (xflag) {
            d |= 1;                             // if xflag was set before the shift, set LSB
        }
        if (last_out != 0) {                    // bit goes to xflag
            xflag = true;
        } else {
            xflag = false;
        }
    }
    d &= 0x0000ffff;
    cpu.setDataRegisterWord(reg, d);
    if (xflag) {
        maskFlags = X_FLAG + C_FLAG;            // if last bit was 1, set flags accordingly
    } else {
        maskFlags = 0;                          // these flags aren't set
    }
    if (d == 0) {
        maskFlags += Z_FLAG;
    }
    if ((d & 0x8000) != 0) {
        maskFlags += N_FLAG;                    // N flag (the V flag is always 0)
    }
    cpu.setCCRegister(maskFlags);
    return 6 + shift + shift;
}

int32_t ROXL::roxl_long_imm(int32_t opcode) {
    int32_t shift = (opcode >> 9) & 0x07;
    if (shift == 0) {
        shift = 8;
    }

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterLong(reg);
    int32_t last_out;
    bool xflag = cpu.isFlagSet(X_FLAG);             // state of the X flag
    int32_t maskFlags;                              // there is no 0 rotate count here
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x80000000;              // bit rotated out before ths shift
        d <<= 1;
        if (xflag) {
            d |= 1;                             // if xflag was set before the shift, set LSB
        }
        if (last_out != 0) {                    // bit goes to xflag
            xflag = true;
        } else {
            xflag = false;
        }
    }
    cpu.setDataRegisterLong(reg, d);
    if (xflag) {
        maskFlags = X_FLAG + C_FLAG;            // if last bit was 1, set flags accordingly
    } else {
        maskFlags = 0;                          // these flags aren't set
    }
    if (d == 0) {
        maskFlags += Z_FLAG;
    }
    if ((d & 0x80000000) != 0) {
        maskFlags += N_FLAG;                    // N flag (the V flag is always 0)
    }
    cpu.setCCRegister(maskFlags);
    return 8 + shift + shift;
}

int32_t ROXL::roxl_byte_reg(int32_t opcode) {
    int32_t shift = cpu.getDataRegisterLong((opcode >> 9) & 0x07) & 63;

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterByte(reg);

    int32_t last_out;
    bool xflag = cpu.isFlagSet(X_FLAG);             // state of the X flag
    int32_t maskFlags = xflag ? X_FLAG : 0;         // value of X flag at start of operation
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x80;                    // state of last bit befrre rotate
        d <<= 1;
        if (xflag) {                            // X flag was set before the rotate
            d |= 1;                             // so set it in LSB
        }
        if (last_out != 0) {                    // now set new state of X flag according to last bit moved out
            xflag = true;
        } else {
            xflag = false;
        }
    }
    d &= 0xff;
    cpu.setDataRegisterByte(reg, d);
    if (shift != 0) {                               // X flags in unaffected by a 0 count rotate
        if (xflag) {
            maskFlags = X_FLAG + C_FLAG;            // if last bit was 1, set flags accordingly
        } else {
            maskFlags = 0;                          // these flags aren't set
        }
    } else {
        if (maskFlags != 0) {
            maskFlags += C_FLAG;                    //  rotate count : c flag = state of x flag
        }
    }
    if (d == 0) {
        maskFlags += Z_FLAG;
    }
    if ((d & 0x80) != 0) {
        maskFlags += N_FLAG;                        // N flag (the V flag is always 0)
    }
    cpu.setCCRegister(maskFlags);
    return 6 + shift + shift;
}

int32_t ROXL::roxl_word_reg(int32_t opcode) {
    int32_t shift = cpu.getDataRegisterLong((opcode >> 9) & 0x07) & 63;

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterWord(reg);

    int32_t last_out;
    bool xflag = cpu.isFlagSet(X_FLAG);             // state of the X flag
    int32_t maskFlags = xflag ? X_FLAG : 0;         // value of X flag at start of operation
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x8000;                  // state of last bit befrre rotate
        d <<= 1;
        if (xflag) {                            // X flag was set before the rotate
            d |= 1;                             // so set it in LSB
        }
        if (last_out != 0) {                    // now set new state of X flag according to last bit moved out
            xflag = true;
        } else {
            xflag = false;
        }
    }
    d &= 0xffff;
    cpu.setDataRegisterWord(reg, d);
    if (shift != 0) {                               // X flags is unaffected by a 0 count rotate
        if (xflag) {
            maskFlags = X_FLAG + C_FLAG;            // if last bit was 1, set flags accordingly
        } else {
            maskFlags = 0;                          // these flags aren't set
        }
    } else {
        if (maskFlags != 0) {
            maskFlags += C_FLAG;                    //  rotate count : c flag = state of x flag
        }
    }
    if (d == 0) {
        maskFlags += Z_FLAG;
    }
    if ((d & 0x8000) != 0) {
        maskFlags += N_FLAG;                        // N flag (the V flag is always 0)
    }
    cpu.setCCRegister(maskFlags);
    return 6 + shift + shift;
}

int32_t ROXL::roxl_long_reg(int32_t opcode) {
    int32_t shift = cpu.getDataRegisterLong((opcode >> 9) & 0x07) & 63;

    int32_t reg = (opcode & 0x07);
    int32_t d = cpu.getDataRegisterLong(reg);

    int32_t last_out;
    bool xflag = cpu.isFlagSet(X_FLAG);             // state of the X flag
    int32_t maskFlags = xflag ? X_FLAG : 0;         // value of X flag at start of operation
    for (int32_t s = 0; s < shift; s++) {
        last_out = d & 0x80000000;              // state of last bit before rotate
        d <<= 1;
        if (xflag) {                            // X flag was set before the rotate
            d |= 1;                             // so set it in LSB
        }
        if (last_out != 0) {                    // now set new state of X flag according to last bit moved out
            xflag = true;
        } else {
            xflag = false;
        }
    }
    cpu.setDataRegisterLong(reg, d);
    if (shift != 0) {                               // X flags is unaffected by a 0 count rotate
        if (xflag) {
            maskFlags = X_FLAG + C_FLAG;            // if last bit was 1, set flags accordingly
        } else {
            maskFlags = 0;                          // these flags aren't set
        }
    } else {
        if (maskFlags != 0) {
            maskFlags += C_FLAG;                    //  rotate count : c flag = state of x flag
        }
    }
    if (d == 0) {
        maskFlags += Z_FLAG;
    }
    if ((d & 0x80000000) != 0) {
        maskFlags += N_FLAG;                        // N flag (the V flag is always 0)
    }
    cpu.setCCRegister(maskFlags);
    return 8 + shift + shift;
}

int32_t ROXL::roxl_word_mem(int32_t opcode) {
    Operand* op = cpu.resolveDstEA((opcode >> 3) & 0x07, (opcode & 0x07), Size::Word);
    int32_t v = op->getWord();
    int32_t last_out = v & 0x8000;
    v <<= 1;
    if (cpu.isFlagSet(X_FLAG)) {
        v |= 0x01;
    }

    op->setWord(v);
    int32_t maskFlags;
    if (last_out != 0) {
        maskFlags = X_FLAG + C_FLAG;            // if last bit was 1, set flags accordingly
    } else {
        maskFlags = 0;                          // these flags aren't set
    }
    if ((v & 0xffff) == 0) {
        maskFlags += Z_FLAG;
    }
    if ((v & 0x8000) != 0) {
        maskFlags += N_FLAG;                    // N flag (the V flag is always 0)
    }
    cpu.setCCRegister(maskFlags);
    return 8 + op->getTiming();
}

DisassembledInstruction ROXL::disassembleOp(int32_t address, int32_t opcode, Size sz) {
    if ((opcode & 0x00c0) == 0x00c0) {
        //mem mode
        DisassembledOperand src = cpu.disassembleDstEA(address + 2, (opcode >> 3) & 0x07, (opcode & 0x07), sz);
        return DisassembledInstruction(address, opcode, std::string("roxl") + ext(sz), src);
    } else if ((opcode & 0x0020) == 0x0020) {
        //reg mode
        DisassembledOperand src("d" + std::to_string((opcode >> 9) & 0x07));
        DisassembledOperand dst("d" + std::to_string(opcode & 0x07));
        return DisassembledInstruction(address, opcode, std::string("roxl") + ext(sz), src, dst);
    } else {
        //immediate mode
        int32_t count = (opcode >> 9) & 0x07;
        if (count == 0) {
            count = 8;
        }

        DisassembledOperand src("#" + std::to_string(count));
        DisassembledOperand dst("d" + std::to_string(opcode & 0x07));
        return DisassembledInstruction(address, opcode, std::string("roxl") + ext(sz), src, dst);
    }
}

} // namespace m68k::cpu::instructions
