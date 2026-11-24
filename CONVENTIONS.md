# Java → C++ conversion conventions

This directory holds a **faithful C++17 port** of the Java sources in `../java/src`.
The goal is behaviour-preserving, reviewable, mechanically-traceable code — *not* a
redesign. Every C++ construct below has a fixed Java counterpart. Follow it exactly.

## Layout

Java package `m68k.cpu.instructions` → `cpp/src/m68k/cpu/instructions`, one `.h` + `.cpp`
pair per Java class, same class name and file name. Include root is `cpp/src`, so a header
is reached as `#include "m68k/cpu/Cpu.h"`.

## Type mapping

| Java | C++ |
|------|-----|
| `int` | `int32_t` (include `<cstdint>`) |
| `boolean` | `bool` |
| `String` | `std::string` |
| `StringBuilder` | `std::string` (`append(x)` → `+=`) |
| `byte` (as a cast/slot) | `uint8_t` for formatting, `int32_t` for arithmetic |
| `short` (cast) | `uint16_t` when fed to a hex format, else `int32_t` |
| `char` | `char` |
| `void` | `void` |
| `null` | `nullptr` |
| `final` on a field/method | *(drop it)* |
| `public static final int` constant | `inline constexpr int32_t` in `m68k::cpu` |
| Java `int` arithmetic | identical, because the build uses `-fwrapv` |

Java `int` is exactly `int32_t`. Signed overflow wraps in Java and is UB in C++, so the
build compiles with `-fwrapv`; **never** "fix" arithmetic that looks like it might overflow.

## Language features

* **Interface → abstract class** with `virtual ~X() = default;` and pure virtuals.
  Implementing classes write `override`.
* **Nested/anonymous class implementing `Instruction`** → `LambdaInstruction` (see
  `m68k/cpu/LambdaInstruction.h`). Java:
  ```java
  i = new Instruction() {
          public int execute(int opcode) { return add_byte_dn_dest(opcode); }
          public DisassembledInstruction disassemble(int address, int opcode) {
              return disassembleOp(address, opcode, Size.Byte);
          }
      };
  ```
  C++:
  ```cpp
  i = std::make_shared<LambdaInstruction>(
      [this](int32_t opcode) { return add_byte_dn_dest(opcode); },
      [this](int32_t address, int32_t opcode) { return disassembleOp(address, opcode, Size::Byte); });
  ```
* **`InstructionHandler.register(InstructionSet)`** → `registerInstructions(InstructionSet&)`
  (`register` is a C++ keyword).
* Handler constructors take `Cpu&` and store `Cpu& cpu;` — the Java field is
  `protected final Cpu cpu`. Instruction classes must **not** depend on `CpuCore`.
* **`Operand` parameters become pointers**: Java `Operand src = cpu.resolveSrcEA(...)`
  → `Operand* src = cpu.resolveSrcEA(...)`, and every `src.getByte()` → `src->getByte()`.
  `DisassembledOperand` / `DisassembledInstruction` stay value types (returned by value).
* **enum** `Size.Byte` → `Size::Byte`; `InstructionType.ADD` → `InstructionType::ADD`.
  The `Size` accessors became free functions: `sz.byteCount()` → `byteCount(sz)`,
  `sz.ext()` → `ext(sz)`, `sz.msb()` → `msb(sz)`.
* **Flag constants** live in namespace `m68k::cpu`: `cpu.C_FLAG` → `C_FLAG`
  (likewise `V_FLAG`, `Z_FLAG`, `N_FLAG`, `X_FLAG`, `SUPERVISOR_FLAG`, …).
* **Exceptions**: `new IllegalArgumentException(m)` → `std::invalid_argument(m)`,
  `new RuntimeException(m)` → `std::runtime_error(m)`, `new CpuException(m)` → `CpuException(m)`.
  Prefix with `throw`. Never catch with `catch (Exception e)`; catch `const std::exception&`.
* **Arrays**: `int[] x = new int[n]` → `std::vector<int32_t> x(n)`; fixed-size → `std::array`.
  `new Instruction[65536]` → `std::array<std::shared_ptr<Instruction>, 65536>`.
* **`String.format(...)`** → `m68k::util::format(...)` (same `printf` specifiers).
  An integer formatted with `%s` must become `%d` / `%x` / `%04x` as appropriate.
* **`Integer.toHexString(v)`** → `m68k::util::toHexString(v)`.
  `Integer.parseInt(s)` → `std::stoi(s)`; `Integer.parseInt(s, 16)` → `std::stoi(s, nullptr, 16)`;
  `Integer.parseInt(s, 2)` → `std::stoi(s, nullptr, 2)`.
* **`Long.parseLong(s) & 0x0ffffffffL`** → `std::stoll(s)` then cast to `int32_t`.
* **`>>>` (unsigned shift)** → `static_cast<uint32_t>(x) >> n` (Java `>>` maps directly).
  Mask any variable shift count: `x << (n & 31)` / `x >> (n & 31)`, because Java masks and
  C++ would be UB.
* **`Math.abs`** → `std::abs` (`<cstdlib>`); other `Math` calls → `<cmath>` equivalents.
* `String` methods: `substring(a)` → `substr(a)`, `substring(a, b)` → `substr(a, b - a)`,
  `charAt(i)` → `at(static_cast<size_t>(i))`, `equals(x)` → `==`, `length()` → `size()`,
  `toUpperCase()`/`toLowerCase()` → `std::toupper`/`std::tolower` per character.
* Java `switch` on an enum/int keeps working; every Java `default:` stays, and a missing
  `default` gets one that throws the equivalent exception.
* Do **not** add `using namespace std;`. Qualify with `std::`.
* Keep the original comments, identifiers and statement order. Keep Java method names
  (`toString()` stays `toString()`, returning `std::string`).
* Instruction execution must not allocate. `std::shared_ptr<LambdaInstruction>` objects are
  only created inside `registerInstructions()`.

## Collections, streams and errors

| Java | C++ |
|------|-----|
| `List<T>`, `ArrayList<T>` | `std::vector<T>` |
| `LinkedList<T>` (used as a queue/stack) | `std::deque<T>` |
| `Map<K,V>`, `HashMap<K,V>` | `std::unordered_map<K,V>` (iterate deliberately, never rely on order) |
| `Set<T>`, `HashSet<T>` | `std::unordered_set<T>` |
| `Comparator<T>` / `Collections.sort` | `std::sort(v.begin(), v.end(), [](const T& a, const T& b){...})` |
| `System.out` / `PrintStream` | `std::ostream&` (`std::cout`), `#include <iostream>` |
| `System.err` | `std::cerr` |
| `FileReader` + `BufferedReader` | `std::ifstream` + `std::getline` |
| `FileWriter`, `FileOutputStream` | `std::ofstream` (`#include <fstream>`) |
| `IOException` | `std::runtime_error` (or `std::ios_base::failure`) |
| `ParseException` | `class ParseException : public std::runtime_error` |
| `Pattern` / `Matcher` | `std::regex` (`#include <regex>`) |
| `Thread` / `Runnable` | `std::thread` (`#include <thread>`) |
| `Integer.MAX_VALUE` / `MIN_VALUE` | `INT32_MAX` / `INT32_MIN` (`<cstdint>`) |

Iterate maps with `for (const auto& entry : m)` and use `entry.first` / `entry.second`.
`Map.get(k)` returning null maps to `find()` + `end()` check, or `std::optional`.

## Files in this port already written (read them before converting)

* `m68k/cpu/Size.h`, `InstructionType.h`, `Operand.h`, `Instruction.h`, `InstructionSet.h`,
  `InstructionHandler.h`, `LambdaInstruction.h`, `Cpu.h`, `CpuUtils.h`, `CpuException.h`
* `m68k/cpu/CpuCore.h` / `.cpp`, `MC68000.h` / `.cpp`, `Disassembled*.h` / `.cpp`
* `m68k/memory/*`
* `m68k/util/Format.h`
* **`m68k/cpu/instructions/ADD.h` / `.cpp` — the reference conversion.**

## Build

```
cmake -S cpp -B cpp/build -G "MinGW Makefiles"
cmake --build cpp/build -j
```

Compiler flags include `-std=c++17 -fwrapv`. Include root is `cpp/src`.
