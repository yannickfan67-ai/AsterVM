#include "astervm/core.hpp"

namespace astervm {
DecodedInstruction decode16(const CpuState& state, Bus& bus) {
    DecodedInstruction d;
    const auto fetch=[&](){
        const auto value=bus.read8(real_address(state.segment[CS],std::uint16_t(state.eip+d.length)));
        d.bytes.at(d.length++)=value; return value;
    };
    const auto word=[&](){ const auto lo=fetch(); const auto hi=fetch(); return std::uint16_t(lo|(hi<<8)); };
    const auto op=fetch();
    if(op>=0xb0 && op<=0xbf) {
        d.operation=Operation::MovImmediate; d.reg=op&7; d.width=op<0xb8?8:16;
        d.immediate=d.width==8?fetch():word(); return d;
    }
    if(op>=0x40 && op<=0x4f) {
        d.operation=op<0x48?Operation::Increment:Operation::Decrement;
        d.width=16; d.reg=op&7; return d;
    }
    if(op>=0x50 && op<=0x5f) {
        d.operation=op<0x58?Operation::PushRegister:Operation::PopRegister;
        d.width=16; d.reg=op&7; return d;
    }
    switch(op) {
    case 0x68: case 0x6a:
        d.operation=Operation::PushImmediate; d.width=16;
        if(op==0x68) d.immediate=word();
        else { const auto v=fetch(); d.immediate=v<128?v:std::uint16_t(0xff00u|v); }
        return d;
    case 0xe8: d.operation=Operation::CallNear; d.width=16; d.immediate=word(); return d;
    case 0xc2: case 0xc3:
        d.operation=Operation::ReturnNear; d.width=16;
        if(op==0xc2) d.immediate=word();
        return d;
    case 0xf5: case 0xf8: case 0xf9: case 0xfc: case 0xfd:
        d.operation=Operation::FlagControl; d.immediate=op; return d;
    case 0xe0: case 0xe1: case 0xe2: case 0xe3:
        d.operation=Operation::Loop; d.width=8; d.reg=op&3; d.immediate=fetch(); return d;
    case 0x04: case 0x05: d.operation=Operation::Add; break;
    case 0x2c: case 0x2d: d.operation=Operation::Sub; break;
    case 0x3c: case 0x3d: d.operation=Operation::Compare; break;
    case 0xeb: case 0xe9: d.operation=Operation::Jump; d.width=op==0xeb?8:16;
        d.immediate=d.width==8?fetch():word(); return d;
    case 0x74: case 0x75: d.operation=op==0x74?Operation::JumpZero:Operation::JumpNotZero;
        d.width=8; d.immediate=fetch(); return d;
    case 0x90: d.operation=Operation::Nop; return d;
    case 0xf4: d.operation=Operation::Halt; return d;
    case 0xa0: case 0xa2: d.operation=op==0xa0?Operation::LoadAl:Operation::StoreAl;
        d.width=8; d.immediate=word(); return d;
    case 0xe4: case 0xe6: case 0xec: case 0xee:
        d.operation=(op==0xe4 || op==0xec)?Operation::Input:Operation::Output;
        d.width=8; d.port_from_dx=op==0xec || op==0xee;
        if(!d.port_from_dx) d.immediate=fetch();
        return d;
    default: return d;
    }
    d.width=(op&1)?16:8;
    d.immediate=d.width==8?fetch():word(); return d;
}
} // namespace astervm
