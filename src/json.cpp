#include "astervm/core.hpp"
#include <iomanip>
#include <sstream>

namespace astervm {
namespace {
std::string quote(const std::string& text) {
    std::ostringstream out; out<<'"';
    const char* digits="0123456789abcdef";
    for(unsigned char c:text) {
        if(c=='"' || c=='\\') out<<'\\'<<char(c);
        else if(c<32 || c>=127) out<<"\\u00"<<digits[c>>4]<<digits[c&15];
        else out<<char(c);
    }
    out<<'"'; return out.str();
}
std::string memory_json(const std::vector<MemoryChange>& changes) {
    std::ostringstream out; out<<'['; bool first=true;
    for(const auto& c:changes) {
        if(!first) out<<',';
        first=false;
        out<<"{\"address\":"<<c.address<<",\"before\":"<<unsigned(c.before)
           <<",\"after\":"<<unsigned(c.after)<<'}';
    }
    out<<']'; return out.str();
}
}
const char* name(Outcome value) noexcept {
    switch(value) {
    case Outcome::Retired:return "retired";
    case Outcome::Halted:return "halted";
    case Outcome::Unsupported:return "unsupported";
    case Outcome::Fault:return "fault";
    }
    return "fault";
}
const char* name(Stop value) noexcept {
    switch(value) {
    case Stop::Halted:return "halted";
    case Stop::Unsupported:return "unsupported";
    case Stop::Fault:return "fault";
    case Stop::Budget:return "budget";
    }
    return "fault";
}
std::string state_json(const CpuState& state) {
    static constexpr const char* regs[]={"eax","ecx","edx","ebx","esp","ebp","esi","edi"};
    static constexpr const char* segs[]={"es","cs","ss","ds","fs","gs"};
    std::ostringstream out; out<<'{';
    for(std::size_t i=0;i<8;++i) { if(i) out<<','; out<<quote(regs[i])<<':'<<state.gpr[i]; }
    for(std::size_t i=0;i<6;++i) out<<','<<quote(segs[i])<<':'<<state.segment[i];
    out<<",\"eip\":"<<state.eip<<",\"eflags\":"<<state.eflags
       <<",\"halted\":"<<(state.halted?"true":"false")<<'}'; return out.str();
}
std::string trace_json(const StepRecord& r) {
    std::ostringstream out;
    out<<"{\"schema\":\"astervm.trace.v1\",\"before\":"<<state_json(r.before)
       <<",\"after\":"<<state_json(r.after)<<",\"outcome\":"<<quote(name(r.result.outcome))
       <<",\"retired\":"<<(r.retired?"true":"false")<<",\"bytes\":[";
    for(unsigned i=0;i<r.result.instruction.length && i<15;++i) {
        if(i) out<<',';
        out<<unsigned(r.result.instruction.bytes[i]);
    }
    out<<"],\"writes\":"<<memory_json(r.writes)<<",\"io\":[";
    bool first=true;
    for(const auto& e:r.io) {
        if(!first) out<<',';
        first=false;
        out<<"{\"port\":"<<e.port<<",\"value\":"<<unsigned(e.value)
           <<",\"input\":"<<(e.input?"true":"false")<<'}';
    }
    out<<"],\"detail\":"<<quote(r.result.detail)<<'}'; return out.str();
}
std::string snapshot_json(const Machine& machine, const RunResult& run) {
    std::ostringstream out;
    out<<"{\"schema\":\"astervm.snapshot.v1\",\"contract\":\""<<contract_version
       <<"\",\"mode\":\"real16\",\"stop\":"<<quote(name(run.stop))
       <<",\"steps\":"<<run.steps<<",\"attempts\":"<<run.attempts
       <<",\"state\":"<<state_json(machine.state())<<",\"tty\":"<<quote(machine.bus().tty())
       <<",\"memory\":"<<memory_json(machine.bus().changes())
       <<",\"detail\":"<<quote(run.detail)<<'}'; return out.str();
}
} // namespace astervm
