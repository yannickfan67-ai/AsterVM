#include "astervm/core.hpp"
#include <algorithm>
#include <limits>

namespace astervm {
std::uint32_t real_address(std::uint16_t segment, std::uint16_t offset) noexcept {
    return ((std::uint32_t(segment)<<4)+offset)&0xfffffu;
}
std::uint16_t reg16(const CpuState& s, std::size_t r) { return std::uint16_t(s.gpr.at(r)); }
std::uint8_t reg8(const CpuState& s, std::size_t r) {
    if(r>=8) throw std::out_of_range("8-bit register index");
    return std::uint8_t(s.gpr.at(r%4)>>(r<4?0:8));
}
void set_reg16(CpuState& s, std::size_t r, std::uint16_t v) {
    s.gpr.at(r)=(s.gpr.at(r)&0xffff0000u)|v;
}
void set_reg8(CpuState& s, std::size_t r, std::uint8_t v) {
    if(r>=8) throw std::out_of_range("8-bit register index");
    const auto shift=r<4?0u:8u;
    auto& target=s.gpr.at(r%4);
    target=(target&~(0xffu<<shift))|(std::uint32_t(v)<<shift);
}
bool operator==(const CpuState& a, const CpuState& b) noexcept {
    return a.gpr==b.gpr && a.segment==b.segment && a.eip==b.eip &&
           a.eflags==b.eflags && a.halted==b.halted;
}
FlatBus::FlatBus(std::size_t bytes):ram_(bytes,0) {
    if(bytes==0 || bytes>std::numeric_limits<std::uint32_t>::max())
        throw std::invalid_argument("RAM size must fit a nonzero uint32 address space");
    inputs_.fill(0xff);
}
void FlatBus::check(std::uint32_t a) const {
    if(a>=ram_.size()) throw std::out_of_range("physical RAM access out of range");
}
std::uint8_t FlatBus::read8(std::uint32_t a) { check(a); return ram_[a]; }
void FlatBus::write8(std::uint32_t a, std::uint8_t v) {
    check(a);
    if(active_) {
        auto found=std::find_if(journal_.begin(),journal_.end(),[a](const auto& c){return c.address==a;});
        if(found==journal_.end()) journal_.push_back({a,ram_[a],v});
        else found->after=v;
    }
    ram_[a]=v;
}
std::uint8_t FlatBus::in8(std::uint16_t p) {
    if(active_) io_.push_back({p,inputs_[p],true});
    return inputs_[p];
}
void FlatBus::out8(std::uint16_t p, std::uint8_t v) {
    if(!active_) throw std::logic_error("port output requires a step transaction");
    io_.push_back({p,v,false});
}
void FlatBus::set_input(std::uint16_t p, std::uint8_t v) { inputs_[p]=v; }
void FlatBus::load(std::uint32_t a, const std::vector<std::uint8_t>& bytes) {
    if(active_) throw std::logic_error("cannot load during a CPU step");
    if(a>ram_.size() || bytes.size()>ram_.size()-a) throw std::out_of_range("image exceeds RAM");
    std::copy(bytes.begin(),bytes.end(),ram_.begin()+a);
}
void FlatBus::clear_history() {
    if(active_) throw std::logic_error("cannot clear an active transaction");
    journal_.clear(); io_.clear(); changes_.clear(); tty_.clear();
}
void FlatBus::begin_step() {
    if(active_) throw std::logic_error("nested CPU transaction");
    journal_.clear(); io_.clear(); active_=true;
}
void FlatBus::commit_step() {
    if(!active_) throw std::logic_error("no active CPU transaction");
    // Allocate host-side history before committing; a host allocation failure is
    // outside the architectural fault model (the CLI reports a host error).
    for(const auto& c:journal_) {
        auto it=changes_.find(c.address);
        if(it==changes_.end()) { if(c.before!=c.after) changes_.emplace(c.address,c); }
        else { it->second.after=c.after; if(it->second.before==c.after) changes_.erase(it); }
    }
    for(const auto& e:io_) if(!e.input && e.port==0xe9) tty_.push_back(char(e.value));
    active_=false;
}
void FlatBus::rollback_step() noexcept {
    if(active_) for(const auto& c:journal_) ram_[c.address]=c.before;
    journal_.clear(); io_.clear(); active_=false;
}
std::vector<MemoryChange> FlatBus::changes() const {
    std::vector<MemoryChange> result;
    for(const auto& pair:changes_) result.push_back(pair.second);
    return result;
}
void Machine::reset(std::uint16_t cs, std::uint16_t ip) {
    state_={}; state_.segment[CS]=cs; state_.eip=ip; retired_=0; bus_.clear_history();
}
StepRecord Machine::step(Stepper& core) {
    StepRecord record; record.before=state_;
    if(state_.halted) {
        record.after=state_; record.result.outcome=Outcome::Halted;
        record.result.detail="already halted"; return record;
    }
    bus_.begin_step();
    try { record.result=core.step(state_,bus_); }
    catch(const std::exception& e) { record.result.outcome=Outcome::Fault; record.result.detail=e.what(); }
    const bool success=record.result.outcome==Outcome::Retired || record.result.outcome==Outcome::Halted;
    const bool consistent=record.result.instruction.length>=1 && record.result.instruction.length<=15 &&
        state_.eip<=0xffff && (state_.eflags&2)!=0 &&
        state_.halted==(record.result.outcome==Outcome::Halted);
    if(success && !consistent) {
        record.result.outcome=Outcome::Fault; record.result.detail="core violated M0 state/step contract";
    }
    if(success && consistent) {
        record.writes=bus_.step_changes(); record.io=bus_.step_io();
        bus_.commit_step(); ++retired_; record.retired=true;
    } else { bus_.rollback_step(); state_=record.before; }
    record.after=state_; return record;
}
RunResult Machine::run(Stepper& core, std::uint64_t budget,
                       const std::function<void(const StepRecord&)>& observer) {
    RunResult result; const auto start=retired_;
    if(state_.halted) { result.stop=Stop::Halted; return result; }
    for(std::uint64_t i=0;i<budget;++i) {
        auto record=step(core); ++result.attempts;
        if(observer) observer(record);
        result.steps=retired_-start; result.detail=record.result.detail;
        switch(record.result.outcome) {
        case Outcome::Retired: break;
        case Outcome::Halted: result.stop=Stop::Halted; return result;
        case Outcome::Unsupported: result.stop=Stop::Unsupported; return result;
        case Outcome::Fault: result.stop=Stop::Fault; return result;
        }
    }
    result.stop=Stop::Budget; return result;
}
StepResult Interpreter16::step(CpuState& state, Bus& bus) {
    return execute16(state,bus,decode16(state,bus));
}
std::vector<std::uint8_t> demo_program() {
    std::vector<std::uint8_t> result;
    for(unsigned char c:std::string("AsterVM M0\r\n")) result.insert(result.end(),{0xb0,c,0xe6,0xe9});
    result.push_back(0xf4); return result;
}
} // namespace astervm
