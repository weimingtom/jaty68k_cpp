#include "m68k/cpu/assemble/OperandParser.h"

#include <cctype>
#include <optional>
#include <string>

#include "m68k/cpu/assemble/ParseException.h"

namespace m68k::cpu::assemble {

namespace {

// Java String.trim(): strips leading and trailing characters <= ' '.
std::string trim(const std::string& value) {
    size_t begin = 0;
    size_t end = value.size();

    while (begin < end && static_cast<unsigned char>(value[begin]) <= ' ') {
        begin++;
    }
    while (end > begin && static_cast<unsigned char>(value[end - 1]) <= ' ') {
        end--;
    }

    return value.substr(begin, end - begin);
}

// Java String.toLowerCase(): one std::tolower call per character.
std::string toLowerCase(const std::string& value) {
    std::string result(value);

    for (char& ch : result) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    return result;
}

} // namespace

std::regex OperandParser::labelPattern("[a-zA-Z][a-zA-Z0-9_]+");

std::unordered_map<std::string, AddressingMode> OperandParser::modeMapping;

OperandParser::OperandParser() {
    // Java's instance initialiser block: it runs for every new OperandParser and
    // (re)fills the shared static modeMapping.
    modeMapping["v"] = AddressingMode::IMMEDIATE;
    modeMapping["vv"] = AddressingMode::IMMEDIATE;
    modeMapping["a"] = AddressingMode::IMMEDIATE_ADDRESS;
    modeMapping["d"] = AddressingMode::IMMEDIATE_DATA;
    modeMapping["i"] = AddressingMode::INDIRECT;
    modeMapping["-i"] = AddressingMode::INDIRECT_PRE;
    modeMapping["i+"] = AddressingMode::INDIRECT_POST;

    modeMapping["vw"] = AddressingMode::ABSOLUTE_NEAR;
    modeMapping["vl"] = AddressingMode::ABSOLUTE_FAR;

    modeMapping["va"] = AddressingMode::INDIRECT_DISP;
    modeMapping["vi"] = AddressingMode::INDIRECT_DISP;
    modeMapping["vad"] = AddressingMode::INDIRECT_INDEX;
    modeMapping["vid"] = AddressingMode::INDIRECT_INDEX;
    modeMapping["vadw"] = AddressingMode::INDIRECT_INDEX;
    modeMapping["vadl"] = AddressingMode::INDIRECT_INDEX;
    modeMapping["vaaw"] = AddressingMode::INDIRECT_INDEX;
    modeMapping["vaal"] = AddressingMode::INDIRECT_INDEX;
    modeMapping["vidw"] = AddressingMode::INDIRECT_INDEX;
    modeMapping["vidl"] = AddressingMode::INDIRECT_INDEX;
    modeMapping["viaw"] = AddressingMode::INDIRECT_INDEX;
    modeMapping["vial"] = AddressingMode::INDIRECT_INDEX;
    modeMapping["vp"] = AddressingMode::PC_DISP;
    modeMapping["vpd"] = AddressingMode::PC_INDEX;
    modeMapping["vpa"] = AddressingMode::PC_INDEX;
    modeMapping["vpdw"] = AddressingMode::PC_INDEX;
    modeMapping["vpdl"] = AddressingMode::PC_INDEX;
    modeMapping["vpaw"] = AddressingMode::PC_INDEX;
    modeMapping["vpal"] = AddressingMode::PC_INDEX;

    modeMapping["s"] = AddressingMode::SR;
    modeMapping["c"] = AddressingMode::CCR;
}

void OperandParser::clear() {
    parts.clear();
    current = Part();
}

void OperandParser::newPart() {
    if (current.type != ' ') {
        parts.push_back(current);
    }

    current = Part();
}

void OperandParser::setType(char t) {
    if (current.type == ' ') {
        current.type = t;
    }
}

AssembledOperand OperandParser::parse(Labels& labels, Size size, int32_t pc, int32_t lineNumber,
                                      const std::string& operand) {
    clear();
    const std::string lower = toLowerCase(trim(operand));
    char last = ' ';

    const int32_t length = static_cast<int32_t>(lower.length());

    for (int32_t index = 0; index < length; index++) {
        const char ch = lower.at(static_cast<size_t>(index));
        char next = ' ';

        if (index < length - 1) {
            next = lower.at(static_cast<size_t>(index + 1));
        }

        switch (ch) {
            case ',':
            case '.':
            case '(':
            case ')':
            case '/':
                newPart();
                break;
            case '+':
                if (last == ')') {
                    setType(ch);
                }
                newPart();
                current.text += ch;
                break;
            case '-':
                if (next == '(') {
                    setType(ch);
                }
                newPart();
                current.text += ch;
                break;
            case 'a':
                if (last == '(') {
                    setType('i');
                } else {
                    setType(ch);
                }
                current.text += ch;
                break;
            case 'd':
            case 'p':
            case 'c':
            case 's':
            case 'w':
            case 'l':
                setType(ch);
                current.text += ch;
                break;
            case '#':
                setType('v');
                break;
            case '%':
            case '$':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                setType('v');
                current.text += ch;
                break;
            case 'r':
                current.text += ch;
                break;
            default:
                // The Java switch has no default case: every other character is
                // deliberately ignored, so nothing is thrown here.
                break;
        }

        if (ch != ' ') {
            last = ch;
        }
    }
    newPart();

    // Java: AddressingMode mode = modeMapping.get(descriptor()); the map returns
    // null for an unknown descriptor, which std::optional models here.
    std::optional<AddressingMode> mode;
    const auto modeEntry = modeMapping.find(descriptor());
    if (modeEntry != modeMapping.end()) {
        mode = modeEntry->second;
    }

    int32_t bytes = 0;
    int32_t memory_read = 0;
    Conditional conditional = Conditional::NA;
    int32_t register_ = 0;
    int32_t ext_reg = 0;
    int32_t ext_data = 0;
    Size ext_size = Size::Unsized;

    if (!mode.has_value()) {
        if (parts.empty()) {
            if (couldBeLabel(operand)) {
                mode = AddressingMode::LABEL;
            } else {
                throw ParseException("Unable to parse expression '" + operand + "'", lineNumber);
            }
        } else {
            bool registerList = true;

            for (const Part& part : parts) {
                if (part.type != 'd' && part.type != 'a') {
                    registerList = false;
                }
            }

            if (registerList) {
                mode = AddressingMode::REGISTER_LIST;
            } else {
                if (couldBeLabel(operand)) {
                    mode = AddressingMode::LABEL;

                    if (labels.hasLabel(operand)) {
                        memory_read = labels.getLabel(operand, 0, false, size);

                        if (memory_read == -1) {
                            throw ParseException("Unkown label " + operand, lineNumber);
                        }
                    } else {
                        memory_read = 0;
                    }
                } else {
                    throw ParseException("Unable to parse expression '" + operand + "'", lineNumber);
                }
            }
        }
    }

    switch (mode.value()) {
        case AddressingMode::IMMEDIATE: {
            memory_read = parseValue(parts.at(parts.size() - 1).text);
            register_ = 4;
            if (size == Size::Long) {
                bytes = 4;
            } else {
                bytes = 2;
            }
            break;
        }
        case AddressingMode::IMMEDIATE_DATA:
        case AddressingMode::IMMEDIATE_ADDRESS:
        case AddressingMode::INDIRECT:
        case AddressingMode::INDIRECT_POST: {
            register_ = std::stoi(parts.at(0).text.substr(1));
            break;
        }
        case AddressingMode::INDIRECT_PRE: {
            register_ = std::stoi(parts.at(1).text.substr(1));
            break;
        }
        case AddressingMode::ABSOLUTE_NEAR: {
            memory_read = parseValue(parts.at(0).text);
            bytes = 2;
            break;
        }
        case AddressingMode::ABSOLUTE_FAR: {
            memory_read = parseValue(parts.at(0).text);
            bytes = 4;
            register_ = 1;
            break;
        }
        case AddressingMode::INDIRECT_DISP: {
            memory_read = parseValue(parts.at(0).text);
            register_ = std::stoi(parts.at(1).text.substr(1));
            bytes = 2;
            break;
        }
        case AddressingMode::INDIRECT_INDEX: {
            ext_data = parseValue(parts.at(0).text);
            ext_reg = std::stoi(parts.at(2).text.substr(1));
            register_ = std::stoi(parts.at(1).text.substr(1));
            if (parts.size() > 3) {
                ext_size = parts.at(3).type == 'w' ? Size::Word : Size::Long;
            } else {
                ext_size = Size::Word;
            }
            break;
        }
        case AddressingMode::PC_DISP: {
            memory_read = (parseValue(parts.at(0).text) - pc - 2) & 0xffff;
            register_ = 2;
            bytes = 2;
            break;
        }
        case AddressingMode::PC_INDEX: {
            ext_data = parseValue(parts.at(0).text);
            ext_reg = std::stoi(parts.at(2).text.substr(1));
            register_ = 3;
            if (parts.size() > 3) {
                ext_size = parts.at(3).type == 'w' ? Size::Word : Size::Long;
            } else {
                ext_size = Size::Word;
            }
            break;
        }
        case AddressingMode::REGISTER_LIST: {
            int32_t lastReg = -1;
            char lastType = ' ';
            for (const Part& part : parts) {
                int32_t reg = -1;
                const bool dataReg = part.type == 'd';

                if (part.text.at(0) == '-') {
                    // lastReg to reg
                    reg = std::stoi(std::string(1, part.text.at(2)));

                    if (lastType == 'd' && !dataReg) {
                        for (; lastReg <= 7; lastReg++) {
                            const int32_t orMask = 1 << (lastReg & 31);
                            register_ |= orMask;
                        }
                        lastReg = 0;
                        for (; lastReg <= reg; lastReg++) {
                            const int32_t orMask = 1 << (lastReg & 31) << 8;
                            register_ |= orMask;
                        }
                    } else {
                        for (; lastReg <= reg; lastReg++) {
                            int32_t orMask = 1 << (lastReg & 31);
                            if (!dataReg) {
                                orMask = orMask << 8;
                            }
                            register_ |= orMask;
                        }
                    }
                } else {
                    reg = std::stoi(std::string(1, part.text.at(1)));

                    int32_t orMask = 1 << (reg & 31);
                    if (!dataReg) {
                        orMask = orMask << 8;
                    }

                    register_ |= orMask;
                }

                lastReg = reg;
                lastType = part.type;
            }
            break;
        }
        case AddressingMode::LABEL: {
            mode = AddressingMode::IMMEDIATE;
            if (size == Size::Long) {
                bytes = 4;
            } else {
                bytes = 2;
            }
            register_ = 4;
            break;
        }
        // The Java switch has no case for these enumerators, so in Java nothing
        // happens for them; they are spelled out here so the C++ switch stays
        // exhaustive without inventing a default that throws.
        case AddressingMode::NA:
        case AddressingMode::SR:
        case AddressingMode::CCR: {
            break;
        }
    }

    if (mode == AddressingMode::PC_INDEX) {
        // relative to pc
        memory_read = ((ext_data - pc - 2) & 0xff) | (ext_reg << 12);
        if (ext_size == Size::Long) {
            memory_read |= 0x800;
        }
        bytes = 2;
    } else if (mode == AddressingMode::INDIRECT_INDEX) {
        memory_read = (ext_data & 0xff) | (ext_reg << 12);
        if (ext_size == Size::Long) {
            memory_read |= 0x800;
        }
        bytes = 2;
    }

    return AssembledOperand(lower, bytes, memory_read, mode.value(), conditional, register_);
}

bool OperandParser::couldBeLabel(const std::string& value) {
    const int32_t length = static_cast<int32_t>(value.length());

    for (int32_t index = 0; index < length; index++) {
        const char ch = value.at(static_cast<size_t>(index));

        if (!(ch >= 'a' && ch <= 'z') && index == 0) {
            return false;
        }
        if (!(ch >= 'a' && ch <= 'z') && !(ch >= '0' && ch <= '9') && !(ch == '_')) {
            return false;
        }
    }

    return true;
}

int32_t OperandParser::parseValue(const std::string& value) {
    if (value.at(0) == '$') {
        return std::stoi(value.substr(1), nullptr, 16);
    } else if (value.at(0) == '%') {
        return std::stoi(value.substr(1), nullptr, 2);
    } else {
        return std::stoi(value);
    }
}

std::string OperandParser::descriptor() {
    std::string result;

    for (const Part& part : parts) {
        result += part.type;
    }

    return result;
}

std::vector<OperandParser::Part> OperandParser::getParts() {
    return parts;
}

} // namespace m68k::cpu::assemble
