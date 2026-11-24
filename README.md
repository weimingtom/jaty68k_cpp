# jaty68k_cpp
[Very WIP, DO NOT USE] My cpp port of alexwinston/Jaty68k by AI DeepSeek V4.1 Flash

# jaty68k — Java 到 C++17 的移植

本目录是 `../java/src` 下 Java 版 Motorola 68000 模拟器/汇编器/调试监控器的
**C++17 移植版本**。目标是**逐类、逐方法的行为等价移植**，而不是重新设计：
类名、方法名、opcode 计算、时序常量、注释和语句顺序都尽量与 Java 原版一致，
便于对照审查。

原 Java 版本：`M68k - Java Amiga MachineCore`，Copyright (c) 2008-2010 Tony Headford
（保留原 BSD 许可声明，见各源文件头部注释）。

## 目录结构

Java 包与 C++ 目录一一对应，`cpp/src` 是头文件搜索根目录：

```
java/src/m68k/cpu/instructions/ADD.java   ->  cpp/src/m68k/cpu/instructions/ADD.h + ADD.cpp
java/src/m68k/cpu/assemble/Assembler.java ->  cpp/src/m68k/cpu/assemble/Assembler.h + Assembler.cpp
java/src/m68k/memory/MemorySpace.java     ->  cpp/src/m68k/memory/MemorySpace.h + MemorySpace.cpp
```

| 位置 | 内容 | 文件数 |
|------|------|--------|
| `src/m68k/cpu/` | `Cpu`/`CpuCore`/`MC68000`、操作数(EA)处理器、反汇编、`CpuException` | 12 对 |
| `src/m68k/cpu/instructions/` | 83 个指令处理器（每个 Java 类一个 `.h`+`.cpp`） | 83 对 |
| `src/m68k/cpu/assemble/` | 汇编器前端（Token/OperandParser/Assembler 等） | 13 对 + `ParseException.h` |
| `src/m68k/memory/` | `AddressSpace`/`MemorySpace`/`KatyMemorySpace` | 3 对 |
| `src/m68k/` | `Monitor`（主程序逻辑）、`Console`、`main.cpp` | 3 对 + main |
| `src/m68k/util/Format.h` | `String.format` / `Integer.toHexString` 的替身 | — |
| `tests/smoke_test.cpp` | 功能自检（不属于最终程序） | — |

## 编译

需要支持 C++17 的编译器。**主路径（已实测）**：

```powershell
cmake -S cpp -B cpp/build -G "MinGW Makefiles"
cmake --build cpp/build -j
ctest --test-dir cpp/build --output-on-failure   # 运行功能自检
```

产物：`cpp/build/jaty68k.exe`。也可直接调用编译器（不依赖 CMake）：

```powershell
g++ -std=c++17 -fwrapv -O2 -Icpp/src -o cpp/build/jaty68k.exe `
    (Get-ChildItem cpp/src -Recurse -Filter *.cpp).FullName
```

`-fwrapv` **不是可选项**：Java 的 `int` 溢出回绕是有定义的，而 C++ 有符号溢出是
未定义行为。有了 `-fwrapv`，才能把 Java 的算式原封不动地照抄。

## 运行

Java 原版的 `Monitor.main` 忽略命令行参数，始终加载当前工作目录下的
`linux-pcb.bin` 并立刻开始执行客户机程序（`handleGo`），因此：

```powershell
cd <包含 linux-pcb.bin 的目录>; .\jaty68k.exe
```

监控命令与 Java 版一致：`r`（寄存器）、`d <地址> <条数>`（反汇编）、`s`（单步）、
`b`（断点）、`m`/`mb`/`mw`/`ml`（内存）、`g`（运行）、`q`（退出）、`h`（帮助）。

## 移植中的对应关系

完整规则见 [`CONVENTIONS.md`](CONVENTIONS.md)，要点：

| Java | C++ |
|------|-----|
| `int` / `boolean` / `String` | `int32_t` / `bool` / `std::string` |
| 接口 `interface` | 带虚析构的抽象类 + `override` |
| `Size.Byte`、`sz.ext()` | `Size::Byte`、`ext(sz)` |
| `cpu.C_FLAG` | `C_FLAG`（命名空间常量） |
| `new Instruction(){...}` 匿名内部类 | `std::make_shared<LambdaInstruction>(lambda, lambda)` |
| `Operand` 变量 | `Operand*`（`resolveSrcEA` 返回指针） |
| `register(InstructionSet)` | `registerInstructions(InstructionSet&)`（`register` 是 C++ 关键字） |
| `IllegalArgumentException` / `RuntimeException` | `std::invalid_argument` / `std::runtime_error` |
| `StringBuilder`、`String.format` | `std::string` + `m68k::util::format` |
| `>>>` | `static_cast<uint32_t>(x) >> n` |
| `List`/`Map`/`Set` | `std::vector`/`std::unordered_map`/`std::unordered_set` |

## 有意的差异（仅这些）

1. **`Console`：Swing → 标准输入桥接。** 原版是 `JFrame` 窗口，把按键写入
   内存映射端口 `0x7c000`/`0x78000` 并 `raiseSafeInterrupt(2)`。无依赖的 C++ 移植
   没有 GUI 工具包，改为从 `std::cin` 读取字符写入同样的端口（回车→`0x0d`，
   退格→`0x08`，与原 `KeyEventDispatcher` 的键码一致）。客户机可见行为不变。
2. **`NullCpu` 取代空引用。** 汇编器用 `new EXG(null)` 只是为了识别助记符；
   C++ 引用不能为空（把引用绑定到 `nullptr` 是未定义行为），因此改绑到一个
   调用即抛异常的 `NullCpu` 对象。
3. **输出端口 `0x7a000` 每字符 flush。** 原版 GUI 会把每个字符立刻追加到
   `JTextArea`，客户机常常打印一个不带换行的提示符然后等输入；因此这里在写入该
   端口后 `std::flush`，保证提示符（如 `084000>`）立刻可见。除此之外输出逻辑不变。
3. **对象生命周期显式化。** Java 靠 GC 保活：指令的匿名内部类持有其 handler。
   C++ 用 `shared_ptr` 让 opcode 表持有指令，并由 `MC68000` 持有全部 handler，
   保证 lambda 捕获的 handler 不会悬垂。
4. **`MOVE` 的两个时序表由 package-private 改为 `public`。** `MOVEA` 在 Java 里
   并不继承 `MOVE`，只是同包访问；C++ 无包级可见性，改为 public（可达范围相同）。
5. **`Assembler` / `OperandParser` 里的调试用 `static void main` 自检没有移植**
   （它们不属于库逻辑）。`Assembler.assemble()` 在 Java 里本身就是未完成的 TODO
   （返回空集合），本移植保留该行为与注释。
6. **例外类型集中定义**：`ParseException` 单独成文件，对应
   `java.text.ParseException`（携带行号）。

## 验证情况

- **编译**：105 个翻译单元在 `-std=c++17 -O2 -Wall -Wextra` 下零警告零错误；
  `-fwrapv` 打开。CMake 与直接 `g++` 两条路径都实测可构建并链接。
- **opcode 覆盖率**：`MC68000: Loaded 46371 instructions`；自检确认 65536 个
  opcode 全部能解析到指令对象（未注册的走 `UNKNOWN` 回退）。
- **常量对照**：对 83 个指令类做 Java↔C++ 十六进制常量集合比对，无任何缺失
  （唯一差异是 `& 7` 写成 `& 0x07` 这类写法差异）。
- **功能自检**（`tests/smoke_test.cpp`，45 项，`ctest` 亦可运行）：复位向量、大端内存、
  `moveq/add.l/swap/nop` 执行与 PC 推进、溢出标志 V/N/C、`(a0)+` 后置递增、
  写内存、`pushLong`/`popLong` 栈行为、条件码判断、反汇编输出、汇编器前端
  （标签/注释/未知助记符抛 `ParseException`）——**45/45 全部通过**。
- **真实固件启动**（`tests/rom_test.cpp`）：把 `java/linux-pcb.bin`（489776 字节）
  按原版方式载入地址 0 并连续执行 50 万条指令，没有崩溃；客户机程序成功跑起来并
  打印出自己的监控提示符：

  ```
  zBug(ROM) for 68Katy (press ? for help)

  084000>
  ```

  这说明移植后的取指/译码/EA 解析/标志位/调用栈在真实 68k 代码上是可用的，
  而不仅是能编译。

## 未做的事

- 原 Java 工程不含单元测试，因此本移植的验证依靠上述自检、真实固件启动、
  opcode 覆盖统计与常量对照，而不是与 Java 版逐条对跑（该环境未安装 JDK）。
- 除 GUI 之外，`KatyMemorySpace` 仍会像原版一样在当前目录创建 `a.log`。
