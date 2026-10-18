#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace astervm {
constexpr const char* contract_version = "astervm.m0.v1";
enum Register : std::size_t { AX, CX, DX, BX, SP, BP, SI, DI };
enum Segment : std::size_t { ES, CS, SS, DS, FS, GS };
enum Flag : std::uint32_t { CF=1, PF=4, AF=16, ZF=64, SF=128, IF=512, DF=1024, OF=2048 };
struct CpuState {
    std::array<std::uint32_t,8> gpr{};
    std::array<std::uint16_t,6> segment{};
    std::uint32_t eip=0, eflags=2;
    bool halted=false;
};
std::uint32_t real_address(std::uint16_t segment, std::uint16_t offset) noexcept;
std::uint16_t reg16(const CpuState&, std::size_t);
std::uint8_t reg8(const CpuState&, std::size_t);
void set_reg16(CpuState&, std::size_t, std::uint16_t);
void set_reg8(CpuState&, std::size_t, std::uint8_t);
bool operator==(const CpuState&, const CpuState&) noexcept;

class Bus {
public:
    virtual ~Bus()=default;
    virtual std::uint8_t read8(std::uint32_t physical)=0;
    virtual void write8(std::uint32_t physical, std::uint8_t value)=0;
    virtual std::uint8_t in8(std::uint16_t port)=0;
    virtual void out8(std::uint16_t port, std::uint8_t value)=0;
};
struct MemoryChange { std::uint32_t address; std::uint8_t before, after; };
struct IoEvent { std::uint16_t port; std::uint8_t value; bool input; };
// FlatBus is a test/reference physical bus, not an MMU or a PC chipset.
class FlatBus final : public Bus {
public:
    explicit FlatBus(std::size_t bytes=1u<<20);
    std::uint8_t read8(std::uint32_t) override;
    void write8(std::uint32_t, std::uint8_t) override;
    std::uint8_t in8(std::uint16_t) override;
    void out8(std::uint16_t, std::uint8_t) override;
    void load(std::uint32_t, const std::vector<std::uint8_t>&);
    void set_input(std::uint16_t, std::uint8_t);
    std::size_t size() const noexcept { return ram_.size(); }
    const std::string& tty() const noexcept { return tty_; }
    std::vector<MemoryChange> changes() const;
    void begin_step();
    void commit_step();
    void rollback_step() noexcept;
    const std::vector<MemoryChange>& step_changes() const noexcept { return journal_; }
    const std::vector<IoEvent>& step_io() const noexcept { return io_; }
    void clear_history();
private:
    void check(std::uint32_t) const;
    std::vector<std::uint8_t> ram_;
    std::array<std::uint8_t,65536> inputs_{};
    std::vector<MemoryChange> journal_;
    std::vector<IoEvent> io_;
    std::map<std::uint32_t,MemoryChange> changes_;
    std::string tty_;
    bool active_=false;
};

enum class Operation { Unsupported, MovImmediate, Add, Sub, Compare, Increment,
    Decrement, Jump, JumpZero, JumpNotZero, Nop, Halt, LoadAl, StoreAl, Input, Output };
struct DecodedInstruction {
    Operation operation=Operation::Unsupported;
    std::array<std::uint8_t,15> bytes{};
    std::uint8_t length=0, width=0, reg=0;
    std::uint16_t immediate=0;
    bool port_from_dx=false;
};
enum class Outcome { Retired, Halted, Unsupported, Fault };
struct StepResult {
    Outcome outcome=Outcome::Unsupported;
    DecodedInstruction instruction{};
    std::string detail;
};
// Qwen owns decode16; it must not mutate state, RAM or ports.
DecodedInstruction decode16(const CpuState&, Bus&);
// Manus owns execute16; Machine supplies transaction/rollback boundaries.
StepResult execute16(CpuState&, Bus&, const DecodedInstruction&);
class Stepper {
public:
    virtual ~Stepper()=default;
    virtual StepResult step(CpuState&, Bus&)=0;
};
class Interpreter16 final : public Stepper {
public:
    StepResult step(CpuState&, Bus&) override;
};
struct StepRecord {
    CpuState before{}, after{};
    StepResult result{};
    bool retired=false;
    std::vector<MemoryChange> writes;
    std::vector<IoEvent> io;
};
enum class Stop { Halted, Unsupported, Fault, Budget };
struct RunResult { Stop stop=Stop::Budget; std::uint64_t steps=0, attempts=0; std::string detail; };
class Machine {
public:
    explicit Machine(std::size_t ram_bytes=1u<<20) : bus_(ram_bytes) {}
    CpuState& state() noexcept { return state_; }
    const CpuState& state() const noexcept { return state_; }
    FlatBus& bus() noexcept { return bus_; }
    const FlatBus& bus() const noexcept { return bus_; }
    void reset(std::uint16_t cs=0, std::uint16_t ip=0x7c00);
    StepRecord step(Stepper&);
    RunResult run(Stepper&, std::uint64_t budget,
                  const std::function<void(const StepRecord&)>& observer={});
    std::uint64_t retired() const noexcept { return retired_; }
private:
    CpuState state_{};
    FlatBus bus_;
    std::uint64_t retired_=0;
};
const char* name(Outcome) noexcept;
const char* name(Stop) noexcept;
std::string state_json(const CpuState&);
std::string trace_json(const StepRecord&);
std::string snapshot_json(const Machine&, const RunResult&);
std::vector<std::uint8_t> demo_program();
} // namespace astervm
