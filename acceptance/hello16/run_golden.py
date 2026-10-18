#!/usr/bin/env python3
"""
run_golden.py — 黄金参考执行器（golden reference runner）。

对 hello.bin 建模式 CPU 应有的行为，生成期望结果清单 expected.json：
  - 起点 CS:IP = 0x07C0:0x0000（线性 0x7C00），DS=ES=SS=0
  - 逐指令跟踪，遇到 INT 0x10 AH=0x0E 输出 AL 字符（teletype）
  - 终止条件：HLT（或步数上限保护）
模拟器实现完成后，用同一 hello.bin 跑出的 tty 输出与本文件比对即可验收。
纯 Python、零依赖，任何 Linux/Windows 主机可复现。
"""
import json
import os
import struct
import sys

ORG = 0x7C00
TTY_EXPECTED = "Hello, x86emu!\r\n"


def load_image(path):
    with open(path, "rb") as f:
        img = f.read()
    assert len(img) == 512, "boot image must be 512 bytes"
    assert img[510:512] == b"\x55\xAA", "bad boot signature"
    return img


def emulate(img):
    """极简语义模型：只识别 hello.S 实际用到的指令，用于推导期望输出与终态。"""
    mem = bytearray(1 << 20)          # 1MB 常规内存
    mem[ORG:ORG + 512] = img
    regs = dict(ax=0, bx=0, cx=0, dx=0, si=0, di=0, sp=0xFFFE, bp=0,
                ip=0, cs=0x07C0, ds=0, es=0, ss=0)
    flags = dict(zf=False, cf=False)
    tty = []
    steps = 0
    while True:
        steps += 1
        if steps > 100000:
            raise RuntimeError("step limit exceeded (should not happen)")
        ea = (regs["cs"] << 4) + regs["ip"]
        op = mem[ea]
        nxt = lambda n=1: (ea + n)
        if op == 0x31 and mem[ea + 1] == 0xC0:      # xor ax,ax
            regs["ax"] = 0; flags["zf"] = True; regs["ip"] += 2
        elif op == 0x8E and mem[ea + 1] == 0xD8:    # mov ds,ax
            regs["ds"] = regs["ax"]; regs["ip"] += 2
        elif op == 0xBC:                            # mov bx, imm16
            regs["bx"] = struct.unpack_from("<H", mem, ea + 1)[0]; regs["ip"] += 3
        elif op == 0x8A and mem[ea + 1] == 0x07:    # mov al,[bx]
            la = (regs["ds"] << 4) + regs["bx"]
            regs["ax"] = (regs["ax"] & 0xFF00) | mem[la]; regs["ip"] += 2
        elif op == 0x84 and mem[ea + 1] == 0xC0:    # test al,al
            v = regs["ax"] & 0xFF
            flags["zf"] = (v == 0); flags["cf"] = False; regs["ip"] += 2
        elif op == 0x74:                            # jz rel8
            r = struct.unpack_from("<b", mem, ea + 1)[0]
            regs["ip"] += (2 + r) if flags["zf"] else 2
        elif op == 0xB4:                            # mov ah, imm8
            regs["ax"] = (mem[ea + 1] << 8) | (regs["ax"] & 0xFF); regs["ip"] += 2
        elif op == 0x31 and mem[ea + 1] == 0xC9:    # xor cx,cx
            regs["cx"] = 0; flags["zf"] = True; regs["ip"] += 2
        elif op == 0xCD and mem[ea + 1] == 0x10:    # int 0x10
            ah = (regs["ax"] >> 8) & 0xFF
            if ah == 0x0E:
                tty.append(chr(regs["ax"] & 0xFF))
            regs["ip"] += 2
        elif op == 0x43:                            # inc bx
            regs["bx"] = (regs["bx"] + 1) & 0xFFFF
            flags["zf"] = regs["bx"] == 0; regs["ip"] += 1
        elif op == 0xEB:                            # jmp short rel8
            regs["ip"] += 2 + struct.unpack_from("<b", mem, ea + 1)[0]
        elif op == 0xFA:                            # cli
            regs["ip"] += 1
        elif op == 0xF4:                            # hlt -> end of golden run
            return "".join(tty), regs, steps
        else:
            raise RuntimeError(f"unknown opcode {op:#02x} at {ea:#x}")


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    binp = os.path.join(here, "hello.bin")
    if not os.path.exists(binp):
        print("[!] hello.bin missing, run build.py first"); return 1
    img = load_image(binp)
    tty, regs, steps = emulate(img)
    ok = tty == TTY_EXPECTED
    result = {
        "image": "hello.bin",
        "sha256": __import__("hashlib").sha256(img).hexdigest(),
        "start": {"cs_ip": "0x07C0:0x0000", "linear": hex(ORG)},
        "steps": steps,
        "final_regs": {k: hex(v) for k, v in regs.items()},
        "tty_output": tty,
        "expected_tty": TTY_EXPECTED,
        "verdict": "PASS" if ok else "FAIL",
    }
    out = os.path.join(here, "expected.json")
    with open(out, "w") as f:
        json.dump(result, f, indent=2, ensure_ascii=False)
    print(json.dumps(result, indent=2, ensure_ascii=False))
    print(f"[{'OK' if ok else 'FAIL'}] golden reference written to {out}")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
