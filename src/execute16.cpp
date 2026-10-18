#include "astervm/core.hpp"

namespace astervm {
namespace {
constexpr std::uint32_t arithmetic_flags=CF|PF|AF|ZF|SF|OF;
std::uint32_t arithmetic(CpuState& state, std::uint32_t a, std::uint32_t b,
                         unsigned width, bool subtraction, bool preserve_cf=false) {
    const auto mask=width==8?0xffu:0xffffu;
    const auto sign=1u<<(width-1);
    const auto raw=subtraction?a-b:a+b;
    const auto result=raw&mask;
    std::uint32_t f=0;
    if(subtraction?a<b:raw>mask) f|=CF;
    unsigned ones=0; for(unsigned bit=0;bit<8;++bit) ones+=(result>>bit)&1;
    if((ones&1)==0) f|=PF;
    if((a^b^result)&0x10) f|=AF;
    if(result==0) f|=ZF;
    if(result&sign) f|=SF;
    if((subtraction?((a^b)&(a^result)):(~(a^b)&(a^result)))&sign) f|=OF;
    if(preserve_cf) f=(f&~CF)|(state.eflags&CF);
    state.eflags=(state.eflags&~arithmetic_flags)|f|2;
    return result;
}
std::int32_t displacement(std::uint16_t value, unsigned width) {
    return width==8?(value<128?std::int32_t(value):std::int32_t(value)-256):
        (value<32768?std::int32_t(value):std::int32_t(value)-65536);
}
}
StepResult execute16(CpuState& state, Bus& bus, const DecodedInstruction& d) {
    StepResult result; result.instruction=d;
    if(d.operation==Operation::Unsupported) {
        result.outcome=Outcome::Unsupported; result.detail="opcode/prefix outside M0 subset"; return result;
    }
    state.eip=std::uint16_t(state.eip+d.length);
    switch(d.operation) {
    case Operation::MovImmediate:
        if(d.width==8) set_reg8(state,d.reg,std::uint8_t(d.immediate));
        else set_reg16(state,d.reg,d.immediate);
        break;
    case Operation::Add: case Operation::Sub: case Operation::Compare: {
        const auto value=arithmetic(state,d.width==8?reg8(state,0):reg16(state,AX),d.immediate,
            d.width,d.operation!=Operation::Add);
        if(d.operation!=Operation::Compare) {
            if(d.width==8) set_reg8(state,0,std::uint8_t(value)); else set_reg16(state,AX,std::uint16_t(value));
        }
        break;
    }
    case Operation::Increment: case Operation::Decrement:
        set_reg16(state,d.reg,std::uint16_t(arithmetic(state,reg16(state,d.reg),1,16,
            d.operation==Operation::Decrement,true))); break;
    case Operation::Jump: case Operation::JumpZero: case Operation::JumpNotZero:
        if(d.operation==Operation::Jump || (d.operation==Operation::JumpZero && (state.eflags&ZF)) ||
           (d.operation==Operation::JumpNotZero && !(state.eflags&ZF)))
            state.eip=std::uint16_t(std::int32_t(state.eip)+displacement(d.immediate,d.width));
        break;
    case Operation::LoadAl: set_reg8(state,0,bus.read8(real_address(state.segment[DS],d.immediate))); break;
    case Operation::StoreAl: bus.write8(real_address(state.segment[DS],d.immediate),reg8(state,0)); break;
    case Operation::Input: set_reg8(state,0,bus.in8(d.port_from_dx?reg16(state,DX):d.immediate)); break;
    case Operation::Output: bus.out8(d.port_from_dx?reg16(state,DX):d.immediate,reg8(state,0)); break;
    case Operation::Nop: break;
    case Operation::Halt: state.halted=true; result.outcome=Outcome::Halted; return result;
    case Operation::Unsupported: break;
    }
    result.outcome=Outcome::Retired; return result;
}
} // namespace astervm
