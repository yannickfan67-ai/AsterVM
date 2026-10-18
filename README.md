# AsterVM — M0 v0.1.0

AsterVM 的产品目标是可在 **Linux 与 Windows 宿主**上原生运行的桌面虚拟机软件。本包是首个可复现的开发基线：C++17 机器运行层、real16 有限指令解释器、命令行、单步桥、trace 和状态验收工具。

当前实际能力：执行下列机器码、输出调试端口数据、导出状态和 RAM 差异。桌面管理界面、完整 PC 固件/设备、保护模式和客户操作系统启动属于后续里程碑。此包没有 Windows 原生实测记录，也没有 Windows 可执行文件；跨平台源码与构建入口已提供。

## 快速运行

依赖：C++17 编译器；CMake 3.16+。Python 3.10+ 用于验收工具。

Linux：

```sh
cmake -S . -B build/cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/cmake -j 2
ctest --test-dir build/cmake --output-on-failure
./build/cmake/astervm_cli --demo --trace build/demo.ndjson --snapshot build/demo.json
```

Windows（Visual Studio C++ 工具链命令行）：

```bat
cmake -S . -B build\msvc -A x64
cmake --build build\msvc --config Release
ctest --test-dir build\msvc -C Release --output-on-failure
build\msvc\Release\astervm_cli.exe --demo
```

没有 CMake 时：`python tools/build.py --test`。Linux 默认 `g++`，Windows 默认开发者命令行内的 `cl`；可用 `CXX` 指定编译器。随包 `bin/linux-x86_64/` 是本次 Linux 环境构建的便捷产物，其他环境可从源码重建。

`--demo` 真正执行 MOV/OUT/HLT 字节码，经虚拟端口 0xE9 产生 `AsterVM M0\r\n`；终态为 halted，退休 25 条指令。标准输出始终为一个 JSON 快照。

## 加载机器码与比较预期

```sh
./build/cmake/astervm_cli --load examples/demo.bin --entry 0000:7c00 --max-steps 1000
python tools/verify.py --cli build/cmake/astervm_cli --load examples/demo.bin --expect examples/demo.expected.json
```

`--load` 将原始文件直接放到指定入口的物理地址，并从该入口解释执行。这是确定性的测试入口，不是 BIOS 引导流程；引导扇区签名本身不代表能够启动客户系统。

原生命令：`--demo` 或 `--load FILE` 二选一；可配 `--entry HEXCS:HEXIP`、`--max-steps N`、`--trace FILE`、`--snapshot FILE`。预算可为 0，最高 10,000,000。退出码：0 halted，2 参数/宿主错误，3 unsupported，4 总线/核心契约 fault，5 budget。

`tools/verify.py --expect` 比较 JSON 的指定字段，未指定字段不作验收。整数位置可用 `{"mask":2261,"value":...}` 只比较有定义的 FLAGS 位。每个期望文件必须显式给出 `stop`。

## M0 指令范围

| 编码 | 实现 |
|---|---|
| B0–B7 / B8–BF | MOV r8/r16, imm |
| 04/05、2C/2D、3C/3D | ADD/SUB/CMP AL/AX, imm |
| 40–47 / 48–4F | INC/DEC r16，保留 CF |
| EB/E9、74/75 | 相对 JMP、JZ、JNZ |
| 90 / F4 | NOP / HLT |
| A0 / A2 | AL 与 DS:moffs8 之间装载/存储 |
| E4/EC、E6/EE | IN/OUT AL，立即数端口或 DX 端口 |

前缀、ModR/M、16 位内存操作、栈、CALL/RET、BIOS/中断等尚未实现时，返回 `unsupported` 并保留失败前的状态。`unsupported` 表示实现覆盖范围，不是向客户注入 #UD。

## 验证结果和来源

- Linux GCC 13.3：28 组核心测试、8 项 CLI 集成测试通过。两组核心测试穷举了 131,072 个 8 位加减输入组合。
- CMake/CTest：3 个测试目标通过；外部 QA 目标按明示 M0 范围判定。
- Genspark 框架自测：13/13 在此环境复跑通过。
- 原生 AsterVM 单步接入 Genspark 原始 40 案例：**16 通过、24 unsupported**；M0 支持的 16 个案例全部通过。完整 40 项兼容尚未通过。完整失败列表保留在 evidence/，没有更改黄金预期或借用外部模拟器执行产品指令。

`external/astervm_qa/` 是 Genspark QA 提供的原始资料，随附原作者 README、SHA256SUMS 与自报参考日志。其 Unicorn 适配器是可选外部参考；AsterVM 核心和本包的真实 CPU 验收不使用 Unicorn 后端。

见 [接口契约](docs/CONTRACT.md)、[职业与任务](docs/TASKS.md)、[里程碑](docs/ROADMAP.md)、[本次交付记录](docs/DELIVERY.md)。
