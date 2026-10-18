// cpu16.hpp — AsterVM 16 位实模式 CPU v0（ASTER-CPU-001，QWEN 35）
// 头文件内联实现，零外部依赖。黄金验收：acceptance/hello16/hello.bin vs expected.json。
#pragma once
#include "astervm/cpu.hpp"
#include <cstdio>
#include <string>

namespace astervm {

class Cpu16 {
public:
    Cpu16(IMemory& mem, IoDevice& io) : mem_(mem), io_(io) {}

    CpuState& st() { return st_; }
    uint64_t steps() const { return steps_; }

    void loadAndReset(const uint8_t* img, size_t n, uint16_t seg, uint16_t off) {
        for (size_t i = 0; i < n; ++i) mem_.write8(lin(seg,off)+uint32_t(i), img[i]);
        st_ = CpuState{}; st_.cs=seg; st_.ip=off; st_.flags=0x0002; st_.sp=0xFFFE;
        steps_=0;
    }

    StepResult step() {
        ++steps_;
        ipBase_ = lin(st_.cs, st_.ip);
        pc_ = ipBase_;
        uint8_t op = decodePrefixes();
        return dispatch(op);
    }

private:
    IMemory& mem_; IoDevice& io_; CpuState st_{};
    uint64_t steps_=0;
    uint32_t ipBase_=0, pc_=0;

    static uint32_t lin(uint16_t s, uint16_t o){ return (uint32_t(s)<<4)+o; }
    uint8_t fetch(){ return mem_.read8(pc_++); }
    uint16_t fetch16(){ uint16_t lo=fetch(), hi=fetch(); return uint16_t(lo|(hi<<8)); }
    int16_t sext8(uint8_t b){ return int16_t(int8_t(b)); }
    void commit(){ st_.ip = uint16_t(pc_ - ipBase_); }
    void jumpAbs(uint16_t cs,uint16_t ip){ st_.cs=cs; st_.ip=ip; } // 下条指令重算 ipBase_

    // legacy 前缀：v0 吞掉不改语义（0x66/0x67/0xF2/0xF3/段覆盖）。CS 段(0x3A/0x3B)是 opcode 本身。
    uint8_t decodePrefixes(){
        for(;;){
            uint8_t b=mem_.read8(pc_);
            switch(b){
            case 0x26: case 0x2E: case 0x36: case 0x3E: case 0x64: case 0x65:
            case 0x66: case 0x67: case 0xF2: case 0xF3: pc_++; continue;
            default: pc_++; return b;
            }
        }
    }

    struct Dec{ int mod,reg,rm; uint16_t disp; };
    Dec modrm(){ uint8_t m=fetch(); Dec d{m>>6,(m>>3)&7,m&7,0};
        if(d.mod==0&&d.rm==6) d.disp=fetch16();
        else if(d.mod==1) d.disp=uint16_t(sext8(fetch()));
        else if(d.mod==2) d.disp=fetch16();
        return d; }

    uint16_t& greg(int r){ switch(r){case 0:return st_.ax;case 1:return st_.cx;case 2:return st_.dx;case 3:return st_.bx;
        case 4:return st_.sp;case 5:return st_.bp;case 6:return st_.si;default:return st_.di;} }
    // 8 位寄存器视图：sel=0 低字节(AL..BL)；sel=1 高字节(AH..BH)，mod=3 rm>=4 时有效
    uint8_t getR8(int reg,int sel){ uint16_t v=greg(reg); return sel? uint8_t(v>>8):uint8_t(v); }
    void putR8(int reg,int sel,uint8_t b){ uint16_t& v=greg(reg); v = sel? uint16_t((v&0xFF)|(uint16_t(b)<<8)) : uint16_t((v&0xFF00)|b); }

    uint32_t ea(const Dec& d){ // 8086 16 位 EA；BP 基址用 SS，其余 DS
        if(d.mod==3) return 0;
        uint16_t e=0; bool ss=false;
        if(d.mod==0 && d.rm==6){ return lin(st_.ds,d.disp); }
        switch(d.rm){
        case 0:e=st_.bx+st_.si;break; case 1:e=st_.bx+st_.di;break;
        case 2:e=st_.bp+st_.si;ss=true;break; case 3:e=st_.bp+st_.di;ss=true;break;
        case 4:e=st_.si;break; case 5:e=st_.di;break;
        case 6:e=st_.bp;ss=true;break; case 7:e=st_.bx;break;}
        e=uint16_t(e+d.disp);
        return lin(ss?st_.ss:st_.ds,e);
    }
    uint8_t getRM8(const Dec& d){ return d.mod==3? getR8(d.rm, d.rm>=4?1:0) : mem_.read8(ea(d)); }
    uint16_t getRM16(const Dec& d){ return d.mod==3? greg(d.rm) : mem_.read16(ea(d)); }
    void putRM8(const Dec& d,uint8_t v){ if(d.mod==3) putR8(d.rm,d.rm>=4?1:0,v); else mem_.write8(ea(d),v); }
    void putRM16(const Dec& d,uint16_t v){ if(d.mod==3) greg(d.rm)=v; else mem_.write16(ea(d),v); }

    // ---- flags ----
    void flSet(uint16_t m,bool c){ if(c) st_.flags|=m; else st_.flags&=~m; }
    static bool pfOf(uint8_t x){ int c=0; for(int i=0;i<8;++i) if((x>>i)&1)++c; return !(c&1); }
    void logicF8(uint8_t r){ st_.flags&=~(F_CF|F_OF|F_AF); flSet(F_ZF,r==0); flSet(F_SF,r&0x80); flSet(F_PF,pfOf(r)); }
    void logicF16(uint16_t r){ st_.flags&=~(F_CF|F_OF|F_AF); flSet(F_ZF,r==0); flSet(F_SF,r&0x8000); flSet(F_PF,pfOf(uint8_t(r))); }
    void addF8(uint8_t a,uint8_t b,uint8_t r){ uint16_t s=uint16_t(a)+b;
        flSet(F_CF,s>0xFF); flSet(F_AF,(a^b^r)&0x10); flSet(F_OF,((a^r)&(b^r)&0x80)!=0);
        flSet(F_ZF,r==0); flSet(F_SF,r&0x80); flSet(F_PF,pfOf(r)); }
    void subF8(uint8_t a,uint8_t b,uint8_t r){ uint16_t s=uint16_t(a)-b;
        flSet(F_CF,s>0xFF); flSet(F_AF,(a^b^r)&0x10); flSet(F_OF,((a^b)&(a^r)&0x80)!=0);
        flSet(F_ZF,r==0); flSet(F_SF,r&0x80); flSet(F_PF,pfOf(r)); }
    void addF16(uint16_t a,uint16_t b,uint16_t r){ uint32_t s=uint32_t(a)+b;
        flSet(F_CF,s>0xFFFF); flSet(F_AF,(a^b^r)&0x10); flSet(F_OF,((a^r)&(b^r)&0x8000)!=0);
        flSet(F_ZF,r==0); flSet(F_SF,r&0x8000); flSet(F_PF,pfOf(uint8_t(r))); }
    void subF16(uint16_t a,uint16_t b,uint16_t r){ uint32_t s=uint32_t(a)-b;
        flSet(F_CF,s>0xFFFF); flSet(F_AF,(a^b^r)&0x10); flSet(F_OF,((a^b)&(a^r)&0x8000)!=0);
        flSet(F_ZF,r==0); flSet(F_SF,r&0x8000); flSet(F_PF,pfOf(uint8_t(r))); }
    void incF16(uint16_t a,uint16_t r){ uint16_t cf=st_.flags&F_CF;
        flSet(F_AF,(a&0xF)==0xF); flSet(F_OF,a==0x7FFF); flSet(F_ZF,r==0); flSet(F_SF,r&0x8000); flSet(F_PF,pfOf(uint8_t(r)));
        st_.flags=(st_.flags&~F_CF)|cf; }
    void decF16(uint16_t a,uint16_t r){ uint16_t cf=st_.flags&F_CF;
        flSet(F_AF,(a&0xF)==0); flSet(F_OF,a==0x8000); flSet(F_ZF,r==0); flSet(F_SF,r&0x8000); flSet(F_PF,pfOf(uint8_t(r)));
        st_.flags=(st_.flags&~F_CF)|cf; }

    uint16_t pop(){ uint16_t v=mem_.read16(lin(st_.ss,st_.sp)); st_.sp=uint16_t(st_.sp+2); return v; }
    void push(uint16_t v){ st_.sp=uint16_t(st_.sp-2); mem_.write16(lin(st_.ss,st_.sp),v); }

    bool cond(int cc){ switch(cc){
        case 0: return st_.flags&F_OF;  case 1: return !(st_.flags&F_OF);
        case 2: return st_.flags&F_CF;  case 3: return !(st_.flags&F_CF);
        case 4: return st_.flags&F_ZF;  case 5: return !(st_.flags&F_ZF);
        case 6: return st_.flags&F_PF;  case 7: return !(st_.flags&F_PF);
        case 8: return st_.flags&F_SF;  case 9: return !(st_.flags&F_SF);
        case 10:return st_.flags&F_CF;  case 11:return !(st_.flags&F_CF);
        case 12:return st_.flags&F_ZF;  case 13:return !(st_.flags&F_ZF);
        case 14:return st_.flags&F_PF;  default:return !(st_.flags&F_PF);} }

    // w=0..7: ADD OR ADC SBB AND SUB XOR CMP
    void alu8(int w,uint8_t a,uint8_t b,uint8_t& res){
        uint8_t r;
        switch(w){
        case 0: r=uint8_t(a+b); addF8(a,b,r); break;
        case 1: r=uint8_t(a|b); logicF8(r); break;
        case 2: { uint8_t c=uint8_t(st_.flags&F_CF); r=uint8_t(a+b+c); addF8(uint8_t(a+b),c,r); } break;
        case 3: { uint8_t c=uint8_t(st_.flags&F_CF); r=uint8_t(a-b-c); subF8(a,uint8_t(b+c),r); } break;
        case 4: r=uint8_t(a&b); logicF8(r); break;
        case 5: r=uint8_t(a-b); subF8(a,b,r); break;
        case 6: r=uint8_t(a^b); logicF8(r); break;
        default:r=uint8_t(a-b); subF8(a,b,r); break; }
        res=r; }
    void alu16(int w,uint16_t a,uint16_t b,uint16_t& res){
        uint16_t r;
        switch(w){
        case 0: r=uint16_t(a+b); addF16(a,b,r); break;
        case 1: r=uint16_t(a|b); logicF16(r); break;
        case 2: { uint16_t c=st_.flags&F_CF; r=uint16_t(a+b+c); addF16(uint16_t(a+b),c,r); } break;
        case 3: { uint16_t c=st_.flags&F_CF; r=uint16_t(a-b-c); subF16(a,uint16_t(b+c),r); } break;
        case 4: r=uint16_t(a&b); logicF16(r); break;
        case 5: r=uint16_t(a-b); subF16(a,b,r); break;
        case 6: r=uint16_t(a^b); logicF16(r); break;
        default:r=uint16_t(a-b); subF16(a,b,r); break; }
        res=r; }

    StepResult fault(const char* why,uint8_t op){
        std::fprintf(stderr,"[astervm] FAULT(%s) op=%02X at %04X:%04X\n",why,op,st_.cs,st_.ip);
        return StepResult::Fault; }

    StepResult dispatch(uint8_t op){
        // ---- 双操作数 ALU 家族 00..3F（排除 06/07/0E/0F/16/17/1E/1F/26/27/2E/2F/36/37/3E/3F）----
        if(op<=0x3F && (op&0x07)<=5 && ((op>>3)&3)!=3){
            int w=(op>>3)&7;
            switch(op&7){
            case 0: { Dec d=modrm(); uint8_t a=getRM8(d),b=getR8(d.reg,0),r; alu8(w,a,b,r); if(w!=7) putRM8(d,r); break; }
            case 1: { Dec d=modrm(); uint16_t a=greg(d.reg),b=getRM16(d),r; alu16(w,a,b,r); if(w!=7) greg(d.reg)=r; break; }
            case 2: { Dec d=modrm(); uint8_t a=getR8(d.reg,0),b=getRM8(d),r; alu8(w,a,b,r); if(w!=7) putR8(d.reg,0,r); break; }
            case 3: { Dec d=modrm(); uint16_t a=getRM16(d),b=greg(d.reg),r; alu16(w,a,b,r); if(w!=7) putRM16(d,r); break; }
            case 4: { uint8_t imm=fetch(); uint8_t r; alu8(w,uint8_t(st_.ax),imm,r); if(w!=7) st_.ax=uint16_t((st_.ax&0xFF00)|r); break; }
            default:{ uint16_t imm=fetch16(); uint16_t r; alu16(w,st_.ax,imm,r); if(w!=7) st_.ax=r; break; }
            }
            commit(); return StepResult::Ok;
        }

        switch(op){
        case 0x0F: case 0x3C: case 0x3D:
            return fault("unsupported opcode",op);
        case 0xD4: case 0xD5: case 0xD6: case 0xD7: case 0x27: case 0x2F:
            return fault("BCD not implemented",op);
        case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:{
            uint16_t a=greg(op&7), r=uint16_t(a+1); incF16(a,r); greg(op&7)=r; commit(); return StepResult::Ok;}
        case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F:{
            uint16_t a=greg(op&7), r=uint16_t(a-1); decF16(a,r); greg(op&7)=r; commit(); return StepResult::Ok;}
        case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
            push(greg(op&7)); commit(); return StepResult::Ok;
        case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F:
            greg(op&7)=pop(); commit(); return StepResult::Ok;
        case 0x60: { uint16_t vals[8]={st_.ax,st_.cx,st_.dx,st_.bx,st_.sp,st_.bp,st_.si,st_.di};
            for(int i=0;i<8;++i) push(vals[i]); commit(); return StepResult::Ok; }              // pusha
        case 0x61: { st_.di=pop(); st_.si=pop(); st_.bp=pop(); pop(); st_.bx=pop(); st_.dx=pop(); st_.cx=pop(); st_.ax=pop(); commit(); return StepResult::Ok; } // popa
        case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
        case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:{
            int8_t rel=int8_t(fetch()); uint32_t next=pc_;
            if(cond(op&0xF)) pc_=next+uint32_t(int32_t(rel));
            commit(); return StepResult::Ok; }
        case 0x80: case 0x81: case 0x82: case 0x83: { Dec d=modrm(); int w=d.reg;
            if(op==0x80||op==0x82){ uint8_t imm=fetch(); uint8_t a=getRM8(d),r; alu8(w,a,imm,r); if(w!=7) putRM8(d,r); }
            else { uint16_t imm=(op==0x83)?uint16_t(sext8(fetch())):fetch16(); uint16_t a=getRM16(d),r; alu16(w,a,imm,r); if(w!=7) putRM16(d,r); }
            commit(); return StepResult::Ok; }
        case 0x84: { Dec d=modrm(); uint8_t r=uint8_t(getRM8(d)&getR8(d.reg,0)); logicF8(r); commit(); return StepResult::Ok; }
        case 0x85: { Dec d=modrm(); uint16_t r=uint16_t(getRM16(d)&greg(d.reg)); logicF16(r); commit(); return StepResult::Ok; }
        case 0x86: { Dec d=modrm(); uint8_t a=getRM8(d),b=getR8(d.reg,0); putRM8(d,b); putR8(d.reg,0,a); commit(); return StepResult::Ok; }
        case 0x87: { Dec d=modrm(); uint16_t a=getRM16(d),b=greg(d.reg); putRM16(d,b); greg(d.reg)=a; commit(); return StepResult::Ok; }
        case 0x88: { Dec d=modrm(); putRM8(d,getR8(d.reg,0)); commit(); return StepResult::Ok; }
        case 0x89: { Dec d=modrm(); putRM16(d,greg(d.reg)); commit(); return StepResult::Ok; }
        case 0x8A: { Dec d=modrm(); putR8(d.reg,0,getRM8(d)); commit(); return StepResult::Ok; }
        case 0x8B: { Dec d=modrm(); greg(d.reg)=getRM16(d); commit(); return StepResult::Ok; }
        case 0x8C: { Dec d=modrm(); uint16_t seg = d.reg==0?st_.es:d.reg==1?st_.cs:d.reg==2?st_.ss:d.reg==3?st_.ds:0;
                     putRM16(d,seg); commit(); return StepResult::Ok; }
        case 0x8D: { Dec d=modrm(); greg(d.reg)=uint16_t(ea(d)-lin(st_.ds,0)); commit(); return StepResult::Ok; } // lea
        case 0x8E: { Dec d=modrm(); uint16_t v=getRM16(d);
                     switch(d.reg){case 0:st_.es=v;break;case 2:st_.ss=v;break;case 3:st_.ds=v;break;default:return fault("bad sreg",op);}
                     commit(); return StepResult::Ok; }
        case 0x8F: { Dec d=modrm(); putRM16(d,pop()); commit(); return StepResult::Ok; }          // pop r/m16
        case 0x90: commit(); return StepResult::Ok;                                               // nop
        case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:{
            uint16_t t=st_.ax; st_.ax=greg(op&7); greg(op&7)=t; commit(); return StepResult::Ok; } // xchg ax,r
        case 0x98: { int8_t l=int8_t(st_.ax); st_.ax=uint16_t(l); commit(); return StepResult::Ok; }  // cbw
        case 0x99: { st_.dx = (st_.ax&0x8000)?0xFFFF:0; commit(); return StepResult::Ok; }            // cwd
        case 0x9B: commit(); return StepResult::Ok;                                                   // wait/nop
        case 0xA0: case 0xA1: case 0xA2: case 0xA3: { uint16_t off=fetch16(); uint32_t la=lin(st_.ds,off);
            if(op==0xA0) st_.ax=uint16_t((st_.ax&0xFF00)|mem_.read8(la));
            else if(op==0xA1) st_.ax=mem_.read16(la);
            else if(op==0xA2) mem_.write8(la,uint8_t(st_.ax));
            else mem_.write16(la,st_.ax);
            commit(); return StepResult::Ok; }
        case 0xA4: case 0xA5: { int dir=(st_.flags&F_DF)?-1:1;
            if(op==0xA4){ uint8_t v=mem_.read8(lin(st_.ds,st_.si)); mem_.write8(lin(st_.es,st_.di),v);
                          st_.si=uint16_t(st_.si+dir); st_.di=uint16_t(st_.di+dir); }
            else { uint16_t v=mem_.read16(lin(st_.ds,st_.si)); mem_.write16(lin(st_.es,st_.di),v);
                   st_.si=uint16_t(st_.si+2*dir); st_.di=uint16_t(st_.di+2*dir); }
            commit(); return StepResult::Ok; }
        case 0xA8: { uint8_t imm=fetch(); uint8_t r=uint8_t(st_.ax&imm); logicF8(r); commit(); return StepResult::Ok; } // test al,imm
        case 0xA9: { uint16_t imm=fetch16(); uint16_t r=uint16_t(st_.ax&imm); logicF16(r); commit(); return StepResult::Ok; }
        case 0xAA: case 0xAB: { int dir=(st_.flags&F_DF)?-1:1;
            if(op==0xAA){ mem_.write8(lin(st_.es,st_.di),uint8_t(st_.ax)); st_.di=uint16_t(st_.di+dir); }
            else { mem_.write16(lin(st_.es,st_.di),st_.ax); st_.di=uint16_t(st_.di+2*dir); }
            commit(); return StepResult::Ok; }
        case 0xAC: case 0xAD: { int dir=(st_.flags&F_DF)?-1:1;
            if(op==0xAC){ st_.ax=uint16_t((st_.ax&0xFF00)|mem_.read8(lin(st_.ds,st_.si))); st_.si=uint16_t(st_.si+dir); }
            else { st_.ax=mem_.read16(lin(st_.ds,st_.si)); st_.si=uint16_t(st_.si+2*dir); }
            commit(); return StepResult::Ok; }
        case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7:{
            uint8_t imm=fetch(); putR8(op&7,0,imm); commit(); return StepResult::Ok; }
        case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBE: case 0xBF:{
            greg(op&7)=fetch16(); commit(); return StepResult::Ok; }
        case 0xC2: { st_.ip=pop(); st_.sp=uint16_t(st_.sp+fetch16()); commit(); return StepResult::Ok; } // ret imm16
        case 0xC3: st_.ip=pop(); commit(); return StepResult::Ok;                                        // ret
        case 0xCA: { uint16_t ip=pop(), cs=pop(); jumpAbs(cs,ip); return StepResult::Ok; }               // retf
        case 0xCC: if(!io_.onInt(3,st_)) return fault("int3 no handler",op); commit(); return StepResult::Ok;
        case 0xCD: { uint8_t n=fetch(); if(!io_.onInt(n,st_)) return fault("int no handler",op); commit(); return StepResult::Ok; }
        case 0xCE: if(!(st_.flags&F_OF)) { commit(); return StepResult::Ok; } if(!io_.onInt(4,st_)) return fault("into",op); commit(); return StepResult::Ok;
        case 0xCF: { uint16_t ip=pop(), cs=pop(), f=pop(); st_.ip=ip; st_.cs=cs; st_.flags=f; return StepResult::Ok; } // iret
        case 0xE8: { int16_t rel=int16_t(fetch16()); uint16_t next=uint16_t(pc_-ipBase_); push(next);
                     pc_=ipBase_+3+uint32_t(int32_t(rel)); commit(); return StepResult::Ok; }
        case 0xE9: { int16_t rel=int16_t(fetch16()); pc_=ipBase_+3+uint32_t(int32_t(rel)); commit(); return StepResult::Ok; }
        case 0xEA: { uint16_t ip=fetch16(), cs=fetch16(); jumpAbs(cs,ip); return StepResult::Ok; }
        case 0xEB: { int8_t rel=int8_t(fetch()); pc_=ipBase_+2+uint32_t(int32_t(rel)); commit(); return StepResult::Ok; }
        case 0xF4: commit(); return StepResult::Halted;                    // hlt
        case 0xF5: st_.flags&=~(F_CF|F_OF); commit(); return StepResult::Ok; // cmc
        case 0xF6: case 0xF7: { Dec d=modrm();
            if(d.reg==0){ if(op==0xF6){uint8_t r=uint8_t(~getRM8(d)); st_.flags&=~(F_CF|F_OF); st_.flags|=F_AF; flSet(F_ZF,r==0); flSet(F_SF,r&0x80); flSet(F_PF,pfOf(r)); putRM8(d,r);}
                          else {uint16_t r=uint16_t(~getRM16(d)); st_.flags&=~(F_CF|F_OF); st_.flags|=F_AF; flSet(F_ZF,r==0); flSet(F_SF,r&0x8000); flSet(F_PF,pfOf(uint8_t(r))); putRM16(d,r);} }
            else if(d.reg==1){ if(op==0xF6){uint8_t imm=fetch(); uint8_t r=uint8_t(getRM8(d)&imm); logicF8(r);} else {uint16_t imm=fetch16(); uint16_t r=uint16_t(getRM16(d)&imm); logicF16(r);} }
            else if(d.reg==3){ if(op==0xF6){uint8_t a=getRM8(d); uint8_t r=uint8_t(0-a); subF8(0,a,r); putRM8(d,r);}
                               else {uint16_t a=getRM16(d); uint16_t r=uint16_t(0-a); subF16(0,a,r); putRM16(d,r);} }
            else if(d.reg==4||d.reg==5){
                if(op==0xF6){ uint8_t src=getRM8(d); uint16_t p=uint16_t(uint8_t(st_.ax))*src;
                    if(d.reg==4){ st_.ax=p; flSet(F_CF|F_OF, uint8_t(p>>8)!=0); }
                    else { int16_t sp=int16_t(p); st_.ax=uint16_t(sp); flSet(F_CF|F_OF, uint8_t(sp>>8)!=uint8_t((sp&0x8000)?0xFF:0)); } }
                else { uint16_t src=getRM16(d);
                    if(d.reg==4){ uint32_t p=uint32_t(st_.ax)*src; st_.ax=uint16_t(p); st_.dx=uint16_t(p>>16); flSet(F_CF|F_OF,p>0xFFFF); }
                    else { int32_t p=int32_t(int16_t(st_.ax))*int32_t(int16_t(src)); st_.ax=uint16_t(p); st_.dx=uint16_t(uint32_t(p)>>16);
                           flSet(F_CF|F_OF, uint16_t(p>>16)!=(uint16_t(p)&0x8000?0xFFFF:0)); } } }
            else if(d.reg==6||d.reg==7){
                if(op==0xF6){ uint8_t src=getRM8(d); if(!src) return fault("div0",op);
                    uint16_t num=st_.ax; if(d.reg==6){ st_.ax=uint16_t((num%src)<<8)|(num/src); } else { st_.ax=uint16_t(num/src); st_.ax=uint16_t((st_.ax&0xFF)|(uint16_t(num%src)<<8)); } }
                else { uint16_t src=getRM16(d); if(!src) return fault("div0",op);
                    uint32_t num=(uint32_t(st_.dx)<<16)|st_.ax;
                    if(d.reg==6){ st_.ax=uint16_t(num/src); st_.dx=uint16_t(num%src); }
                    else { int32_t sn=int32_t(num); st_.ax=uint16_t(sn/int16_t(src)); st_.dx=uint16_t(sn%int16_t(src)); } } }
            else return fault("group3 reserved",op);
            commit(); return StepResult::Ok; }
        case 0xF8: st_.flags&=~F_CF; commit(); return StepResult::Ok;
        case 0xF9: st_.flags|=F_CF; commit(); return StepResult::Ok;
        case 0xFA: st_.flags&=~F_IF; commit(); return StepResult::Ok;
        case 0xFB: st_.flags|=F_IF; commit(); return StepResult::Ok;
        case 0xFC: st_.flags&=~F_DF; commit(); return StepResult::Ok;
        case 0xFD: st_.flags|=F_DF; commit(); return StepResult::Ok;
        case 0xFE: { Dec d=modrm(); if(d.reg>1) return fault("group4 reserved",op);
                     uint8_t a=getRM8(d); uint8_t r=uint8_t(d.reg==0?a+1:a-1);
                     uint16_t cf=st_.flags&F_CF;
                     flSet(F_AF,(a&0xF)==(d.reg==0?0xF:0)); flSet(F_OF,d.reg==0?(a==0x7F):(a==0x80));
                     flSet(F_ZF,r==0); flSet(F_SF,r&0x80); flSet(F_PF,pfOf(r)); st_.flags=(st_.flags&~F_CF)|cf;
                     putRM8(d,r); commit(); return StepResult::Ok; }
        case 0xFF: { Dec d=modrm();
            switch(d.reg){
            case 0:{uint16_t a=getRM16(d),r=uint16_t(a+1); incF16(a,r); putRM16(d,r); break;}
            case 1:{uint16_t a=getRM16(d),r=uint16_t(a-1); decF16(a,r); putRM16(d,r); break;}
            case 2: push(getRM16(d)); break;
            case 3: { uint32_t t=ea(d); jumpAbs(mem_.read16(t+2),mem_.read16(t)); return StepResult::Ok; } // jmp far [m]
            case 4: { uint32_t t=ea(d); push(uint16_t(pc_-ipBase_)); jumpAbs(mem_.read16(t+2),mem_.read16(t)); return StepResult::Ok; } // call far [m]
            case 5: st_.ip=getRM16(d); commit(); return StepResult::Ok; }                                  // jmp r/m16
            if(d.reg==6){ push(uint16_t(pc_-ipBase_)); st_.ip=getRM16(d); commit(); return StepResult::Ok; } // call r/m16
            return fault("group5 reserved",op); }
        default: return fault("unknown opcode",op);
        }
    }
};

} // namespace astervm
