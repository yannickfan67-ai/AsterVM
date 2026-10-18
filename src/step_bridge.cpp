#include "astervm/core.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

int main(int argc,char** argv) {
    try {
        if(argc!=3 || std::string(argv[1])!="--input") throw std::invalid_argument("usage: astervm_step --input FIXTURE.avm");
        std::ifstream file(argv[2],std::ios::binary|std::ios::ate);
        constexpr std::size_t expected=8+52+(1u<<20);
        if(!file || file.tellg()!=std::streamoff(expected)) throw std::runtime_error("invalid fixture length");
        file.seekg(0);
        std::vector<std::uint8_t> data(expected);
        if(!file.read(reinterpret_cast<char*>(data.data()),std::streamsize(data.size()))) throw std::runtime_error("fixture short read");
        if(std::string(data.begin(),data.begin()+8)!="AVMSTEP1") throw std::runtime_error("invalid fixture magic");
        std::size_t offset=8;
        const auto u16=[&](){const auto v=unsigned(data.at(offset))|(unsigned(data.at(offset+1))<<8);offset+=2;return std::uint16_t(v);};
        const auto u32=[&](){std::uint32_t v=0;for(unsigned b=0;b<4;++b)v|=std::uint32_t(data.at(offset++))<<(8*b);return v;};
        astervm::Machine machine; machine.reset(0,0);
        for(auto& r:machine.state().gpr) r=u32();
        for(auto& s:machine.state().segment) s=u16();
        machine.state().eip=u32(); machine.state().eflags=u32();
        if(machine.state().eip>0xffff || !(machine.state().eflags&2)) throw std::runtime_error("invalid M0 input state");
        machine.bus().load(0,std::vector<std::uint8_t>(data.begin()+offset,data.end()));
        astervm::Interpreter16 core;
        const auto step=machine.step(core);
        std::string memory; memory.reserve(2u<<20);
        constexpr const char* hex="0123456789abcdef";
        for(std::uint32_t a=0;a<(1u<<20);++a) {const auto v=machine.bus().read8(a);memory.push_back(hex[v>>4]);memory.push_back(hex[v&15]);}
        std::cout<<"{\"outcome\":\""<<astervm::name(step.result.outcome)<<"\",\"retired\":"
                 <<(step.retired?"true":"false")<<",\"state\":"<<astervm::state_json(machine.state())
                 <<",\"memory_hex\":\""<<memory<<"\"}\n";
        return 0;
    } catch(const std::exception& error) {std::cerr<<"AsterVM step: "<<error.what()<<'\n';return 2;}
}
