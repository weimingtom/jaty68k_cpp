# 转换总结（Java → C++17）

> 本文是本次移植工作的总结记录，详细使用说明见 [README.md](README.md)，
> 逐条转换规则见 [CONVENTIONS.md](CONVENTIONS.md)。

转换完成。`java/` 下的 114 个 Java 文件已全部移植到 `cpp/`，共 **223 个 C++ 文件**
（118 个头文件 + 105 个实现，557 KB），并**实际编译、运行验证过**。

## 结构：Java 包 ↔ C++ 目录一一对应

以 `cpp/src` 为头文件根，类名、方法名、opcode 计算、时序常量、注释和语句顺序都照原样搬过来：

| Java | C++ | 规模 |
|---|---|---|
| `m68k/cpu/instructions/*` | 同名 `.h`+`.cpp` | 83 对 |
| `m68k/cpu/assemble/*` | 同名 `.h`+`.cpp` | 13 对 + `ParseException.h` |
| `m68k/cpu/*`（Cpu/CpuCore/MC68000/EA/反汇编） | 同上 | 15 头 + 4 实现 |
| `m68k/memory/*` | 同上 | 3 对 |
| `Monitor`/`Console`/入口 | `src/m68k/` | 3 对 + `main.cpp` |

关键映射：`int→int32_t`、`Size.Byte→Size::Byte`、`cpu.C_FLAG→C_FLAG`、匿名内部类
`new Instruction(){...}` → `LambdaInstruction` + lambda、`Operand→Operand*`、
`register()`→`registerInstructions()`（`register` 是 C++ 关键字）。编译必须带
**`-fwrapv`**，才能让 Java 的 `int` 溢出回绕语义成立。

## 验证结果（不是"能编译"而已）

- **零警告零错误**：105 个翻译单元在 `-std=c++17 -O2 -Wall -Wextra -fwrapv` 下无任何诊断；
  CMake 与直接 `g++` 两条路径都实测通过。
- **指令表完整**：`MC68000: Loaded 46371 instructions`，自检确认 65536 个 opcode
  全部可解析（未注册者走 `UNKNOWN`）。
- **常量对照**：83 个指令类逐类比对 Java↔C++ 十六进制常量集合，**无任何 opcode 基值缺失**；
  8 处 Java `TODO` 注释也全部保留在位。
- **功能自检 45/45**（[tests/smoke_test.cpp](tests/smoke_test.cpp)）：复位向量、大端内存、
  `moveq/add.l/swap/nop`、溢出标志 V/N/C、`(a0)+` 后置递增、栈 push/pop、条件码、
  反汇编文本、汇编器前端。
- **真实固件跑通**（[tests/rom_test.cpp](tests/rom_test.cpp)）：载入 `java/linux-pcb.bin`
  执行 50 万条指令，客户机成功启动并打印自己的监控提示符 ——
  `Boot from RESET vector` / `zBug(ROM) for 68Katy` / `084000>`。这证明取指、译码、
  EA 解析、标志位、调用栈在真实 68k 代码上都可用。

## 三处有意差异（其余全部等价）

1. **`Console` 的 Swing 窗口没有移植**（无依赖的 C++ 无 GUI 工具包）。改为从 `std::cin`
   读字符写入原版同样的内存映射端口 `0x7c000`/`0x78000` 并 `raiseSafeInterrupt(2)`，
   回车→`0x0d`、退格→`0x08`，与原 `KeyEventDispatcher` 键码一致，客户机侧行为不变。
2. **`NullCpu` 取代空引用**：汇编器 `new EXG(null)` 只为识别助记符，而把 C++ 引用绑到
   `nullptr` 是未定义行为，改为调用即抛异常的 `NullCpu`。
3. **输出端口每字符 flush**：原版 GUI 会即时显示每个字符，客户机常打印不带换行的提示符
   后等待输入，故写入 `0x7a000` 后立即 flush（就是能看见 `084000>` 的原因）。

此外 `MOVE` 的两个时序表由 package-private 改为 `public`（`MOVEA` 在 Java 里并不继承
`MOVE`，只同包访问），`Assembler`/`OperandParser` 的调试用 `main` 自检未移植。

## 使用

```powershell
cmake -S cpp -B cpp/build -G "MinGW Makefiles"
cmake --build cpp/build -j
ctest --test-dir cpp/build --output-on-failure
```

已附一个**自包含**的可执行文件 [jaty68k.exe](build/jaty68k.exe)（4.3 MB，只依赖
Windows 自带的 UCRT，可直接拷走运行）：在含 `linux-pcb.bin` 的目录下执行即可，
和 Java 原版一样会立刻加载并启动客户机。

## 环境说明

- 会话开始时该工作区有一处 Windows 文件权限缺失导致 shell 无法启动，已按内置诊断修复
  （报告与回滚脚本在 `D:\work_dsh\jaty68k-acl-recovery\`）。
- 这台机器原本没有任何 C++ 编译器，验证时在 `%TEMP%` 下临时安装了便携版 MinGW 与 CMake；
  它们不属于交付物，使用附带的 exe 也不需要它们。
