// golden_main.cpp — 黄金验收 CLI（ASTER-CLI-001）
// 契约：astervm_cli --load <bin> --expect <expected.json> [--trace trace.txt]
// 兼容旧式位置参数：<bin> <expected_tty.txt> [steps]
// 判定项：sha256、步数、终态寄存器、tty 输出。全部来自 expected.json（run_golden.py 生成）。
#include "../cpu16.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cctype>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>

using namespace astervm;

// ---------- 迷你 JSON 读取（零依赖，仅支持 expected.json 的扁平两层结构） ----------
struct Json {
    std::map<std::string,std::string> top;                       // "key" -> raw value string
    std::map<std::string,std::map<std::string,std::string>> obj; // nested objects by parent key
    bool parse(const std::string& path){
        std::ifstream in(path, std::ios::binary); if(!in) return false;
        std::stringstream ss; ss<<in.rdbuf(); std::string s=ss.str();
        size_t i=0; auto skipWs=[&]{ while(i<s.size()&&isspace((unsigned char)s[i])) ++i; };
        skipWs(); if(i>=s.size()||s[i]!='{') return false; ++i;
        while(true){
            skipWs(); if(i<s.size()&&s[i]=='}') break;
            if(s[i]!='"') return false;
            size_t e=s.find('"',i+1); std::string key=s.substr(i+1,e-i-1); i=e+1;
            skipWs(); if(s[i]!=':') return false; ++i; skipWs();
            if(s[i]=='"'){ size_t q=i+1; while(q<s.size()&&s[q]!='"'){ if(s[q]=='\\') ++q; ++q; }
                top[key]=s.substr(i+1,q-i-1); i=q+1; }
            else if(s[i]=='{'){ size_t depth=0,st=i; do{ if(s[i]=='{')++depth; else if(s[i]=='}')--depth; ++i; }while(i<s.size()&&depth);
                std::string body=s.substr(st,i-st); auto& m=obj[key];
                size_t j=1; while(true){ j=body.find('"',j); if(j==std::string::npos) break;
                    size_t k=body.find('"',j+1); if(k==std::string::npos) break;
                    std::string kk=body.substr(j+1,k-j-1); size_t c=body.find(':',k);
                    size_t v2=body.find('"',c); size_t v3=body.find('"',v2+1);
                    if(v2==std::string::npos||v3==std::string::npos) break;
                    m[kk]=body.substr(v2+1,v3-v2-1); j=v3+1; } }
            else { size_t v=i; while(v<s.size()&&s[v]!=','&&s[v]!='}'&&!isspace((unsigned char)s[v])) ++v;
                top[key]=s.substr(i,v-i); i=v; }
            skipWs(); if(i<s.size()&&s[i]==',') ++i;
        }
        return true;
    }
    static std::string deescape(const std::string& s){
        std::string o; for(size_t i=0;i<s.size();++i){ char c=s[i];
            if(c=='\\'&&i+1<s.size()){ char n=s[++i]; switch(n){case 'n':o+='\n';break;case 'r':o+='\r';break;
                case 't':o+='\t';break;case '"':o+='"';break;case '\\':o+='\\';break;default:o+=n;} }
            else o+=c; }
        return o; }
};

// ---------- sha256（FIPS 180-4，零外部依赖） ----------
static void sha256(const uint8_t* d,size_t n,std::string& hexOut){
    static const uint32_t K[64]={
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x1006dc01,
        0x183224ef,0x201c7124,0x229b56f6,0x23389e3d,0x2f3fe71b,0x2ce6b0c3,0x3a3be2ac,0x3e7e2e80,
        0x49ede9c2,0x54ee3f1d,0x682e4ff0,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa};
    uint32_t H[8]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
    auto rotr=[](uint32_t x,int n)->uint32_t{return (x>>n)|(x<<(32-n));};
    std::vector<uint8_t> msg(d,d+n); uint64_t bits=uint64_t(n)*8; msg.push_back(0x80);
    while(msg.size()%64!=56) msg.push_back(0);
    for(int i=7;i>=0;--i) msg.push_back(uint8_t(bits>>(i*8)));
    for(size_t off=0;off<msg.size();off+=64){
        uint32_t w[64]; for(int t=0;t<16;++t) w[t]=(msg[off+t*4]<<24)|(msg[off+t*4+1]<<16)|(msg[off+t*4+2]<<8)|msg[off+t*4+3];
        for(int t=16;t<64;++t){ uint32_t s0=rotr(w[t-15],7)^rotr(w[t-15],18)^(w[t-15]>>3),
                                    s1=rotr(w[t-2],17)^rotr(w[t-2],19)^(w[t-2]>>10);
                                  w[t]=w[t-16]+s0+w[t-7]+s1; }
        uint32_t a=H[0],b=H[1],c=H[2],dd=H[3],e=H[4],f=H[5],g=H[6],h=H[7];
        for(int t=0;t<64;++t){ uint32_t S1=rotr(e,6)^rotr(e,11)^rotr(e,25), ch=(e&f)^((~e)&g),
                                t1=h+S1+ch+K[t]+w[t], S0=rotr(a,2)^rotr(a,13)^rotr(a,22),
                                maj=(a&b)^(a&c)^(b&c), t2=S0+maj;
                              h=g;g=f;f=e;e=dd+t1;dd=c;c=b;b=a;a=t1+t2; }
        H[0]+=a;H[1]+=b;H[2]+=c;H[3]+=dd;H[4]+=e;H[5]+=f;H[6]+=g;H[7]+=h;
    }
    char buf[65]; for(int i=0;i<8;++i) std::sprintf(buf+i*8,"%08x",H[i]);
    hexOut=buf;
}

class BiosTeletype : public IoDevice {          // INT 0x10 AH=0E（对齐 run_golden.py 模型）
public:
    bool onInt(uint8_t n, CpuState& st) override {
        if(n==0x10){ uint8_t ah=uint8_t(st.ax>>8); if(ah==0x0E) tty.push_back(char(st.ax&0xFF)); return true; }
        if(n==0x18||n==0x20||n==0x21) return true; // 固件中断占位：吞掉
        return false;
    }
    std::string tty;
};

static uint16_t u16(const std::string& s){ return uint16_t(std::strtoul(s.c_str(),nullptr,16)); }

int main(int argc,char** argv){
    std::string binPath, expPath, tracePath; long wantSteps=-1;
    // 解析 --load/--expect/--trace 或旧式位置参数
    for(int i=1;i<argc;++i){
        if(!std::strcmp(argv[i],"--load")&&i+1<argc) binPath=argv[++i];
        else if(!std::strcmp(argv[i],"--expect")&&i+1<argc) expPath=argv[++i];
        else if(!std::strcmp(argv[i],"--trace")&&i+1<argc) tracePath=argv[++i];
        else if(binPath.empty()) binPath=argv[i];
        else if(expPath.empty()) expPath=argv[i];
    }
    if(binPath.empty()||expPath.empty()){ std::fprintf(stderr,"usage: %s --load hello.bin --expect expected.json [--trace t.txt]\n",argv[0]); return 2; }

    std::vector<uint8_t> img(512);
    { FILE* f=std::fopen(binPath.c_str(),"rb"); if(!f){ perror("open bin"); return 2; }
      size_t got=std::fread(img.data(),1,512,f); std::fclose(f);
      if(got!=512 || img[510]!=0x55 || img[511]!=0xAA){ std::fprintf(stderr,"bad boot image\n"); return 2; } }

    Json j; if(!j.parse(expPath)){ std::fprintf(stderr,"parse expected.json fail\n"); return 2; }
    std::string expSha=j.top.count("sha256")? j.top["sha256"]:"";
    if(j.top.count("steps")) wantSteps=std::atol(j.top["steps"].c_str());
    std::string expTty = j.top.count("expected_tty")? Json::deescape(j.top["expected_tty"])
                     : j.top.count("tty_output")? Json::deescape(j.top["tty_output"]) : "";
    // 若顶层没有，尝试从 obj 中找 final_regs
    auto& regs = j.obj.count("final_regs")? j.obj["final_regs"] : j.obj[""]; 

    // sha256 校验
    std::string sha; sha256(img.data(),img.size(),sha);
    bool okSha = expSha.empty() || sha==expSha;

    FlatMemory mem; BiosTeletype bios; Cpu16 cpu(mem,bios);
    cpu.loadAndReset(img.data(), img.size(), 0x07C0, 0x0000);   // ORG=0x7C00 => CS=07C0 IP=0000
    FILE* tr = tracePath.empty()? nullptr : std::fopen(tracePath.c_str(),"w");
    StepResult r=StepResult::Ok;
    for(long i=0;i<100000 && r==StepResult::Ok;++i){
        if(tr){ auto& s=cpu.st(); std::fprintf(tr,"%04X:%04X ax=%04X bx=%04X cx=%04X dx=%04X si=%04X di=%04X sp=%04X bp=%04X fl=%04X\n",
                s.cs,s.ip,s.ax,s.bx,s.cx,s.dx,s.si,s.di,s.sp,s.bp,s.flags); }
        r=cpu.step();
    }
    if(tr) std::fclose(tr);

    auto& fr = j.obj.count("final_regs")? j.obj["final_regs"]:regs;
    bool okRegs=true; CpuState& s=cpu.st();
    struct {const char*k;uint16_t v;} cur[]={{"ax",s.ax},{"bx",s.bx},{"cx",s.cx},{"dx",s.dx},{"si",s.si},{"di",s.di},
                                             {"sp",s.sp},{"bp",s.bp},{"ip",s.ip},{"cs",s.cs},{"ds",s.ds},{"es",s.es},{"ss",s.ss}};
    for(auto& c:cur){ if(fr.count(c.k)){ uint16_t e=u16(fr[c.k]); if(e!=c.v){ okRegs=false;
        std::fprintf(stderr,"reg %s: got %04X want %04X\n",c.k,c.v,e);} } }

    bool okSteps = wantSteps<0 || long(cpu.steps())==wantSteps;
    bool okHalt  = (r==StepResult::Halted);
    bool okTty   = (bios.tty==expTty);
    bool ok = okSha&&okTty&&okHalt&&okSteps&&okRegs;

    std::printf("sha256=%s (%s)\ntty=[%s]\nexpected_tty=[%s]\nsteps=%llu (want %ld)\nhalted=%d regs_ok=%d\nresult=%s\n",
        sha.c_str(), okSha?"OK":"MISMATCH", bios.tty.c_str(), expTty.c_str(),
        (unsigned long long)cpu.steps(), wantSteps, int(okHalt), int(okRegs), ok?"PASS":"FAIL");
    return ok?0:1;
}
