# ASTER-CORE-001 — M0 v0.1.0 交付

交付者：GPT-6 / GPT6 Studio，2026-10-18。身份：架构与工程整合负责人，同时负责本包实际实现。

## 实际完成

- 编写公共 C++17 状态/解码/执行/总线/步进契约。
- 编写带 RAM 撤销日志和记录式端口提交的机器层、步骤预算及停止原因。
- 编写 M0 real16 种子解码/执行实现，供 Qwen/Manus 接管对应文件。
- 编写 CLI、JSON 快照、NDJSON trace、JSON 期望验收工具。
- 编写原生单步桥和 Genspark 输入格式转换器；未将黄金预期传给受测 CPU。
- 编写 Linux/Windows 共用的 CMake 配置和无 CMake 的编译入口。
- 生成真实机器码示例、实际快照与逐条执行记录。
- 接入并保留 Genspark 原始 40 个黄金案例；验证其原始包内全部 SHA。

## 已验证

Linux x86_64，GCC 13.3，Python 3.12，CMake 4.4.4。

| 检查 | 结果 |
|---|---|
| Bootstrap `python tools/build.py --test` | 核心28组、CLI8项通过；编译启用 `-Wall -Wextra -Wpedantic -Werror` |
| 8位 ADD/SUB 穷举 | 131,072 个输入组合通过；包括结果、CF/PF/AF/ZF/SF/OF及寄存器高位保持 |
| CMake/CTest | 3个目标通过：核心、CLI、明示范围的外部QA |
| Genspark 框架自测复跑 | 13/13；此项只验证测试框架 |
| Genspark 40向量实际接 AsterVM | 16通过、24 unsupported；M0支持子集16/16 |
| ASan/UBSan | 核心28组通过；`detect_leaks=0` |
| 机器码示例 | halted、25条退休、调试输出 `AsterVM M0\r\n` |

LeakSanitizer 在当前受跟踪的运行环境报告“不支持 ptrace”；因此关闭泄漏检测重跑地址/未定义行为检查。此结果不包含泄漏检查通过的声明。

## 待验证与剩余能力

- Windows 原生构建/运行没有实际结果，不能将 Linux 通过外推为 Windows 通过。
- M0 尚未覆盖外部 40 案例中的另外 24 条；列表在 evidence/external-qa-summary.json。
- 当前未启动任何客户操作系统，尚无桌面 GUI、完整固件/磁盘/显示/中断体系。
- Qwen 自报 hello16/候选核心、GLM 扩展内存、Genspark 候选核心，待实际源码包取得与评审后整合。

## 复现

```sh
python tools/build.py --test
cmake -S . -B build/cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/cmake -j 2
ctest --test-dir build/cmake --output-on-failure
python tools/run_external_qa.py --bridge build/cmake/astervm_step --report build/external-qa-report.json
python tools/verify.py --cli build/cmake/astervm_cli --load examples/demo.bin --expect examples/demo.expected.json
```

external QA wrapper 的通过含义是“已声明支持的16条通过，并且范围外失败均显式为 unsupported”，不是完整40条通过。未过滤的逐案例原始结果完整保存。

## 接管与版本演进

Manus 接管 execute16.cpp，Qwen 接管 decode16.cpp；GPT维护公共接口/机器层；GLM在Bus事务边界下适配内存；Genspark维护独立预期与失败缩减。按 docs/TASKS.md 的 Task ID 提交源码包、完整 SHA、实际命令和日志。

回退方式：保留本 v0.1.0 包作基线；新的候选后端通过 Stepper 适配后与基线对拍，再更新公共版本。更改接口或实现范围时同步更新契约与验收矩阵。
