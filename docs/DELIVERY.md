# ASTER-EXEC-002 — M0 v0.2.0

交付：GPT-6 / GPT6 Studio，2026-10-18。基于M0 v0.1.0源码扩展，原始ZIP的SHA256为 `ee9ee3408ab3a73d93511525171178b7d1b24bb96077fd8534065696cb402da5`。

新增PUSH/POP r16、PUSH imm16/符号扩展imm8、近CALL、RET/RET imm16、CLC/STC/CMC/CLD/STD、LOOPNE/LOOPE/LOOP/JCXZ。只补相应解码，不包含ModR/M、段覆盖或模式切换。公共状态、Bus、步进帧与JSON schema保持，Operation增量追加，contract升为astervm.m0.v2，需重新编译模块。

| 本地实际检查（Linux x86_64，GCC13.3，Python3.12.14，CMake4.4.4） | 结果 |
|---|---|
| 严格-Werror bootstrap | 核心48组通过，含原131072个算术组合、256个imm8符号扩展组合与LOOP/CX/ZF矩阵 |
| CLI | 9项通过，含栈调用机器码、状态与trace |
| CMake/CTest | 3个目标通过 |
| Genspark原始40例 | 27通过，13明确unsupported；声明支持子集27/27；不改原始预期 |
| QA框架自测 | 原13项通过，仅代表框架 |
| ASan + UBSan | 核心48组通过，ASAN_OPTIONS=detect_leaks=0；不声称泄漏检测通过 |
| stack-call.bin | 实际8条退休、halted，BX=1234、AX=0、SP=8000，trace/快照归档 |

原v0.1.0证据另存evidence-v0.1.0；当前结果在evidence/。运行工具不使用外部模拟器作为执行后端。

## 复现

```sh
python tools/build.py --test
cmake -S . -B build/cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/cmake -j 2
ctest --test-dir build/cmake --output-on-failure
python tools/run_external_qa.py --bridge build/cmake/astervm_step --report build/external-qa-report.json
python tools/verify.py --cli build/cmake/astervm_cli --load examples/stack-call.bin --expect examples/stack-call.expected.json
```

## 范围与风险

- SS:FFFF起点的word访问返回可回滚M0 fault，不实现#SS/#DF投递。PUSH SP采用286+语义；不是8086逐周期模拟。
- Windows原生尚无本地实跑；CI已提供Linux/Windows矩阵，须查实际运行结果后更新状态。
- 仍缺13个原始案例覆盖，见evidence/external-qa-summary.json；不是完整x86或PC系统。
- GLM v0.2包可见但下载未成功，未集成；Claude独立审查待完成。
- 没有启动客体操作系统；GUI、固件、磁盘和中断设备尚未实现。

回退：使用Git历史中的v0.1.0导入提交或原始基线ZIP。此次评审以独立开发分支提交，不直接改main。
