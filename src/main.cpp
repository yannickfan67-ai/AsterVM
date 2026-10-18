#include "astervm/core.hpp"
#include <charconv>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace {
std::uint64_t number(const std::string& text, int base) {
    std::uint64_t value=0;
    const auto parsed=std::from_chars(text.data(),text.data()+text.size(),value,base);
    if(text.empty() || parsed.ec!=std::errc{} || parsed.ptr!=text.data()+text.size())
        throw std::invalid_argument("invalid unsigned numeric argument: "+text);
    return value;
}
std::vector<std::uint8_t> read_image(const std::string& path) {
    std::ifstream stream(path,std::ios::binary|std::ios::ate);
    if(!stream) throw std::runtime_error("cannot open image: "+path);
    const auto bytes=stream.tellg();
    if(bytes<=0 || bytes>(1<<20)) throw std::runtime_error("image must contain 1..1048576 bytes");
    stream.seekg(0);
    std::vector<std::uint8_t> result(static_cast<std::size_t>(bytes));
    if(!stream.read(reinterpret_cast<char*>(result.data()),bytes)) throw std::runtime_error("short image read");
    return result;
}
}
int main(int argc, char** argv) {
    try {
        std::string image,trace_path,snapshot_path;
        std::uint64_t budget=100000; std::uint16_t cs=0,ip=0x7c00;
        bool demo=false;
        for(int i=1;i<argc;++i) {
            const std::string arg=argv[i];
            const auto next=[&](){ if(i+1>=argc) throw std::invalid_argument("missing value for "+arg); return std::string(argv[++i]); };
            if(arg=="--demo") demo=true;
            else if(arg=="--load") image=next();
            else if(arg=="--trace") trace_path=next();
            else if(arg=="--snapshot") snapshot_path=next();
            else if(arg=="--max-steps") budget=number(next(),10);
            else if(arg=="--entry") {
                const auto text=next(); const auto colon=text.find(':');
                if(colon==std::string::npos) throw std::invalid_argument("entry must be hexadecimal CS:IP");
                const auto seg=number(text.substr(0,colon),16),off=number(text.substr(colon+1),16);
                if(seg>65535 || off>65535) throw std::invalid_argument("CS and IP must fit 16 bits");
                cs=std::uint16_t(seg); ip=std::uint16_t(off);
            } else if(arg=="--help") {
                std::cout<<"AsterVM M0 v0.1.0\nastervm_cli (--demo | --load IMAGE) [--entry 0000:7c00] "
                    "[--max-steps N] [--trace FILE.ndjson] [--snapshot FILE.json]\n"
                    "JSON expected-state checking: python tools/verify.py --help\n"; return 0;
            } else throw std::invalid_argument("unknown argument: "+arg);
        }
        if(demo==!image.empty()) throw std::invalid_argument("choose exactly one of --demo or --load");
        if(budget>10000000) throw std::invalid_argument("M0 step budget limit is 10000000");
        astervm::Machine machine; machine.reset(cs,ip);
        machine.bus().load(astervm::real_address(cs,ip),demo?astervm::demo_program():read_image(image));
        std::ofstream trace;
        if(!trace_path.empty()) {
            trace.open(trace_path,std::ios::binary|std::ios::trunc);
            if(!trace) throw std::runtime_error("cannot open trace output");
        }
        astervm::Interpreter16 core;
        const auto run=machine.run(core,budget,[&](const astervm::StepRecord& step){
            if(trace.is_open()) { trace<<astervm::trace_json(step)<<'\n'; if(!trace) throw std::runtime_error("trace write failed"); }
        });
        if(trace.is_open()) { trace.close(); if(!trace) throw std::runtime_error("trace flush failed"); }
        const auto snapshot=astervm::snapshot_json(machine,run);
        if(!snapshot_path.empty()) {
            std::ofstream output(snapshot_path,std::ios::binary|std::ios::trunc);
            if(!output || !(output<<snapshot<<'\n')) throw std::runtime_error("snapshot write failed");
            output.close(); if(!output) throw std::runtime_error("snapshot flush failed");
        }
        std::cout<<snapshot<<'\n';
        switch(run.stop) {
        case astervm::Stop::Halted:return 0;
        case astervm::Stop::Unsupported:return 3;
        case astervm::Stop::Fault:return 4;
        case astervm::Stop::Budget:return 5;
        }
    } catch(const std::exception& e) { std::cerr<<"AsterVM: "<<e.what()<<'\n'; return 2; }
    return 2;
}
