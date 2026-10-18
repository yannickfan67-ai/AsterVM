# AsterVM CPU 接口草案 v0（QWEN 35，ASTER-CPU-001）

范围：16 位实模式子集，C++17，零外部依赖（gcc/clang 均可编译）。
目标：跑通 `acceptance/hello16/hello.bin` 并对 `expected.json` 判 PASS。

## 模块边界
- **前端口（QWEN）**：取指、legacy 前缀、ModRM/SIB、有效地址 EA 计算、#UD/#GP 触发点。
- **后端口（Manus，建议）**：ALU/EFLAGS 精确语义、控制流写回。
- **内存（GLM）**：v0 使用 flat memory；`translate()` 契约冻结后切换。
- **QA（Genspark）**：`step(snapshot) -> diff` 适配层 + 单步向量互测。

## 核心类型
```cpp
struct CpuState { uint16_t ax,bx,cx,dx,si,di,bp,sp; uint16_t ip,cs,ds,ss; uint16_t flags; };
class IMemory { uint8_t read8(uint32_t pa); void write8(uint32_t pa,uint8_t v); /* 16/32 便捷封装 */ };
class IoDevice { bool onInt(uint8_t n, CpuState&); };   // INT 0x10 AH=0E teletype 等
enum class StepResult { Ok, Halted, Fault };
```

## 黄金验收命令
```
astervm_cli --load hello.bin --expect expected.json [--trace trace.txt]
```
