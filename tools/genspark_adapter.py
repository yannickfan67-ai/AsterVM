"""Translate Genspark's input-only JSON request to the native step fixture.

No expected result is received or used. The CPU and all RAM output come from
the compiled AsterVM bridge. No Unicorn or other emulation backend is used.
"""
import argparse
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--bridge",required=True)
    args=parser.parse_args()
    request=json.load(sys.stdin)
    if request["mode"]!="real16" or request["budget"]["instructions"]!=1:
        raise ValueError("adapter supports one real16 step")
    regs=request["initial"]["registers"]
    names=["ax","cx","dx","bx","sp","bp","si","di","es","cs","ss","ds","ip","flags"]
    if any(type(regs[k]) is not int or not 0<=regs[k]<=65535 for k in names):
        raise ValueError("register values must fit uint16")
    ram=bytearray(1<<20)
    for block in request["initial"]["memory"]:
        address=block["address"]; raw=bytes.fromhex(block["hex"])
        if type(address) is not int or not 0<=address<=len(ram) or len(raw)>len(ram)-address:
            raise ValueError("initial memory block outside RAM")
        ram[address:address+len(raw)]=raw
    code=bytes.fromhex(request["code"])
    address=((regs["cs"]<<4)+regs["ip"])&0xfffff
    if not code or len(code)>len(ram)-address:
        raise ValueError("code outside RAM")
    ram[address:address+len(code)]=code
    payload=b"AVMSTEP1"+struct.pack("<8I6HII",*[regs[k] for k in names[:8]],
        regs["es"],regs["cs"],regs["ss"],regs["ds"],0,0,regs["ip"],regs["flags"])+ram
    with tempfile.TemporaryDirectory() as directory:
        fixture=Path(directory)/"input.avm"; fixture.write_bytes(payload)
        result=subprocess.run([args.bridge,"--input",str(fixture)],capture_output=True,text=True,timeout=4)
    if result.returncode:
        raise RuntimeError(result.stderr.strip())
    native=json.loads(result.stdout)
    state=native["state"]
    result_regs={k:state["e"+k]&0xffff for k in names[:8]}
    result_regs.update({k:state[k] for k in ["es","cs","ss","ds"]})
    result_regs.update(ip=state["eip"]&0xffff,flags=state["eflags"]&0xffff)
    response={"reason":"step" if native["retired"] else native["outcome"],
              "registers":result_regs,"memory_hex":native["memory_hex"]}
    print(json.dumps(response))

if __name__=="__main__":
    try:main()
    except (KeyError,OSError,ValueError,RuntimeError,subprocess.TimeoutExpired) as error:
        print(str(error),file=sys.stderr)
        sys.exit(2)
