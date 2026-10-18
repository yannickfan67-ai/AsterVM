# AsterVM 职业与首批工作

CEO：Yannick Fan。副 CEO：GPT-6 / 群昵称 GPT6 Studio。

| 成员 | 按已报意愿确定的职业 | 首批任务与文件边界 | 验收 |
|---|---|---|---|
| GPT-6 | 架构、机器运行层与工程整合负责人 | ASTER-CORE-001：公共 core.hpp、machine.cpp、CLI、快照/trace、构建和接入桥 | 原生机器码演示；核心和CLI测试；实际接入独立向量；源码包与日志 |
| Manus | CPU 解释执行工程师 | ASTER-CPU-001：接管 src/execute16.cpp，寄存器/EFLAGS、控制流与执行语义 | 在冻结契约下逐指令对拍；先消除当前 QA 支持缺口，附原始失败与修复证据 |
| QWEN 35 | CPU 解码与有效地址工程师 | ASTER-DEC-001：接管 src/decode16.cpp；扩展前缀、ModR/M/EA 先提契约增量；提供 hello16 实际文件 | 取指无写副作用、长度/字节/操作数正确；与执行器同包联测；镜像完整 SHA 和初态/BIOS依赖 |
| GLM | 内存、保护模式与分页工程师 | ASTER-MEM-001：M0物理/实模式地址适配；M1 memory.h/translate、描述符、分页独立包 | RAM边界、回绕和失败副作用测试；M1先审 #PF/CR2/A/D/权限接口，再接核心 |
| Genspark QA | 测试、调试器与正确性验证工程师 | ASTER-QA-001（原X86-QA-001）：独立向量、原生适配/差分/失败缩减；其自报候选整数核心另包评审 | 原始40案例保留；框架与参考/产品核心结果分开；未支持项显式列出 |
| DeepSeek | 待岗位意愿回复 | 回复首选/备选、环境与可验收的首项成果 | 尚不计已接单，确认后再定模块 |

Manus 已在群里确认当前昵称；Qwen 的 hello16/核心和 GLM 扩展模块目前按自报进度记录，未取得实际文件时不记为独立验收通过。Genspark 候选核心也未整合到本包。

## 交付节奏

1. 每人先确认范围与接口阻塞，基于本源码包工作，保留同一套公共契约。
2. 跨文件更改先给出契约增量、理由和受影响测试；解码与执行的同一新指令须成对交付。
3. 包含源码、版本、完整 SHA-256、构建/测试命令、实际日志、已验证平台与剩余失败列表。
4. 交付到共享文件夹并在群里报 Task ID。没有连接同一实际 Git 仓库时，提交源码包或 patch，避免仅声称 PR 已建立。
5. GPT-6 独立复验后更新集成基线；Windows 测试须有 Windows 环境的实际输出。

协作群：https://quickroom.org/?room=dZIh10F60K7SVQb1Ejq__IXj

交付文件夹：https://file.kiwi/e7972a82#2kSVAE_WUdOOoU3QVirxGQ
