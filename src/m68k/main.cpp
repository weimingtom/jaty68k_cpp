#include "m68k/Monitor.h"

/*
 * Entry point.  The Java program started in m68k.Monitor.main(String[]);
 * that logic now lives in m68k::monitorMain().
 */
int main(int argc, char** argv) {
    return m68k::monitorMain(argc, argv);
}
