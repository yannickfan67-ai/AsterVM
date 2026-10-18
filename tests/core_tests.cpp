#include "astervm/core.hpp"
#include <functional>
#include <iostream>
#include <stdexcept>

using namespace astervm;
namespace {
void require(bool value, const char* message) { if(!value) throw std::runtime_error(message); }
struct Fixture {
    Machine machine; Interpreter16 core;
    Fixture(std::initializer_list<std::uint8_t> bytes) { machine.reset(); machine.bus().load(0x7c00,bytes); }
    RunResult run(std::uint64_t budget=100) { return machine.run(core,budget); }
};
std::uint32_t expected8(unsigned a, unsigned b, bool sub) {
    const int signed_a=a<128?int(a):int(a)-256;
    const int signed_b=b<128?int(b):int(b)-256;
    const int signed_answer=sub?signed_a-signed_b:signed_a+signed_b;
    const unsigned answer=(sub?a-b:a+b)&255;
    unsigned ones=0; for(unsigned bit=0;bit<8;++bit) if(answer&(1u<<bit)) ++ones;
    return ((sub?a<b:a+b>255)?CF:0u) | ((ones%2==0)?PF:0u) |
           ((sub?(a&15)<(b&15):(a&15)+(b&15)>15)?AF:0u) |
           (answer==0?ZF:0u) | (answer>=128?SF:0u) |
           ((signed_answer < -128 || signed_answer>127)?OF:0u);
}
class MutateThenFail final : public Stepper {
public:
    bool throws=false;
    StepResult step(CpuState& s, Bus& b) override {
        ++s.gpr[AX]; ++s.eip; b.write8(0x100,7); b.write8(0x100,8); b.out8(0xe9,'X');
        if(throws) throw std::runtime_error("injected bus/core fault");
        StepResult r; r.outcome=Outcome::Unsupported; return r;
    }
};
}
int main() {
    unsigned passed=0,failed=0;
    const auto test=[&](const char* label,const std::function<void()>& body){
        try { body(); ++passed; std::cout<<"PASS "<<label<<'\n'; }
        catch(const std::exception& e) { ++failed; std::cerr<<"FAIL "<<label<<": "<<e.what()<<'\n'; }
    };
    test("mov_high_low_preserves_upper32",[]{
        Fixture f{0xb8,0x34,0x12,0xb4,0xab,0xb0,0xcd,0xf4};
        f.machine.state().gpr[AX]=0xfeed0000;
        require(f.run().stop==Stop::Halted,"halt"); require(f.machine.state().gpr[AX]==0xfeedabcd,"register aliases");
    });
    test("all_register_immediates",[]{
        for(unsigned r=0;r<8;++r) {
            Fixture f{std::uint8_t(0xb8+r),std::uint8_t(r),0x42,0xf4}; f.run();
            require(reg16(f.machine.state(),r)==0x4200+r,"register mapping");
        }
    });
    test("exhaustive_add8_65536_vectors",[]{
        Fixture f{0x04,0};
        for(unsigned a=0;a<256;++a) for(unsigned b=0;b<256;++b) {
            f.machine.reset(); f.machine.bus().load(0x7c01,{std::uint8_t(b)});
            f.machine.state().gpr[AX]=0xabcd1200|a;
            require(f.machine.step(f.core).retired,"add retire");
            require(reg8(f.machine.state(),0)==((a+b)&255),"add result");
            require(f.machine.state().gpr[AX]>>8==0xabcd12,"add upper bits");
            require((f.machine.state().eflags&(CF|PF|AF|ZF|SF|OF))==expected8(a,b,false),"add flags");
        }
    });
    test("exhaustive_sub8_65536_vectors",[]{
        Fixture f{0x2c,0};
        for(unsigned a=0;a<256;++a) for(unsigned b=0;b<256;++b) {
            f.machine.reset(); f.machine.bus().load(0x7c01,{std::uint8_t(b)});
            f.machine.state().gpr[AX]=a;
            require(f.machine.step(f.core).retired,"sub retire");
            require(reg8(f.machine.state(),0)==((a-b)&255),"sub result");
            require((f.machine.state().eflags&(CF|PF|AF|ZF|SF|OF))==expected8(a,b,true),"sub flags");
        }
    });
    test("add16_signed_overflow",[]{
        Fixture f{0xb8,0xff,0x7f,0x05,1,0,0xf4}; f.run();
        require(reg16(f.machine.state(),AX)==0x8000,"result");
        require((f.machine.state().eflags&(CF|OF|SF|ZF))==(OF|SF),"overflow flags");
    });
    test("add16_unsigned_carry",[]{
        Fixture f{0xb8,0xff,0xff,0x05,1,0,0xf4}; f.run();
        require((f.machine.state().eflags&(CF|ZF|AF|PF|OF))==(CF|ZF|AF|PF),"carry flags");
    });
    test("cmp_does_not_write_register",[]{
        Fixture f{0xb8,0x34,0x12,0x3d,0x34,0x12,0xf4}; f.run();
        require(reg16(f.machine.state(),AX)==0x1234,"cmp mutated AX"); require((f.machine.state().eflags&ZF)!=0,"zero");
    });
    test("inc_dec_preserve_carry_and_unrelated_flags",[]{
        Fixture f{0x40,0x48,0xf4}; f.machine.state().eflags=2|CF|IF|DF; f.run();
        require(reg16(f.machine.state(),AX)==0,"inc dec");
        require((f.machine.state().eflags&(CF|IF|DF|ZF))==(CF|IF|DF|ZF),"flag preservation");
    });
    test("conditional_loop_retires_exact_steps",[]{
        Fixture f{0xb8,0,0,0x40,0x3d,3,0,0x75,0xfa,0xf4};
        const auto r=f.run(); require(r.stop==Stop::Halted && r.steps==11,"loop steps");
        require(reg16(f.machine.state(),AX)==3,"loop AX");
    });
    test("jz_taken",[]{
        Fixture f{0x74,2,0x0f,0x0f,0xf4}; f.machine.state().eflags|=ZF;
        require(f.run().stop==Stop::Halted,"branch did not skip unsupported");
    });
    test("near_relative_jump",[]{
        Fixture f{0xe9,1,0,0x0f,0xf4}; require(f.run().steps==2,"near jump");
    });
    test("negative_near_jump_budget",[]{
        Fixture f{0xe9,0xfd,0xff}; const auto r=f.run(33);
        require(r.stop==Stop::Budget && r.steps==33 && f.machine.state().eip==0x7c00,"signed near displacement");
    });
    test("budget_stops_infinite_loop",[]{
        Fixture f{0xeb,0xfe}; const auto r=f.run(1000);
        require(r.stop==Stop::Budget && r.steps==1000 && r.attempts==1000,"budget bound");
    });
    test("zero_budget_has_no_side_effect",[]{
        Fixture f{0xf4}; const auto before=f.machine.state(); const auto r=f.run(0);
        require(r.stop==Stop::Budget && r.steps==0 && f.machine.state()==before,"zero budget");
    });
    test("halt_is_idempotent",[]{
        Fixture f{0xf4}; require(f.run().steps==1,"first HLT"); const auto before=f.machine.state();
        require(f.run().steps==0 && !f.machine.step(f.core).retired,"halt retired twice");
        require(f.machine.state()==before,"halt mutated state");
    });
    test("unsupported_preserves_ip_and_state",[]{
        Fixture f{0x0f}; const auto before=f.machine.state(); const auto r=f.run();
        require(r.stop==Stop::Unsupported && r.steps==0 && r.attempts==1,"unsupported outcome");
        require(f.machine.state()==before,"unsupported state changed");
    });
    test("prefix_is_explicitly_unsupported",[]{
        Fixture f{0x66,0x90}; require(f.run().stop==Stop::Unsupported,"prefix silently ignored");
    });
    test("fetch_wraps_16bit_ip",[]{
        Machine m; Interpreter16 core; m.reset(0,0xffff); m.bus().load(0xffff,{0xb8}); m.bus().load(0,{0x34,0x12,0xf4});
        require(m.run(core,10).stop==Stop::Halted,"fetch wrap halt");
        require(reg16(m.state(),AX)==0x1234 && m.state().eip==3,"fetch wrap result");
    });
    test("segmented_memory_and_write_diff",[]{
        Fixture f{0xb0,0x5a,0xa2,0x10,0,0xb0,0,0xa0,0x10,0,0xf4}; f.machine.state().segment[DS]=0x1000;
        require(f.run().stop==Stop::Halted && reg8(f.machine.state(),0)==0x5a,"segmented load/store");
        const auto changes=f.machine.bus().changes(); require(changes.size()==1 && changes[0].address==0x10010 && changes[0].after==0x5a,"memory diff");
    });
    test("a20_off_wraps_physical_address",[]{
        Fixture f{0xb0,0x5a,0xa2,0x10,0,0xf4}; f.machine.state().segment[DS]=0xffff;
        f.run(); require(f.machine.bus().read8(0)==0x5a,"20-bit wrap");
    });
    test("unmapped_input_returns_ff",[]{
        Fixture f{0xe4,0x80,0xf4}; f.run(); require(reg8(f.machine.state(),0)==0xff,"input default");
    });
    test("dx_port_input_output",[]{
        Fixture f{0xec,0xee,0xf4}; set_reg16(f.machine.state(),DX,0xe9); f.machine.bus().set_input(0xe9,'Z');
        f.run(); require(f.machine.bus().tty()=="Z","DX I/O");
    });
    test("demo_executes_machine_code",[]{
        Machine m; Interpreter16 core; m.reset(); m.bus().load(0x7c00,demo_program());
        const auto r=m.run(core,100); require(r.stop==Stop::Halted && r.steps==25,"demo steps");
        require(m.bus().tty()=="AsterVM M0\r\n","demo output");
    });
    test("failed_step_rolls_back_ram_state_and_io",[]{
        Machine m; m.reset(); MutateThenFail core; const auto before=m.state();
        const auto record=m.step(core);
        require(record.result.outcome==Outcome::Unsupported && m.state()==before,"state rollback");
        require(m.bus().read8(0x100)==0 && m.bus().tty().empty(),"RAM/I/O rollback");
        require(record.writes.empty() && record.io.empty() && m.bus().changes().empty(),"failure trace side effects");
    });
    test("exception_step_rolls_back",[]{
        Machine m; m.reset(); MutateThenFail core; core.throws=true; const auto before=m.state();
        const auto record=m.step(core);
        require(record.result.outcome==Outcome::Fault && m.state()==before && m.bus().read8(0x100)==0,"exception rollback");
    });
    test("out_of_range_fetch_is_host_bus_fault",[]{
        Machine m(16); Interpreter16 core; m.reset(0,16); const auto before=m.state(); const auto r=m.run(core,10);
        require(r.stop==Stop::Fault && m.state()==before && r.steps==0,"fetch fault");
    });
    test("image_load_rejects_partial_overflow",[]{
        FlatBus bus(16); bool caught=false; try {bus.load(15,{1,2});} catch(const std::out_of_range&) {caught=true;}
        require(caught && bus.read8(15)==0,"partial load mutated RAM");
    });
    test("decoder_is_read_only",[]{
        Fixture f{0xb8,0x34,0x12}; const auto before=f.machine.state();
        const auto d=decode16(before,f.machine.bus());
        require(d.length==3 && d.immediate==0x1234 && f.machine.state()==before,"decode contract");
    });
    std::cout<<passed<<" passed, "<<failed<<" failed; includes 131072 exhaustive arithmetic vectors\n";
    return failed?1:0;
}
