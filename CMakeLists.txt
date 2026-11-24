cmake_minimum_required(VERSION 3.16)

project(jaty68k CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(M68K_BUILD_TESTS "Build the functional smoke test" ON)

# Java's int arithmetic wraps on overflow; -fwrapv makes that defined in C++,
# which is what lets the ported code be a literal transcription.
if(MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra -Wno-unused-parameter -fwrapv)
endif()

# One translation unit per ported Java class.
file(GLOB_RECURSE M68K_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/m68k/*.cpp")

# main.cpp defines main() for the executable, so it stays out of the library.
list(REMOVE_ITEM M68K_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/m68k/main.cpp")

add_library(m68k_core STATIC ${M68K_SOURCES})
target_include_directories(m68k_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Make the executables self-contained where possible, so a built binary can be
# copied to a machine that has no MinGW runtime DLLs installed.
if(MINGW)
    set(M68K_RUNTIME_LINK_OPTIONS -static -static-libgcc -static-libstdc++)
elseif(MSVC)
    set(M68K_RUNTIME_LINK_OPTIONS)
    add_compile_options(/MT)
else()
    set(M68K_RUNTIME_LINK_OPTIONS)
endif()

add_executable(jaty68k "${CMAKE_CURRENT_SOURCE_DIR}/src/m68k/main.cpp")
target_link_libraries(jaty68k PRIVATE m68k_core)
target_link_options(jaty68k PRIVATE ${M68K_RUNTIME_LINK_OPTIONS})

if(M68K_BUILD_TESTS)
    enable_testing()

    add_executable(smoke_test "${CMAKE_CURRENT_SOURCE_DIR}/tests/smoke_test.cpp")
    target_link_libraries(smoke_test PRIVATE m68k_core)
    target_link_options(smoke_test PRIVATE ${M68K_RUNTIME_LINK_OPTIONS})
    add_test(NAME smoke_test COMMAND smoke_test)

    # Executes the real firmware image; it needs linux-pcb.bin in the working
    # directory and skips itself (exit 0) when the image is absent.
    add_executable(rom_test "${CMAKE_CURRENT_SOURCE_DIR}/tests/rom_test.cpp")
    target_link_libraries(rom_test PRIVATE m68k_core)
    target_link_options(rom_test PRIVATE ${M68K_RUNTIME_LINK_OPTIONS})
    add_test(NAME rom_test COMMAND rom_test "${CMAKE_CURRENT_SOURCE_DIR}/../java/linux-pcb.bin"
             WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endif()
