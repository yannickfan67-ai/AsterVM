// astervm/cpu.hpp — AsterVM CPU 契约头（v0，QWEN 35 / ASTER-CPU-001）
#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>

namespace astervm {

// 16 位实模式寄存器组。flags 低 16 位有意义（bit2 恒 1）。
struct CpuState {
    uint16_t ax=0, bx=0, cx=0, dx=0, si=0, di=0, bp=0, sp=0xFFFE;
    uint16_t ip=0, cs=0, ds=0, es=0, ss=0;
    uint16_t flags=0x0002;
};

// 平坦内存接口（v0）。GLM 的 translate 契约冻结后由 MemorySystem 实现替换。
class IMemory {
public:
    virtual ~IMemory() = default;
    virtual uint8_t read8 (uint32_t pa) const = 0;
    virtual void    write8(uint32_t pa, uint8_t v) = 0;
    uint16_t read16(uint32_t pa) const { return uint16_t(read8(pa)) | (uint16_t(read8(pa+1))<<8); }
    void write16(uint32_t pa, uint16_t v){ write8(pa,uint8_t(v)); write8(pa+1,uint8_t(v>>8)); }
};

class FlatMemory : public IMemory {           // 默认 1MB
public:
    explicit FlatMemory(size_t bytes = 1u<<20) : d_(bytes,0), mask_(bytes-1) {}
    uint8_t read8(uint32_t pa) const override { return d_[pa & mask_]; }
    void write8(uint32_t pa, uint8_t v) override { d_[pa & mask_] = v; }
private:
    std::vector<uint8_t> d_; size_t mask_;
};

// BIOS/设备挂钩：CPU 遇 INT n 先问设备；返回 false => Fault。
class IoDevice {
public:
    virtual ~IoDevice() = default;
    virtual bool onInt(uint8_t n, CpuState& st) = 0;
};

enum class StepResult { Ok, Halted, Fault };

enum Flag : uint16_t { F_CF=0x0001, F_PF=0x0004, F_AF=0x0010, F_ZF=0x0040,
                       F_SF=0x0080, F_TF=0x0100, F_IF=0x0200, F_DF=0x0400, F_OF=0x8000 };

} // namespace astervm
