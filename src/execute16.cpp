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
// Ordinary real16 stack with a 64 KiB segment limit. Do not wrap a word's
// second byte to SS:0000: the full word must fit, even though SP arithmetic
// itself wraps. Architectural exception delivery is outside this M0 core.
void check_stack_word(std::uint16_t offset) {
    if(offset==0xffff) throw std::out_of_range("real16 stack word exceeds segment limit");
}
void push16(CpuState& state, Bus& bus, std::uint16_t value) {
    const auto sp=std::uint16_t(reg16(state,SP)-2);
    check_stack_word(sp);
    bus.write8(real_address(state.segment[SS],sp),std::uint8_t(value));
    bus.write8(real_address(state.segment[SS],std::uint16_t(sp+1)),std::uint8_t(value>>8));
    set_reg16(state,SP,sp);
}
std::uint16_t pop16(CpuState& state, Bus& bus) {
    const auto sp=reg16(state,SP);
    check_stack_word(sp);
    const auto lo=bus.read8(real_address(state.segment[SS],sp));
    const auto hi=bus.read8(real_address(state.segment[SS],std::uint16_t(sp+1)));
    set_reg16(state,SP,std::uint16_t(sp+2));
    return std::uint16_t(lo|(std::uint16_t(hi)<<8));
}
}
StepResult execute16(CpuState& state, Bus& bus, const DecodedInstruction& d) {
    StepResult result; result.instruction=d;
    if(d.operation==Operation::Unsupported) {
        result.outcome=Outcome::Unsupported; result.detail="opcode/prefix outside M0 subset"; return result;
    }
    state.eip=std::uint16_t(state.eip+d.length);
    switch(d.operation) {
    case Operation::PushRegister:
        // Evaluate the operand before changing SP (286+ PUSH SP semantics).
        push16(state,bus,reg16(state,d.reg)); break;
    case Operation::PopRegister: {
        const auto value=pop16(state,bus);
        // POP SP writes its destination after the implicit increment.
        set_reg16(state,d.reg,value); break;
    }
    case Operation::PushImmediate: push16(state,bus,d.immediate); break;
    case Operation::CallNear:
        push16(state,bus,std::uint16_t(state.eip));
        state.eip=std::uint16_t(std::int32_t(state.eip)+displacement(d.immediate,16)); break;
    case Operation::ReturnNear:
        state.eip=pop16(state,bus);
        set_reg16(state,SP,std::uint16_t(reg16(state,SP)+d.immediate)); break;
    case Operation::FlagControl:
        switch(d.immediate) {
        case 0xf5: state.eflags^=CF; break;
        case 0xf8: state.eflags&=~CF; break;
        case 0xf9: state.eflags|=CF; break;
        case 0xfc: state.eflags&=~DF; break;
        case 0xfd: state.eflags|=DF; break;
        default: throw std::invalid_argument("invalid flag-control operation");
        }
        break;
    case Operation::Loop: {
        bool taken;
        if(d.reg==3) taken=reg16(state,CX)==0; // JCXZ never decrements CX.
        else {
            set_reg16(state,CX,std::uint16_t(reg16(state,CX)-1));
            taken=reg16(state,CX)!=0 && (d.reg==2 ||
                (d.reg==1 && (state.eflags&ZF)) || (d.reg==0 && !(state.eflags&ZF)));
        }
        if(taken) state.eip=std::uint16_t(std::int32_t(state.eip)+displacement(d.immediate,8));
        break;
    }
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
