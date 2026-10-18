# AsterVM 当前分工与交付边界

CEO：Yannick Fan。副 CEO：GPT-6（群昵称 GPT6 Studio）。更新：2026-10-18。

| 成员 | 当前状态与职责 | 下一项可验收交付 |
|---|---|---|
| GPT-6 | 架构、机器层、CLI、构建、执行器与解码维护；接手暂不可用成员的QA接入 | ASTER-EXEC-002：v0.2栈/近调用/标志/循环；48组核心、9项CLI、外部27/40通过；保留13个unsupported |
| Claude | 已确认核心代码审查与正确性岗位；需要可取得源码 | ASTER-REVIEW-001：审查machine.cpp事务、execute16.cpp栈/控制流与decode16.cpp新增编码；按文件/位置列问题、附最小机器码、初态、命令、预期/实际、补丁与回归；独立运行ASan/UBSan，标明未验证项 |
| GLM | 内存、保护模式、分页；v0.2已在共享文件夹发布 | 补完整SHA和可取得源码；独立核验PAE 8字节布局、NX与Bus事务，再决定是否集成。565/9/300为作者自报，不计本核心通过 |
| GitHub Copilot | 已接单ASTER-STATE-001：寄存器别名、快照、状态与边界测试 | 基于core.hpp提交patch及Linux实跑日志；不得自行破坏公共接口 |
| Manus | 额度耗尽，暂停派新任务，保留角色及已有成果 | 执行器暂由GPT-6维护，恢复后基于最新版本重新交接 |
| QWEN 35 / Qwen Coder | CEO群内截图显示额度耗尽，暂停派新任务 | 解码暂由GPT-6维护；尚未取得此前hello16/CPU实际包，不计合入 |
| Genspark QA / Golden | 两位当前不可用，暂停派新任务 | QA的候选核心和40案例已保全；候选核心未合入。Golden所报642项包未取得 |
| DeepSeek | 尚待岗位意愿回复 | 未记为已接单 |

## Claude 审查任务验收

1. 先记录Git提交SHA与实际构建环境；不把他人的日志算作自己复验。
2. CPU实现通过现有48组核心、9项CLI和声明的27项外部案例。完整40例仍包含13项unsupported，不能称40/40通过。
3. 重点检查PUSH SP取旧值、POP SP顺序、CALL返回IP、RET imm16清栈、SS段限、A20/SP/IP回绕、部分写入后回滚和LOOP标志保留。
4. 若存在问题，给出可重现证据与小补丁；若未发现，也说明审查范围和限制。接口增量须写明受影响模块。
5. 未验证Windows、固件/设备和客体启动时明确标注。最终集成验收由GPT-6负责。

协作群：https://quickroom.org/?room=dZIh10F60K7SVQb1Ejq__IXj

交付目录：https://file.kiwi/e7972a82#2kSVAE_WUdOOoU3QVirxGQ
