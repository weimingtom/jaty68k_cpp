#include "m68k/cpu/assemble/Assembler.h"

#include <cctype>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "m68k/cpu/Size.h"
#include "m68k/cpu/NullCpu.h"
#include "m68k/cpu/assemble/AssembledInstruction.h"
#include "m68k/cpu/assemble/AssembledOperand.h"
#include "m68k/cpu/assemble/OperandParser.h"
#include "m68k/cpu/assemble/ParseException.h"
#include "m68k/cpu/instructions/ABCD.h"
#include "m68k/cpu/instructions/ADD.h"
#include "m68k/cpu/instructions/ADDA.h"
#include "m68k/cpu/instructions/ADDI.h"
#include "m68k/cpu/instructions/ADDQ.h"
#include "m68k/cpu/instructions/ADDX.h"
#include "m68k/cpu/instructions/AND.h"
#include "m68k/cpu/instructions/ASL.h"
#include "m68k/cpu/instructions/Bcc.h"
#include "m68k/cpu/instructions/BCHG.h"
#include "m68k/cpu/instructions/CHK.h"
#include "m68k/cpu/instructions/CMP.h"
#include "m68k/cpu/instructions/CMPA.h"
#include "m68k/cpu/instructions/CMPI.h"
#include "m68k/cpu/instructions/CMPM.h"
#include "m68k/cpu/instructions/DBcc.h"
#include "m68k/cpu/instructions/DIVS.h"
#include "m68k/cpu/instructions/DIVU.h"
#include "m68k/cpu/instructions/EOR.h"
#include "m68k/cpu/instructions/EXG.h"
#include "m68k/cpu/instructions/EXT.h"
#include "m68k/cpu/instructions/LEA.h"
#include "m68k/cpu/instructions/LINK.h"
#include "m68k/cpu/instructions/MOVE.h"
#include "m68k/cpu/instructions/MOVEM.h"
#include "m68k/cpu/instructions/MOVEP.h"
#include "m68k/cpu/instructions/MOVEQ.h"
#include "m68k/cpu/instructions/MULS.h"
#include "m68k/cpu/instructions/MULU.h"
#include "m68k/cpu/instructions/NEGX.h"
#include "m68k/cpu/instructions/NOP.h"
#include "m68k/cpu/instructions/OR.h"
#include "m68k/cpu/instructions/PEA.h"
#include "m68k/cpu/instructions/RESET.h"
#include "m68k/cpu/instructions/RTE.h"
#include "m68k/cpu/instructions/RTR.h"
#include "m68k/cpu/instructions/RTS.h"
#include "m68k/cpu/instructions/STOP.h"
#include "m68k/cpu/instructions/SUB.h"
#include "m68k/cpu/instructions/SUBA.h"
#include "m68k/cpu/instructions/SUBI.h"
#include "m68k/cpu/instructions/SUBQ.h"
#include "m68k/cpu/instructions/SUBX.h"
#include "m68k/cpu/instructions/TRAP.h"
#include "m68k/cpu/instructions/TRAPV.h"
#include "m68k/cpu/instructions/UNLK.h"

namespace m68k::cpu::assemble {

namespace {

/*
 * Java created every handler with a null Cpu ("new EXG(null)") because the
 * Assembler never runs one: its only job is to recognise the mnemonic, and the
 * code that would hand the instruction to the handler is still a TODO in the
 * original (see parseLine).  A C++ reference cannot be null, so the handlers
 * are bound to a null reference instead; they store it and never read it.
 */
Cpu& nullCpu() {
    // A real, silent object rather than a null reference: same "no Cpu here"
    // state the Java code expressed with null, but without undefined behaviour.
    static NullCpu instance;
    return instance;
}

/// Java String.trim() - strips every leading/trailing character <= ' '.
std::string trim(const std::string& value) {
    size_t start = 0;
    size_t end = value.size();

    while (start < end && static_cast<unsigned char>(value.at(start)) <= ' ') {
        start++;
    }
    while (end > start && static_cast<unsigned char>(value.at(end - 1)) <= ' ') {
        end--;
    }

    return value.substr(start, end - start);
}

/// Java String.toLowerCase() - per character, as with String.toUpperCase().
std::string toLowerCase(const std::string& value) {
    std::string result = value;

    for (char& ch : result) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    return result;
}

} // namespace

std::unordered_map<std::string, std::shared_ptr<InstructionHandler>> Assembler::commandMapping;

Assembler::Assembler(m68k::memory::AddressSpace* mmu) : mmu(mmu), labels(mmu) {
    // Java declared this table in an instance initializer block, so it was built
    // again for every constructed Assembler; the same put() calls happen here.
    // Each entry creates its own handler, exactly like the original "new X(null)".

    // data transfer instructions
    commandMapping["exg"] = std::make_shared<instructions::EXG>(nullCpu());
    commandMapping["lea"] = std::make_shared<instructions::LEA>(nullCpu());
    commandMapping["link"] = std::make_shared<instructions::LINK>(nullCpu());
    commandMapping["move"] = std::make_shared<instructions::MOVE>(nullCpu());
    commandMapping["movea"] = std::make_shared<instructions::MOVE>(nullCpu());
    commandMapping["movem"] = std::make_shared<instructions::MOVEM>(nullCpu());
    commandMapping["moveq"] = std::make_shared<instructions::MOVEQ>(nullCpu());
    commandMapping["movep"] = std::make_shared<instructions::MOVEP>(nullCpu());
    commandMapping["pea"] = std::make_shared<instructions::PEA>(nullCpu());
    commandMapping["unlk"] = std::make_shared<instructions::UNLK>(nullCpu());

    // arithmetic
    commandMapping["abcd"] = std::make_shared<instructions::ABCD>(nullCpu());
    commandMapping["add"] = std::make_shared<instructions::ADD>(nullCpu());
    commandMapping["adda"] = std::make_shared<instructions::ADDA>(nullCpu());
    commandMapping["addi"] = std::make_shared<instructions::ADDI>(nullCpu());
    commandMapping["addq"] = std::make_shared<instructions::ADDQ>(nullCpu());
    commandMapping["addx"] = std::make_shared<instructions::ADDX>(nullCpu());

    commandMapping["sub"] = std::make_shared<instructions::SUB>(nullCpu());
    commandMapping["suba"] = std::make_shared<instructions::SUBA>(nullCpu());
    commandMapping["subi"] = std::make_shared<instructions::SUBI>(nullCpu());
    commandMapping["subq"] = std::make_shared<instructions::SUBQ>(nullCpu());
    commandMapping["subx"] = std::make_shared<instructions::SUBX>(nullCpu());

    commandMapping["cmp"] = std::make_shared<instructions::CMP>(nullCpu());
    commandMapping["cmpa"] = std::make_shared<instructions::CMPA>(nullCpu());
    commandMapping["cmpi"] = std::make_shared<instructions::CMPI>(nullCpu());
    commandMapping["cmpm"] = std::make_shared<instructions::CMPM>(nullCpu());

    commandMapping["divs"] = std::make_shared<instructions::DIVS>(nullCpu());
    commandMapping["divu"] = std::make_shared<instructions::DIVU>(nullCpu());
    commandMapping["muls"] = std::make_shared<instructions::MULS>(nullCpu());
    commandMapping["mulu"] = std::make_shared<instructions::MULU>(nullCpu());

    commandMapping["ext"] = std::make_shared<instructions::EXT>(nullCpu());

    // logical
    commandMapping["and"] = std::make_shared<instructions::AND>(nullCpu());
    commandMapping["andi"] = std::make_shared<instructions::AND>(nullCpu());
    commandMapping["or"] = std::make_shared<instructions::OR>(nullCpu());
    commandMapping["ori"] = std::make_shared<instructions::OR>(nullCpu());
    commandMapping["eor"] = std::make_shared<instructions::EOR>(nullCpu());
    commandMapping["eori"] = std::make_shared<instructions::EOR>(nullCpu());

    // shifts
    commandMapping["asl"] = std::make_shared<instructions::ASL>(nullCpu());
    commandMapping["asr"] = std::make_shared<instructions::ASL>(nullCpu());
    commandMapping["lsl"] = std::make_shared<instructions::ASL>(nullCpu());
    commandMapping["lsr"] = std::make_shared<instructions::ASL>(nullCpu());
    commandMapping["roxl"] = std::make_shared<instructions::ASL>(nullCpu());
    commandMapping["roxr"] = std::make_shared<instructions::ASL>(nullCpu());
    commandMapping["rol"] = std::make_shared<instructions::ASL>(nullCpu());
    commandMapping["ror"] = std::make_shared<instructions::ASL>(nullCpu());

    // bits
    commandMapping["bchg"] = std::make_shared<instructions::BCHG>(nullCpu());
    commandMapping["btst"] = std::make_shared<instructions::BCHG>(nullCpu());
    commandMapping["bclr"] = std::make_shared<instructions::BCHG>(nullCpu());
    commandMapping["bset"] = std::make_shared<instructions::BCHG>(nullCpu());

    // chk
    commandMapping["chk"] = std::make_shared<instructions::CHK>(nullCpu());

    // negate
    commandMapping["negx"] = std::make_shared<instructions::NEGX>(nullCpu());
    commandMapping["clr"] = std::make_shared<instructions::NEGX>(nullCpu());
    commandMapping["neg"] = std::make_shared<instructions::NEGX>(nullCpu());
    commandMapping["not"] = std::make_shared<instructions::NEGX>(nullCpu());

    // control flow
    // branch
    commandMapping["bra"] = std::make_shared<instructions::Bcc>(nullCpu());
    commandMapping["bsr"] = std::make_shared<instructions::Bcc>(nullCpu());
    commandMapping["bhi"] = std::make_shared<instructions::Bcc>(nullCpu());
    commandMapping["bls"] = std::make_shared<instructions::Bcc>(nullCpu());
    commandMapping["bcc"] = std::make_shared<instructions::Bcc>(nullCpu());
    commandMapping["bcs"] = std::make_shared<instructions::Bcc>(nullCpu());
    commandMapping["bne"] = std::make_shared<instructions::Bcc>(nullCpu());
    commandMapping["beq"] = std::make_shared<instructions::Bcc>(nullCpu());
    commandMapping["bvc"] = std::make_shared<instructions::Bcc>(nullCpu());
    commandMapping["bvs"] = std::make_shared<instructions::Bcc>(nullCpu());
    commandMapping["bpl"] = std::make_shared<instructions::Bcc>(nullCpu());
    commandMapping["bmi"] = std::make_shared<instructions::Bcc>(nullCpu());
    commandMapping["bge"] = std::make_shared<instructions::Bcc>(nullCpu());
    commandMapping["blt"] = std::make_shared<instructions::Bcc>(nullCpu());
    commandMapping["bgt"] = std::make_shared<instructions::Bcc>(nullCpu());
    commandMapping["ble"] = std::make_shared<instructions::Bcc>(nullCpu());
    // dec and brach
    commandMapping["dbra"] = std::make_shared<instructions::DBcc>(nullCpu());
    commandMapping["dbsr"] = std::make_shared<instructions::DBcc>(nullCpu());
    commandMapping["dbhi"] = std::make_shared<instructions::DBcc>(nullCpu());
    commandMapping["dbls"] = std::make_shared<instructions::DBcc>(nullCpu());
    commandMapping["dbcc"] = std::make_shared<instructions::DBcc>(nullCpu());
    commandMapping["dbcs"] = std::make_shared<instructions::DBcc>(nullCpu());
    commandMapping["dbne"] = std::make_shared<instructions::DBcc>(nullCpu());
    commandMapping["dbeq"] = std::make_shared<instructions::DBcc>(nullCpu());
    commandMapping["dbvc"] = std::make_shared<instructions::DBcc>(nullCpu());
    commandMapping["dbvs"] = std::make_shared<instructions::DBcc>(nullCpu());
    commandMapping["dbpl"] = std::make_shared<instructions::DBcc>(nullCpu());
    commandMapping["dbmi"] = std::make_shared<instructions::DBcc>(nullCpu());
    commandMapping["dbge"] = std::make_shared<instructions::DBcc>(nullCpu());
    commandMapping["dblt"] = std::make_shared<instructions::DBcc>(nullCpu());
    commandMapping["dbgt"] = std::make_shared<instructions::DBcc>(nullCpu());
    commandMapping["dble"] = std::make_shared<instructions::DBcc>(nullCpu());

    commandMapping["trap"] = std::make_shared<instructions::TRAP>(nullCpu());
    commandMapping["reset"] = std::make_shared<instructions::RESET>(nullCpu());
    commandMapping["nop"] = std::make_shared<instructions::NOP>(nullCpu());
    commandMapping["stop"] = std::make_shared<instructions::STOP>(nullCpu());

    commandMapping["rte"] = std::make_shared<instructions::RTE>(nullCpu());
    commandMapping["rts"] = std::make_shared<instructions::RTS>(nullCpu());
    commandMapping["trapv"] = std::make_shared<instructions::TRAPV>(nullCpu());
    commandMapping["rtr"] = std::make_shared<instructions::RTR>(nullCpu());
}

std::set<Binary> Assembler::assemble(const std::vector<std::string>& source) {
    return std::set<Binary>();
}

int32_t Assembler::getPc() {
    return pc;
}

void Assembler::setPc(int32_t pc) {
    this->pc = pc;
}

std::optional<DisassembledInstruction> Assembler::parseLine(const std::string& line) {
    return parseLine(-1, line);
}

std::vector<std::string> Assembler::splitLine(const std::string& line) {
    std::vector<std::string> result;

    int32_t part = 0; // 1=mnem, 2=op1, 3=op2
    int32_t inPar = 0;
    std::string current;
    for (int32_t index = 0; index < static_cast<int32_t>(line.size()); index++) {
        char ch = line.at(static_cast<size_t>(index));

        switch (ch) {
            case '\t':
            case ' ':
                if (part == 1) {
                    result.push_back(current);
                    current.clear();
                    part = 2;
                }
                break;
            case '(':
                current += ch;
                inPar++;
                break;
            case ')':
                current += ch;
                inPar--;
                break;
            case ',':
                if (inPar == 0) {
                    result.push_back(current);
                    current.clear();
                    part++;
                } else {
                    current += ch;
                }
                break;
            default:
                if (part == 0) {
                    part = 1;
                }
                current += ch;
                break;
        }
    }

    result.push_back(current);

    return result;
}

std::optional<DisassembledInstruction> Assembler::parseLine(int32_t lineNumber, const std::string& line) {
    OperandParser operandParser;
    std::string lower = toLowerCase(trim(line));

    if (lower.empty() || lower.front() == ';') {
        // comment
        return DisassembledInstruction(pc, -1, line);
    }

    // Java: lower.endsWith(":")
    if (!lower.empty() && lower.back() == ':') {
        labels.addLabel(lower.substr(0, lower.size() - 1), pc);

        return DisassembledInstruction(pc, -1, line);
    }

    if (lower.find(" equ ") != std::string::npos) {
        // Java: String[] parts = lower.split("equ");
        std::vector<std::string> parts;
        size_t start = 0;
        size_t found = lower.find("equ");
        while (found != std::string::npos) {
            parts.push_back(lower.substr(start, found - start));
            start = found + 3;
            found = lower.find("equ", start);
        }
        parts.push_back(lower.substr(start));
        // String.split() drops trailing empty strings.
        while (!parts.empty() && parts.back().empty()) {
            parts.pop_back();
        }

        if (parts.size() != 2) {
            throw std::invalid_argument("Can't parse '" + line + "'");
        } else {
            labels.addLabel(trim(parts.at(0)), operandParser.parseValue(trim(parts.at(1))));
        }

        return DisassembledInstruction(pc, 0, line);
    }

    std::string mnemonic = lower;
    std::string command;
    std::string op1 = "";
    std::string op2 = "";
    Size size = Size::Unsized;
    int32_t numberOfParts = 0;
    // DisassembledOperand's fields are const, so the Java style "operand1 = ..."
    // reassignment below is a rebinding of these pointers.
    std::unique_ptr<AssembledOperand> operand1 = std::make_unique<AssembledOperand>("", 0, 0);
    std::unique_ptr<AssembledOperand> operand2 = std::make_unique<AssembledOperand>("", 0, 0);

    std::vector<std::string> parts = splitLine(lower);

    if (parts.size() > 0) {
        mnemonic = parts.at(0);
    }

    if (parts.size() == 2) {
        numberOfParts = 1;
        op1 = parts.at(1);
    } else if (parts.size() == 3) {
        numberOfParts = 2;
        op1 = parts.at(1);
        op2 = parts.at(2);
    }

    if (mnemonic.find('.') != std::string::npos) {
        command = mnemonic.substr(0, mnemonic.find('.'));
        char sz = mnemonic.at(mnemonic.find('.') + 1);

        switch (sz) {
            case 'b':
                size = Size::Byte;
                break;
            case 'w':
                size = Size::Word;
                break;
            case 'l':
                size = Size::Long;
                break;
        }
    } else {
        command = mnemonic;
    }

    if (commandMapping.find(command) == commandMapping.end()) {
        throw ParseException("Mnemonic '" + command + "' not found!", lineNumber);
    } else {
        std::shared_ptr<InstructionHandler> handler = commandMapping.find(command)->second;

        if (numberOfParts >= 1) {
            operand1 = std::make_unique<AssembledOperand>(
                operandParser.parse(labels, size, pc, lineNumber, op1));
        }
        if (numberOfParts >= 2) {
            operand2 = std::make_unique<AssembledOperand>(
                operandParser.parse(labels, size, pc, lineNumber, op2));
        }

        AssembledInstruction instruction(mnemonic, size, *operand1, *operand2);

        // TODO Assemble
        // DisassembledInstruction disassembled = handler.assemble(pc, instruction, labels);

        // TODO Assemble
        // for (Byte byt : disassembled.bytes()) {
        //     mmu.writeByte(pc++, byt);
        // }

        // TODO Assemble
        // return disassembled;
        return std::nullopt;
    }
}

void Assembler::printSymbols() {
    labels.printLabels(std::cout);
}

} // namespace m68k::cpu::assemble
