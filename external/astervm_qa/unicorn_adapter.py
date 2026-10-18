"""External reference only. This is NOT an AsterVM CPU implementation."""
import json
import sys
from unicorn import Uc, UC_ARCH_X86, UC_MODE_16
from unicorn import x86_const as x

def run(req):
    uc=Uc(UC_ARCH_X86, UC_MODE_16)
    uc.mem_map(0,0x100000)
    for block in req['initial']['memory']:
        uc.mem_write(block['address'],bytes.fromhex(block['hex']))
    regs=req['initial']['registers']
    for name,value in regs.items():
        uc.reg_write(getattr(x,'UC_X86_REG_'+('EFLAGS' if name=='flags' else name.upper())),value)
    start=(regs['cs']<<4)+regs['ip']
    uc.mem_write(start,bytes.fromhex(req['code']))
    uc.emu_start(start,0xfffff,count=req['budget']['instructions'])
    result={name:uc.reg_read(getattr(x,'UC_X86_REG_'+('EFLAGS' if name=='flags' else name.upper()))) for name in regs}
    return {'reason':'step','registers':result,'memory_hex':bytes(uc.mem_read(0,0x100000)).hex()}

if __name__=='__main__':
    print(json.dumps(run(json.load(sys.stdin))))
