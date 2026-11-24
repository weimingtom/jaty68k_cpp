#pragma once

#include <cstdint>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

#include "m68k/cpu/Cpu.h"
#include "m68k/memory/AddressSpace.h"

namespace m68k {

/*
 * C++ port of m68k.Monitor.
 *
 * The Java class implemented Runnable, but nothing in the original ever started
 * a Thread around it: main_() called run() directly.  There is no Runnable
 * interface in this port, so run() is simply a member function; no std::thread is
 * needed because the Java code never created one.
 *
 * The Java reader/writer fields become std::istream* / std::ostream* because
 * they are only assigned in run() and are therefore null before it - exactly the
 * state the Java fields were in (the Java code would have thrown a
 * NullPointerException on an early writer.println()).
 */
class Monitor {
public:
    Monitor(m68k::cpu::Cpu& cpu, m68k::memory::AddressSpace& memory);

    /// Java: public static void main_(String[] args) throws Exception
    static void main_(const std::vector<std::string>& args);

    /// Java: public void run() (the Runnable body)
    void run();

protected:
    void handleCommand(const std::string& line);

    void handleGo(const std::vector<std::string>& tokens);

    void handleBreakPoints(const std::vector<std::string>& tokens);

    void handleAutoRegs(const std::vector<std::string>& tokens);

    void handleShowBytes(const std::vector<std::string>& tokens);

    void handleDataRegs(const std::vector<std::string>& tokens);

    void handleAddrRegs(const std::vector<std::string>& tokens);

    void handleStep(const std::vector<std::string>& tokens);

    void handleMemDump(const std::vector<std::string>& tokens);

    void handleDisassemble(const std::vector<std::string>& tokens);

    void dumpInfo();

    std::string makeFlagView();

    char getPrintable(int32_t val);

    /// Java: protected int parseInt(String value) throws NumberFormatException
    int32_t parseInt(const std::string& value);

    void handlePC(const std::vector<std::string>& tokens);

    void handleSR(const std::vector<std::string>& tokens);

    void handleCCR(const std::vector<std::string>& tokens);

    void handleUSP(const std::vector<std::string>& tokens);

    void handleSSP(const std::vector<std::string>& tokens);

    void handleMemLong(const std::vector<std::string>& tokens);

    void handleMemWord(const std::vector<std::string>& tokens);

    void handleMemByte(const std::vector<std::string>& tokens);

    void handleLoad(const std::vector<std::string>& tokens);

    void showHelp(const std::vector<std::string>& tokens);

private:
    m68k::cpu::Cpu& cpu;
    m68k::memory::AddressSpace& memory;
    bool running = false;
    std::string buffer;
    std::istream* reader = nullptr;
    std::ostream* writer = nullptr;
    bool showBytes = false;
    bool autoRegs = false;
    bool hasBreakpoints = false;
    std::vector<int32_t> breakpoints;
};

/// Java: public static void main(String[] args) throws Exception
int monitorMain(int argc, char** argv);

} // namespace m68k
