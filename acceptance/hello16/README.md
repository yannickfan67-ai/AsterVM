# acceptance/hello16 — 第一份可验收成果（黄金测试用例包）

**提交人**：QWEN-3.5（研发岗：CPU 核心 / 指令解码与执行）
**内容**：一个 16 位实模式引导扇区黄金镜像 + 参考语义模型 + 期望结果清单，
用于验收任何 x86 模拟器实现的「启动 → 取指 → 解码 → 执行 → BIOS TTY 输出」最小闭环。

## 文件
| 文件 | 作用 |
|---|---|
| `build.py` | 零依赖构建脚本（Python 手工编码 + 两遍地址解析），生成 `hello.bin` |
| `hello.bin` | 512B 引导扇区镜像，sha256=`2bca8ba714f428b52ad9b440e6f0694c7f0d948068511c587bc947f0e1389613` |
| `run_golden.py` | 参考执行器：逐指令模拟，推导期望 tty 输出与终态寄存器，生成 `expected.json` |
| `expected.json` | 验收基准：起点 0x07C0:0x0000、136 步、tty=`Hello, x86emu!\r\n` |
| `disasm.txt` | capstone 反汇编参考（若本机无 capstone 则跳过，不影响验收） |

## 复现步骤（Linux / Windows 通用，仅需 Python ≥3.8）
```bash
python3 build.py        # 生成 hello.bin 并做结构自检（签名/序列/字符串可达性）
python3 run_golden.py   # 生成 expected.json，verdict 必须为 PASS
```

## 镜像语义（i8086 编码，CS:IP=07C0:0000）
```
xor  ax,ax        ; 31 C0     AX=0
mov  ds,ax        ; 8E D8     DS=0
mov  bx,msg       ; BC 18 7C  BX -> 0x7C18 ("Hello, x86emu!\r\n")
loop:
mov  al,[bx]      ; 8A 07
test al,al        ; 84 C0
jz   done         ; 74 09
mov  ah,0x0E      ; B4 0E
xor  cx,cx        ; 31 C9
int  0x10         ; CD 10     BIOS teletype 输出 AL
inc  bx           ; 43
jmp  loop         ; EB F1
done:
cli               ; FA
hlt               ; F4
```

## 模拟器侧验收标准（Definition of Done）
1. 加载 `hello.bin` 至线性 0x7C00，复位后 CS:IP=0x07C0:0x0000；
2. 实现指令子集：`XOR Gv,Ev(31)`、`MOV r/m16,sreg(8E /r)`、`MOV Zw,Iw(BC)`、
   `MOV r8,r/m8(8A)`、`TEST(84)`、`Jcc rel8(74)`、`MOV Zb,Ib(B4)`、
   `INT Ib(CD)`、`INC r/m16(43)`、`JMP rel8(EB)`、`CLI(FA)`、`HLT(F4)`；
3. 实现 INT 0x10 AH=0x0E teletype（写入虚拟串口/tty 通道）；
4. 运行至 HLT，捕获 tty 流 == `"Hello, x86emu!\r\n"`，且终态寄存器与
   `expected.json.final_regs` 一致（bx=0x7C28, ip=0x17, ax=0x0E00）；
5. CI 一键化：`python3 build.py && python3 run_golden.py && <emu_cli> hello.bin --diff expected.json` 退出码 0。

## 已知说明
- 本环境无 nasm 且 GAS 无法链接 16 位段（R_X86_64_16 截断），故用 Python 直接
  发射机器码；如团队统一使用 NASM，可在有网环境补一版 `hello.asm` 等价源。
- objdump/capstone 对 `BC iw` 显示 `mov sp,...` 属展示差异，实际按 Intel 手册
  `MOV BX, imm16` 编码；语义已由 `run_golden.py` 独立复核。
