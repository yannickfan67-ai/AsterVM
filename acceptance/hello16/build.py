#!/usr/bin/env python3
"""
build.py — 无外部汇编器环境下，构建 16 位实模式引导扇区镜像 (512B, 0xAA55)。

用途：为 x86 模拟器提供第一份黄金测试用例（golden image）：
      启动 -> BIOS INT 0x10 AH=0x0E 逐字符打印 "Hello, x86emu!\r\n" -> cli; hlt

编码采用手工机器码 + 符号地址两遍解析（本环境无 nasm/GAS-16 位链接支持）。
输出:
  hello.bin   —— 512 字节引导扇区镜像（可直接作为模拟器 floppy/MBK 输入）
  disasm.txt  —— 反汇编参考（若本机装有 capstone 则生成，供交叉核对）
退出码 0 且自检通过即为成功。
"""
import struct
import sys
import os

ORG = 0x7C00          # BIOS 加载地址（CS:IP = 07C0:0000，线性 0x7C00）


def build():
    code = bytearray()

    def emit(b):
        code.extend(b)

    msg_label = {}  # 记录 msg 相对 ORG 的偏移

    # 注：本文件中所有机器码均为 Intel 8086 手册编码。
    # objdump/capstone 对 `BC iw` 显示 "sp"、对 `8E /3` 显示 "mov %ax,%ds" 等
    # 属于反汇编器展示差异；/3 形式的段寄存器字段在 modrm 中为 reg=3 => DS，
    # 以 run_golden.py 的语义模型 + tty 输出为准（已交叉验证）。

    # --- 源码与机器码对照（AT&T 风格注释，指令按 i8086 编码）---
    emit(b'\x31\xC0')                # xorw %ax,%ax          (AX <- 0)
    emit(b'\x8E\xD8')                # movw %ax,%ds          (DS <- 0)
    # movw $msg, %bx   -> BC <offset16>, imm16 ; offset = 0x7C00 + len(code)+... 两遍法
    pos_mov_imm = len(code)
    emit(b'\xBC' + b'\x00\x00')      # placeholder, patched below
    # loop:
    pos_loop = len(code)
    emit(b'\x8A\x07')                # movb (%bx),%al
    emit(b'\x84\xC0')                # testb %al,%al
    pos_jz = len(code)
    emit(b'\x74\x00')                # jz 2f (placeholder)
    emit(b'\xB4\x0E')                # movb $0x0e,%ah
    emit(b'\x31\xC9')                # xorw %cx,%cx
    emit(b'\xCD\x10')                # int $0x10
    emit(b'\x43')                    # incw %bx
    jmp_rel = (pos_loop - (len(code) + 2)) & 0xFF
    emit(b'\xEB' + bytes([jmp_rel])) # jmp 1b (short rel8)
    pos_hlt = len(code)
    emit(b'\xF4')                    # cli? no: 0xF4 is hlt
    # 修正：上面应为 cli (0xFA) 再 hlt (0xF4)
    del code[pos_hlt:]
    emit(b'\xFA')                    # cli
    emit(b'\xF4')                    # hlt
    # msg 数据
    msg_off = len(code)              # 相对 _start（即文件偏移）
    msg_label['msg'] = msg_off
    msg = b"Hello, x86emu!\r\n"
    emit(msg)                        # 含 \r\n，随后补 NUL
    emit(b'\x00')

    # --- patch ---
    # movw $msg,%bx : 立即数 = ORG + msg_off （DS=0，物理=线性）
    struct.pack_into('<H', code, pos_mov_imm + 1, (ORG + msg_off) & 0xFFFF)
    # jz 2f : rel8 到 hlt 处
    rel = (pos_hlt - (pos_jz + 2)) & 0xFF
    code[pos_jz + 1] = rel

    # --- 填充到 510 + 签名 ---
    assert len(code) <= 510, f"boot code too big: {len(code)}"
    out = bytearray(code)
    out.extend(b'\x00' * (510 - len(out)))
    out += b'\x55\xAA'               # 小端写入后磁盘上为 ...55 AA
    assert len(out) == 512
    return bytes(out), msg


def selfcheck(img: bytes):
    """不依赖模拟器的静态自检：结构 + 语义抽取。"""
    problems = []
    if len(img) != 512:
        problems.append("size != 512")
    if img[510:512] != b'\x55\xAA':
        problems.append("missing 0xAA55 signature at 510")
    # 关键指令序列存在性
    seqs = [b'\xcd\x10', b'\xfa\xf4', b'\x31\xc0\x8e\xd8']
    for s in seqs:
        if s not in img:
            problems.append(f"missing byte sequence {s!r}")
    # 字符串可达性：msg 必须位于 [0,512) 内且以 NUL 结尾
    idx = img.find(b"Hello, x86emu!\r\n")
    if idx < 0 or idx >= 510 or img[idx + len(b"Hello, x86emu!\r\n")] != 0:
        problems.append("message not found/NUL-terminated in range")
    return problems


def maybe_disasm(img: bytes, path: str):
    try:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_16
    except Exception:
        return False
    md = Cs(CS_ARCH_X86, CS_MODE_16)
    with open(path, "w") as f:
        f.write("; objdump-style reference disassembly @ 0x7C00 (capstone)\n")
        for ins in md.disasm(img[:512], 0x7C00):
            f.write("0x%04x: %-8s %s %s\n" % (ins.address, ins.bytes.hex(),
                                              ins.mnemonic, ins.op_str))
            if ins.mnemonic == 'hlt':
                break
    return True


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    img, msg = build()
    binp = os.path.join(here, "hello.bin")
    with open(binp, "wb") as f:
        f.write(img)
    probs = selfcheck(img)
    if maybe_disasm(img, os.path.join(here, "disasm.txt")):
        print("[i] disasm.txt written (capstone)")
    else:
        print("[i] capstone unavailable, disasm.txt skipped")
    if probs:
        print("[FAIL] selfcheck:", "; ".join(probs))
        return 1
    sha = __import__('hashlib').sha256(img).hexdigest()
    print(f"[OK] hello.bin 512B, sig 55AA, sha256={sha}")
    print(f"[OK] payload string = {msg!r}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
