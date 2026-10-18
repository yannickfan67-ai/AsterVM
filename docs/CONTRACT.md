# ASTER-CONTRACT-001 — astervm.m0.v1

## 模式与初态

- 唯一支持模式：real16。物理参考总线默认 1 MiB RAM，初始零值。
- 测试地址为 `((segment << 4) + uint16_offset) & 0xfffff`，固定 A20-off。
- 指令逐字节取指，每次按 16 位 IP 回绕；不模拟预取队列或精确时钟周期。
- `Machine::reset(cs, ip)` 是测试上下文初始化，默认入口 `0000:7c00`。它清空 CPU 状态、历史和调试输出，保留 RAM；不声称是某型号 CPU 的完整上电复位。GPR/段均为 0，EFLAGS=2，halted=false，SP=0。
- 公共状态是八个 uint32 GPR（AX,CX,DX,BX,SP,BP,SI,DI）、六个 uint16 段（ES,CS,SS,DS,FS,GS）、uint32 EIP/EFLAGS 和 halted。real16 的 EIP 必须小于 65536。
- 16/8 位寄存器写入保留其余位；FLAGS bit 1 保持为 1。FS/GS 暂只作状态保留，尚无相关前缀执行。

## 三个边界

1. Qwen：`DecodedInstruction decode16(const CpuState&, Bus&)`。只从 RAM 取指，不能写状态、内存或访问设备端口；返回操作、原始字节、长度、宽度、寄存器、立即数及端口形式。
2. Manus：`StepResult execute16(CpuState&, Bus&, const DecodedInstruction&)`。计算下一状态、旗标和数据访问。新增编码/操作须同时更新契约版本、支持矩阵和独立测试。
3. GPT-6：`Machine::step(Stepper&)`。包住整条指令，提交成功结果，回滚失败步骤，并形成 trace。`Stepper` 可接不同候选 CPU 后端，公共状态与语义保持一致。

`include/astervm/core.hpp` 是本版实际可编译的接口源。不是用文字声明替代接口文件。

## 停止与提交

`retired`：完成一条指令；`halted`：本次 HLT 退休，IP 位于 HLT 后。已 halted 的再次 step 不退休新指令。

`unsupported`：不在当前实现范围；`fault`：物理 RAM 越界、核心抛出的标准异常或核心违反 M0 状态/步进契约。二者均恢复失败前 CPU、RAM 和尚未提交的输出；失败指令不计入退休数。fault 暂不等同于体系结构 #PF/#GP 投递。

RAM 写入有逐地址撤销日志；同一步重复写同址，回滚到该步之前的值。端口输出仅在成功提交后进入调试输出。此回滚覆盖当前参考总线的 RAM 与记录式端口行为；真实设备的读副作用必须另定事务契约。宿主内存耗尽等资源故障由进程按宿主错误退出，不作为可恢复客户异常。

`run` 的 budget 是最大可退休指令数，不是 CPU 周期。失败即返回；循环程序必在预算内结束。trace 对每次尝试生成一条记录，包含失败尝试；零预算和已 halted 不执行新指令。

## 总线、GLM 接入与设备

`Bus::read8/write8` 接收物理地址，参考 FlatBus 不擅自做 20 位回绕；实模式转换在 CPU 访问前执行。`in8/out8` 接收 uint16 端口。未映射输入为 0xFF，输出作记录；0xE9 是当前测试专用字节输出端口，不是 BIOS、串口或 VGA。

GLM 的物理内存可先以适配器满足 Bus 字节接口并对拍。保护模式/分页属于 M1：translate 的 linear address、access kind、CPL、CR0/CR3/CR4、返回 physical/fault、CR2/#PF 码及 A/D 写回需先评审。当前 Machine 直接使用有撤销能力的 FlatBus；替换时必须同时实现相同的提交/回滚边界，不能只替换 read8。

## JSON 和外部 QA

快照 `schema=astervm.snapshot.v1`：contract、mode、stop、steps、attempts、state、tty、memory、detail。state 使用 eax/ecx/edx/ebx/esp/ebp/esi/edi/es/cs/ss/ds/fs/gs/eip/eflags/halted。memory 是相对于加载后初态的净改变，按地址排序，字段 address/before/after。

trace `schema=astervm.trace.v1`：before/after、outcome、retired、bytes、writes、io、detail。tty 和 detail 将高字节按 U+00xx 转义，调试端口以字节串看待。

Genspark 适配器只收到 id/mode/code/initial/budget，绝不接收 expected。Python 将输入状态与 1 MiB RAM 编成 `AVMSTEP1` 小端测试帧；原生 `astervm_step` 执行一次真实 `Machine::step` 并返回全部 RAM。Python 只负责格式转换。该测试帧是适配协议，不是虚拟磁盘格式。

失败样例须附：契约版本、初始状态、机器码、步骤预算、有效旗标 mask、RAM 预期差异、实际快照/trace、构建与运行命令。外部模型自测与产品核心结果分别标记。
