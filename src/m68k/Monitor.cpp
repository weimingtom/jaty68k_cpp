#include "m68k/Monitor.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "m68k/Console.h"
#include "m68k/cpu/DisassembledInstruction.h"
#include "m68k/cpu/Instruction.h"
#include "m68k/cpu/MC68000.h"
#include "m68k/memory/KatyMemorySpace.h"
#include "m68k/util/Format.h"

namespace m68k {

namespace {

/*
 * Ports of the java.lang.String methods the monitor calls, so that the statement
 * order of the Java source can be kept line for line.
 */

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

/// Java String.toLowerCase() - per character, as in the rest of the port.
std::string toLowerCase(const std::string& value) {
    std::string result = value;

    for (char& ch : result) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    return result;
}

/// Java String.startsWith(String).
bool startsWith(const std::string& value, const std::string& prefix) {
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

/// Java String.equalsIgnoreCase(String).
bool equalsIgnoreCase(const std::string& value, const std::string& other) {
    return toLowerCase(value) == toLowerCase(other);
}

/// Java String.split(" "): splits at every single space, discarding trailing empty strings.
std::vector<std::string> splitOnSpace(const std::string& line) {
    std::vector<std::string> tokens;

    size_t found = line.find(' ');
    if (found == std::string::npos) {
        // Java returns the input itself when the expression does not match.
        tokens.push_back(line);
        return tokens;
    }

    size_t start = 0;
    while (found != std::string::npos) {
        tokens.push_back(line.substr(start, found - start));
        start = found + 1;
        found = line.find(' ', start);
    }
    tokens.push_back(line.substr(start));

    // String.split() drops trailing empty strings.
    while (!tokens.empty() && tokens.back().empty()) {
        tokens.pop_back();
    }

    return tokens;
}

} // namespace

int monitorMain(int argc, char** argv) {
    // Java: public static void main(String[] args) throws Exception
    // The original ignored the command line it was handed and always started
    // main_() with this single hard-coded argument; argc/argv are kept only
    // because they are the C++ entry point's arguments.  An exception escaping
    // here terminates the program, just as an uncaught exception escaping the
    // Java main() did.
    (void)argc;
    (void)argv;

    Monitor::main_(std::vector<std::string>{"linux-pcb.bin"}); //Once the zBug(ROM) loads simply type "j" and then "003000" to start Katy 68k
    return 0;
}

Monitor::Monitor(m68k::cpu::Cpu& cpu, m68k::memory::AddressSpace& memory)
    : cpu(cpu), memory(memory) {
    buffer.reserve(128); // Java: buffer = new StringBuilder(128);
    showBytes = false;
    autoRegs = false;
    breakpoints = std::vector<int32_t>(); // Java: breakpoints = new ArrayList<Integer>();
}

void Monitor::main_(const std::vector<std::string>& args) {
    int32_t mem_size = 0xfffff; // 512kb of memory default

    std::cout << "m68k Monitor v0.1 - Copyright 2008-2010 Tony Headford\n";

    // Java let the garbage collector reclaim these; the monitor runs until the
    // process exits, so they are deliberately not deleted here either.
    m68k::memory::AddressSpace* memory = new m68k::memory::KatyMemorySpace(mem_size);

    m68k::cpu::Cpu* cpu = new m68k::cpu::MC68000();
    cpu->setAddressSpace(memory);
    cpu->reset(); //init cpu

    // Java: new Console(cpu, memory); the Java constructor installed the key
    // dispatcher itself, so nothing else was called on the object.
    Console console(*cpu, *memory);

    Monitor monitor(*cpu, *memory);
    if (args.size() == 1) {
        monitor.handleLoad(std::vector<std::string>{"load", "0", args.at(0)});
    }
    monitor.run();
    monitor.handleGo(std::vector<std::string>());
}

void Monitor::run() {
    writer = &std::cout; // Java: writer = new PrintWriter(System.out);
    reader = &std::cin;  // Java: reader = new BufferedReader(new InputStreamReader(System.in));

    running = true;
}

void Monitor::handleCommand(const std::string& line) {
    std::vector<std::string> tokens = splitOnSpace(line); // Java: String[] tokens = line.split(" ");
    std::string cmd = toLowerCase(trim(tokens.at(0)));

    if (cmd.size() > 0) {
        if (cmd == "q") {
            running = false;
        } else if (cmd == "r") {
            dumpInfo();
        } else if (cmd == "pc") {
            handlePC(tokens);
        } else if (cmd == "d") {
            handleDisassemble(tokens);
        } else if (cmd == "b") {
            handleBreakPoints(tokens);
        } else if (cmd == "sr") {
            handleSR(tokens);
        } else if (cmd == "ccr") {
            handleCCR(tokens);
        } else if (cmd == "usp") {
            handleUSP(tokens);
        } else if (cmd == "ssp") {
            handleSSP(tokens);
        } else if (cmd == "ml") {
            handleMemLong(tokens);
        } else if (cmd == "mw") {
            handleMemWord(tokens);
        } else if (cmd == "mb") {
            handleMemByte(tokens);
        } else if (cmd == "m") {
            handleMemDump(tokens);
        } else if (cmd == "s") {
            handleStep(tokens);
        } else if (cmd == "g") {
            handleGo(tokens);
        } else if (cmd == "autoregs") {
            handleAutoRegs(tokens);
        } else if (cmd == "showbytes") {
            handleShowBytes(tokens);
        } else if (cmd == "load") {
            handleLoad(tokens);
        } else if (startsWith(cmd, "d")) {
            handleDataRegs(tokens);
        } else if (startsWith(cmd, "a")) {
            handleAddrRegs(tokens);
        } else if (cmd == "?" || cmd == "h" || cmd == "help") {
            showHelp(tokens);
        } else {
            *writer << "Unknown command: " << tokens.at(0) << '\n';
        }
    }
}

void Monitor::handleGo(const std::vector<std::string>& tokens) {
    int32_t timer = 0;
    int32_t count = 0;
    bool going = true;

    while (running && going) {
        try {
            if (timer++ == 10000) {
                timer = 0;
                cpu.raiseInterrupt(5);
            }
            cpu.executeSafeInterrupt();

            int32_t time = cpu.execute();
            count += time;
            int32_t addr = cpu.getPC();
            if (hasBreakpoints &&
                std::find(breakpoints.begin(), breakpoints.end(), addr) != breakpoints.end()) {
                //time to stop
                *writer << "BREAKPOINT" << '\n';
                going = false;
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << '\n'; // Java: e.printStackTrace();
            going = false;
        }
    }
    *writer << m68k::util::format("[Consumed %d ticks]\n", count);
}

void Monitor::handleBreakPoints(const std::vector<std::string>& tokens) {
    if (tokens.size() > 1) {
        // add or remove toggle
        try {
            int32_t addr = parseInt(tokens.at(1));
            // Java: breakpoints.contains(addr) followed by
            // breakpoints.remove(new Integer(addr)), i.e. remove the first match.
            std::vector<int32_t>::iterator found =
                std::find(breakpoints.begin(), breakpoints.end(), addr);
            if (found != breakpoints.end()) {
                breakpoints.erase(found);
                if (breakpoints.size() == 0) {
                    hasBreakpoints = false;
                }
            } else {
                hasBreakpoints = true;
                breakpoints.push_back(addr); // Java: breakpoints.add(addr);
            }
        } catch (const std::exception&) { // NumberFormatException
            return;
        }
    }

    //list breakpoints
    *writer << "Breakpoints:" << '\n';
    for (int32_t bp : breakpoints) {
        *writer << m68k::util::format("$%x", static_cast<uint32_t>(bp)) << '\n';
    }
}

void Monitor::handleAutoRegs(const std::vector<std::string>& tokens) {
    if (tokens.size() > 1) {
        if (equalsIgnoreCase(tokens.at(1), "on")) {
            autoRegs = true;
        } else if (equalsIgnoreCase(tokens.at(1), "off")) {
            autoRegs = false;
        }
    }

    *writer << "autoregs is " << (autoRegs ? "on" : "off") << '\n';
}

void Monitor::handleShowBytes(const std::vector<std::string>& tokens) {
    if (tokens.size() > 1) {
        if (equalsIgnoreCase(tokens.at(1), "on")) {
            showBytes = true;
        } else if (equalsIgnoreCase(tokens.at(1), "off")) {
            showBytes = false;
        }
    }

    *writer << "showbytes is " << (showBytes ? "on" : "off") << '\n';
}

void Monitor::handleDataRegs(const std::vector<std::string>& tokens) {
    std::string reg = trim(tokens.at(0));
    if (reg.size() != 2) {
        *writer << "Bad identifier [" << reg << "]" << '\n';
        return;
    }
    int32_t r = reg.at(1) - '0';
    if (r < 0 || r > 7) {
        *writer << "Bad identifier [" << reg << "]" << '\n';
        return;
    }

    if (tokens.size() == 2) {
        //set a value
        int32_t value;
        try {
            value = parseInt(tokens.at(1));
        } catch (const std::exception&) { // NumberFormatException
            *writer << "Bad value [" << tokens.at(1) << "]" << '\n';
            return;
        }
        cpu.setDataRegisterLong(r, value);
    } else {
        *writer << m68k::util::format("D%d: %08x\n", r,
                                      static_cast<uint32_t>(cpu.getDataRegisterLong(r)));
    }
}

void Monitor::handleAddrRegs(const std::vector<std::string>& tokens) {
    std::string reg = trim(tokens.at(0));
    if (reg.size() != 2) {
        *writer << "Bad identifier [" << reg << "]" << '\n';
        return;
    }
    int32_t r = reg.at(1) - '0';
    if (r < 0 || r > 7) {
        *writer << "Bad identifier [" << reg << "]" << '\n';
        return;
    }

    if (tokens.size() == 2) {
        //set a value
        int32_t value;
        try {
            value = parseInt(tokens.at(1));
        } catch (const std::exception&) { // NumberFormatException
            *writer << "Bad value [" << tokens.at(1) << "]" << '\n';
            return;
        }
        cpu.setAddrRegisterLong(r, value);
    } else {
        *writer << m68k::util::format("A%d: %08x\n", r,
                                      static_cast<uint32_t>(cpu.getAddrRegisterLong(r)));
    }
}

void Monitor::handleStep(const std::vector<std::string>& tokens) {
    int32_t time = cpu.execute();
    *writer << m68k::util::format("[Execute took %d ticks]\n", time);
}

void Monitor::handleMemDump(const std::vector<std::string>& tokens) {
    if (tokens.size() != 2) {
        *writer << "usage: m <start address>" << '\n';
        return;
    }
    std::string address = tokens.at(1);
    int32_t size = memory.size();
    try {
        int32_t addr = parseInt(address);
        if (addr < 0 || addr >= size) {
            *writer << "Address out of range" << '\n';
            return;
        }
        std::string sb;
        sb.reserve(80); // Java: StringBuilder sb = new StringBuilder(80);
        std::string asc;
        asc.reserve(16); // Java: StringBuilder asc = new StringBuilder(16);

        for (int32_t y = 0; y < 8 && addr < size; y++) {
            sb += m68k::util::format("%08x", static_cast<uint32_t>(addr));
            sb += "  ";
            for (int32_t x = 0; x < 16 && addr < size; x++) {
                int32_t b = cpu.readMemoryByte(addr);
                sb += m68k::util::format("%02x ", static_cast<uint32_t>(b));
                asc += getPrintable(b);
                addr++;
            }
            if (sb.size() < 48) {
                for (int32_t n = static_cast<int32_t>(sb.size()); n < 48; n++) {
                    sb += " ";
                }
            }
            sb += "    ";
            sb += asc;
            *writer << sb << '\n';
            sb.clear(); // Java: sb.delete(0, sb.length());
            asc.clear(); // Java: asc.delete(0, asc.length());
        }
    } catch (const std::exception&) { // NumberFormatException
        *writer << "Unknown address [" << address << "]" << '\n';
    }
}

void Monitor::handleDisassemble(const std::vector<std::string>& tokens) {
    int32_t start;
    int32_t num_instructions = 8;

    if (tokens.size() > 2) {
        try {
            num_instructions = parseInt(tokens.at(2));
        } catch (const std::exception&) { // NumberFormatException
            *writer << "Invalid instruction count: " << tokens.at(2) << '\n';
            return;
        }
    }

    if (tokens.size() > 1) {
        std::string address = tokens.at(1);
        try {
            start = parseInt(address);
        } catch (const std::exception&) { // NumberFormatException
            *writer << "Unknown address [" << address << "]" << '\n';
            return;
        }
    } else {
        start = cpu.getPC();
    }

    int32_t count = 0;
    std::string buffer;
    buffer.reserve(80); // Java: StringBuilder buffer = new StringBuilder(80);

    while (start < memory.size() && count < num_instructions) {
        buffer.clear(); // Java: buffer.delete(0, buffer.length());
        int32_t opcode = cpu.readMemoryWord(start);

        m68k::cpu::Instruction* i = cpu.getInstructionFor(opcode);
        m68k::cpu::DisassembledInstruction di = i->disassemble(start, opcode);
        if (showBytes) {
            di.formatInstruction(buffer);
        } else {
            di.shortFormat(buffer);
        }
        *writer << buffer << '\n';
        start += di.size();
        count++;
    }
}

void Monitor::dumpInfo() {
    *writer << '\n';
    *writer << m68k::util::format("D0: %08x   D4: %08x   A0: %08x   A4: %08x     PC:  %08x\n",
                                  static_cast<uint32_t>(cpu.getDataRegisterLong(0)),
                                  static_cast<uint32_t>(cpu.getDataRegisterLong(4)),
                                  static_cast<uint32_t>(cpu.getAddrRegisterLong(0)),
                                  static_cast<uint32_t>(cpu.getAddrRegisterLong(4)),
                                  static_cast<uint32_t>(cpu.getPC()));
    *writer << m68k::util::format("D1: %08x   D5: %08x   A1: %08x   A5: %08x     SR:  %04x %s\n",
                                  static_cast<uint32_t>(cpu.getDataRegisterLong(1)),
                                  static_cast<uint32_t>(cpu.getDataRegisterLong(5)),
                                  static_cast<uint32_t>(cpu.getAddrRegisterLong(1)),
                                  static_cast<uint32_t>(cpu.getAddrRegisterLong(5)),
                                  static_cast<uint32_t>(cpu.getSR()), makeFlagView());

    *writer << m68k::util::format("D2: %08x   D6: %08x   A2: %08x   A6: %08x     USP: %08x\n",
                                  static_cast<uint32_t>(cpu.getDataRegisterLong(2)),
                                  static_cast<uint32_t>(cpu.getDataRegisterLong(6)),
                                  static_cast<uint32_t>(cpu.getAddrRegisterLong(2)),
                                  static_cast<uint32_t>(cpu.getAddrRegisterLong(6)),
                                  static_cast<uint32_t>(cpu.getUSP()));

    *writer << m68k::util::format("D3: %08x   D7: %08x   A3: %08x   A7: %08x     SSP: %08x\n\n",
                                  static_cast<uint32_t>(cpu.getDataRegisterLong(3)),
                                  static_cast<uint32_t>(cpu.getDataRegisterLong(7)),
                                  static_cast<uint32_t>(cpu.getAddrRegisterLong(3)),
                                  static_cast<uint32_t>(cpu.getAddrRegisterLong(7)),
                                  static_cast<uint32_t>(cpu.getSSP()));

    buffer.clear(); // Java: buffer.delete(0, buffer.length());
    int32_t addr = cpu.getPC();
    if (addr < 0 || addr >= memory.size()) {
        buffer += m68k::util::format("%08x   ????", static_cast<uint32_t>(addr));
    } else {
        int32_t opcode = cpu.readMemoryWord(addr);

        m68k::cpu::Instruction* i = cpu.getInstructionFor(opcode);
        m68k::cpu::DisassembledInstruction di = i->disassemble(addr, opcode);
        if (showBytes) {
            di.formatInstruction(buffer);
        } else {
            di.shortFormat(buffer);
        }
    }

    *writer << m68k::util::format("==> %s\n\n", buffer);
}

std::string Monitor::makeFlagView() {
    std::string sb;
    sb.reserve(5); // Java: StringBuilder sb = new StringBuilder(5);
    sb += (cpu.isFlagSet(m68k::cpu::X_FLAG) ? 'X' : '-');
    sb += (cpu.isFlagSet(m68k::cpu::N_FLAG) ? 'N' : '-');
    sb += (cpu.isFlagSet(m68k::cpu::Z_FLAG) ? 'Z' : '-');
    sb += (cpu.isFlagSet(m68k::cpu::V_FLAG) ? 'V' : '-');
    sb += (cpu.isFlagSet(m68k::cpu::C_FLAG) ? 'C' : '-');
    return sb;
}

char Monitor::getPrintable(int32_t val) {
    if (val < ' ' || val > '~') {
        return '.';
    }

    return static_cast<char>(val);
}

int32_t Monitor::parseInt(const std::string& value) {
    int32_t v;

    if (startsWith(value, "$")) {
        try {
            v = static_cast<int32_t>(std::stoll(value.substr(1), nullptr, 16) & 0x0ffffffffL);
        } catch (const std::exception&) { // NumberFormatException
            *writer << "Not a valid hex number [" << value << "]" << '\n';
            throw;
        }
    } else {
        try {
            v = static_cast<int32_t>(std::stoll(value) & 0x0ffffffffL);
        } catch (const std::exception&) { // NumberFormatException
            *writer << "Not a valid decimal number [" << value << "]" << '\n';
            throw;
        }
    }

    return v;
}

void Monitor::handlePC(const std::vector<std::string>& tokens) {
    if (tokens.size() == 1) {
        *writer << m68k::util::format("PC: %08x\n", static_cast<uint32_t>(cpu.getPC()));
    } else if (tokens.size() == 2) {
        //set a value
        int32_t value;
        try {
            value = parseInt(tokens.at(1));
        } catch (const std::exception&) { // NumberFormatException
            *writer << "Bad value [" << tokens.at(1) << "]" << '\n';
            return;
        }
        cpu.setPC(value);
    } else {
        *writer << "usage: " << tokens.at(0) << " [value]" << '\n';
    }
}

void Monitor::handleSR(const std::vector<std::string>& tokens) {
    if (tokens.size() == 1) {
        *writer << m68k::util::format("SR: %04x\n", static_cast<uint32_t>(cpu.getSR()));
    } else if (tokens.size() == 2) {
        //set a value
        int32_t value;
        try {
            value = parseInt(tokens.at(1));
        } catch (const std::exception&) { // NumberFormatException
            *writer << "Bad value [" << tokens.at(1) << "]" << '\n';
            return;
        }
        cpu.setSR(value);
    } else {
        *writer << "usage: " << tokens.at(0) << " [value]" << '\n';
    }
}

void Monitor::handleCCR(const std::vector<std::string>& tokens) {
    if (tokens.size() == 1) {
        *writer << m68k::util::format("CCR: %02x  %s\n",
                                      static_cast<uint32_t>(cpu.getCCRegister()),
                                      makeFlagView());
    } else if (tokens.size() == 2) {
        //set a value
        int32_t value;
        try {
            value = parseInt(tokens.at(1));
        } catch (const std::exception&) { // NumberFormatException
            *writer << "Bad value [" << tokens.at(1) << "]" << '\n';
            return;
        }
        cpu.setCCRegister(value);
    } else {
        *writer << "usage: " << tokens.at(0) << " [value]" << '\n';
    }
}

void Monitor::handleUSP(const std::vector<std::string>& tokens) {
    if (tokens.size() == 1) {
        *writer << m68k::util::format("USP: %08x\n", static_cast<uint32_t>(cpu.getUSP()));
    } else if (tokens.size() == 2) {
        //set a value
        int32_t value;
        try {
            value = parseInt(tokens.at(1));
        } catch (const std::exception&) { // NumberFormatException
            *writer << "Bad value [" << tokens.at(1) << "]" << '\n';
            return;
        }
        cpu.setUSP(value);
    } else {
        *writer << "usage: " << tokens.at(0) << " [value]" << '\n';
    }
}

void Monitor::handleSSP(const std::vector<std::string>& tokens) {
    if (tokens.size() == 1) {
        *writer << m68k::util::format("SSP: %08x\n", static_cast<uint32_t>(cpu.getSSP()));
    } else if (tokens.size() == 2) {
        //set a value
        int32_t value;
        try {
            value = parseInt(tokens.at(1));
        } catch (const std::exception&) { // NumberFormatException
            *writer << "Bad value [" << tokens.at(1) << "]" << '\n';
            return;
        }
        cpu.setSSP(value);
    } else {
        *writer << "usage: " << tokens.at(0) << " [value]" << '\n';
    }
}

void Monitor::handleMemLong(const std::vector<std::string>& tokens) {
    if (tokens.size() != 2 && tokens.size() != 3) {
        *writer << "usage: ml <address> [value]" << '\n';
    } else {
        std::string address = tokens.at(1);

        if (tokens.size() == 2) {
            // just read address
            try {
                int32_t addr = parseInt(address);
                if (addr < 0 || addr >= memory.size()) {
                    *writer << "Address out of range" << '\n';
                    return;
                }
                *writer << m68k::util::format("%08x  %08x\n", static_cast<uint32_t>(addr),
                                              static_cast<uint32_t>(cpu.readMemoryLong(addr)));
            } catch (const std::exception& e) {
                std::cerr << e.what() << '\n'; // Java: e.printStackTrace();
            }
        } else {
            //write address
            std::string value = tokens.at(2);
            try {
                int32_t addr = parseInt(address);
                if (addr < 0 || addr >= memory.size()) {
                    *writer << "Address out of range" << '\n';
                    return;
                }
                int32_t v = parseInt(value);
                cpu.writeMemoryLong(addr, v);
            } catch (const std::exception& e) {
                std::cerr << e.what() << '\n'; // Java: e.printStackTrace();
            }
        }
    }
}

void Monitor::handleMemWord(const std::vector<std::string>& tokens) {
    if (tokens.size() != 2 && tokens.size() != 3) {
        *writer << "usage: mw <address> [value]" << '\n';
    } else {
        std::string address = tokens.at(1);

        if (tokens.size() == 2) {
            // just read address
            try {
                int32_t addr = parseInt(address);
                if (addr < 0 || addr >= memory.size()) {
                    *writer << "Address out of range" << '\n';
                    return;
                }
                *writer << m68k::util::format("%08x  %04x\n", static_cast<uint32_t>(addr),
                                              static_cast<uint32_t>(cpu.readMemoryWord(addr)));
            } catch (const std::exception& e) {
                std::cerr << e.what() << '\n'; // Java: e.printStackTrace();
            }
        } else {
            //write address
            std::string value = tokens.at(2);
            try {
                int32_t addr = parseInt(address);
                if (addr < 0 || addr >= memory.size()) {
                    *writer << "Address out of range" << '\n';
                    return;
                }
                int32_t v = parseInt(value);
                cpu.writeMemoryWord(addr, v);
            } catch (const std::exception& e) {
                std::cerr << e.what() << '\n'; // Java: e.printStackTrace();
            }
        }
    }
}

void Monitor::handleMemByte(const std::vector<std::string>& tokens) {
    if (tokens.size() != 2 && tokens.size() != 3) {
        *writer << "usage: mb <address> [value]" << '\n';
    } else {
        std::string address = tokens.at(1);

        if (tokens.size() == 2) {
            // just read address
            try {
                int32_t addr = parseInt(address);
                if (addr < 0 || addr >= memory.size()) {
                    *writer << "Address out of range" << '\n';
                } else {
                    *writer << m68k::util::format("%08x  %02x\n", static_cast<uint32_t>(addr),
                                                  static_cast<uint32_t>(cpu.readMemoryByte(addr)));
                }
            } catch (const std::exception& e) {
                std::cerr << e.what() << '\n'; // Java: e.printStackTrace();
            }
        } else {
            //write address
            std::string value = tokens.at(2);
            try {
                int32_t addr = parseInt(address);
                if (addr < 0 || addr >= memory.size()) {
                    *writer << "Address out of range" << '\n';
                    return;
                }
                int32_t v = parseInt(value);
                cpu.writeMemoryByte(addr, v);
            } catch (const std::exception& e) {
                std::cerr << e.what() << '\n'; // Java: e.printStackTrace();
            }
        }
    }
}

void Monitor::handleLoad(const std::vector<std::string>& tokens) {
    // load address file
    if (tokens.size() != 3) {
        *writer << "usage: load <address> <file>" << '\n';
        return;
    }

    int32_t address;
    try {
        address = parseInt(tokens.at(1));
    } catch (const std::exception&) { // NumberFormatException
        *writer << "Invalid address specified [" << tokens.at(1) << "]" << '\n';
        return;
    }

    // Java: File f = new File(tokens[2]); if(!f.exists())
    std::ifstream f(tokens.at(2), std::ios::binary);
    if (!f.is_open()) {
        *writer << "Cannot find file [" << tokens.at(2) << "]" << '\n';
        return;
    }

    // Java: (int)f.length()
    f.seekg(0, std::ios::end);
    const int32_t length = static_cast<int32_t>(f.tellg());
    f.seekg(0, std::ios::beg);

    if (address + length >= memory.size()) {
        //need larger memory
        *writer << "Need larger memory to load this file at " << tokens.at(1) << '\n';
        return;
    }
    try {
        // Java: FileInputStream fis / byte[] buffer / fis.read(buffer) / fis.close()
        std::vector<uint8_t> buffer(static_cast<size_t>(length));
        f.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(length));
        int32_t len = static_cast<int32_t>(f.gcount());
        f.close();
        for (int32_t n = 0; n < len; n++) {
            memory.writeByte(address, buffer.at(static_cast<size_t>(n)));
            address++;
        }
        cpu.reset();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n'; // Java: e.printStackTrace();
    }
}

void Monitor::showHelp(const std::vector<std::string>& tokens) {
    *writer << "Command Help:" << '\n';
    *writer << "Addresses and values can be specified in hexadecimal by preceeding the value with '$'" << '\n';
    *writer << "      eg. d0 $deadbeef  - Set register d0 to 0xDEADBEEF" << '\n';
    *writer << "          m $10         - Memory dump starting at 0x10 (16 in decimal)" << '\n';
    *writer << "          pc 10         - Set the PC register to 10 (0x0A in hexadecimal)" << '\n';
    *writer << "General:" << '\n';
    *writer << "  ?,h,help              - Show this help." << '\n';
    *writer << "  q                     - Quit." << '\n';
    *writer << "Registers:" << '\n';
    *writer << "  r                     - Display all registers" << '\n';
    *writer << "  d[0-9] [value]        - Set or view a data register" << '\n';
    *writer << "  a[0-9] [value]        - Set or view an address register" << '\n';
    *writer << "  pc [value]            - Set or view the PC register" << '\n';
    *writer << "  sr [value]            - Set or view the SR register" << '\n';
    *writer << "  ccr [value]           - Set or view the CCR register" << '\n';
    *writer << "  usp [value]           - Set or view the USP register" << '\n';
    *writer << "  ssp [value]           - Set or view the SSP register" << '\n';
    *writer << "Memory:" << '\n';
    *writer << "  m <address>           - View (128 byte) memory dump starting at the specified address" << '\n';
    *writer << "  mb <address> [value]  - Set or view a byte (8-bit) value at the specified address" << '\n';
    *writer << "  mw <address> [value]  - Set or view a word (16-bit) value at the specified address" << '\n';
    *writer << "  ml <address> [value]  - Set or view a long (32-bit) value at the specified address" << '\n';
    *writer << "  load <address> <file> - Load <file> into memory starting at <address>" << '\n';
    *writer << "Execution & Disassembly:" << '\n';
    *writer << "  s                     - Execute the instruction at the PC register" << '\n';
    *writer << "  d <address> [count]   - Disassemble the memory starting at <address> for an optional" << '\n';
    *writer << "                          <count> instructions. Default is 8 instructions." << '\n';
}

} // namespace m68k
