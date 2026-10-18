"""Author explicit golden states; never derive expected values from an emulator."""
import json
from pathlib import Path

REGS = 'ax bx cx dx si di bp sp cs ds es ss ip'.split()
CASES = []

def case(name, code, initial=None, changes=None, flags=2, mask=0xffff, memory=None, writes=None, reason='step'):
    regs = dict.fromkeys(REGS, 0)
    regs.update(sp=0x8000, ip=0x1000)
    regs.update(initial or {})
    before = dict(regs)
    before['flags'] = (initial or {}).get('flags', 2)
    regs.pop('flags', None)
    expected = dict(regs)
    expected['ip'] = (regs['ip'] + len(bytes.fromhex(code))) & 0xffff
    expected.update(changes or {})
    CASES.append(dict(id=name, mode='real16', code=code, initial=dict(registers=before, memory=memory or []),
        expected=dict(registers=expected, flags=flags, flags_mask=mask, memory=writes or [], reason=reason),
        budget=dict(instructions=1, timeout_seconds=5),
        failure_conditions=['register_mismatch','defined_flags_mismatch','memory_mismatch','unexpected_stop','adapter_error','timeout']))

case('mov_ax_imm16','b8 34 12',changes={'ax':0x1234})
case('mov_al_preserves_ah','b0 56',{'ax':0xabcd},{'ax':0xab56})
case('mov_ah_preserves_al','b4 12',{'ax':0xabcd},{'ax':0x12cd})
case('mov_bx_from_ax','89 c3',{'ax':0x4567},{'bx':0x4567})
case('xchg_ax_bx','93',{'ax':0x1111,'bx':0x2222},{'ax':0x2222,'bx':0x1111})
case('nop_preserves_state','90',{'ax':0x1234})
case('add_word_carry_zero','05 01 00',{'ax':0xffff},{'ax':0},0x57,0x8d5)
case('add_word_signed_overflow','05 01 00',{'ax':0x7fff},{'ax':0x8000},0x896,0x8d5)
case('add_byte_auxiliary_carry','04 01',{'ax':0x120f},{'ax':0x1210},0x12,0x8d5)
case('sub_word_borrow','2d 01 00',{'ax':0},{'ax':0xffff},0x97,0x8d5)
case('sub_word_signed_overflow','2d 01 00',{'ax':0x8000},{'ax':0x7fff},0x816,0x8d5)
case('cmp_equal_preserves_ax','3d 34 12',{'ax':0x1234},flags=0x46,mask=0x8d5)
case('inc_preserves_carry','40',{'ax':0xffff,'flags':3},{'ax':0},0x57,0x8d5)
case('dec_preserves_carry','48',{'ax':0,'flags':3},{'ax':0xffff},0x97,0x8d5)
case('xor_zero','31 c0',{'ax':0x1234},{'ax':0},0x46,0x8c5)
case('and_sign','25 00 80',{'ax':0xffff},{'ax':0x8000},0x86,0x8c5)
case('or_parity','0d 03 00',changes={'ax':3},flags=6,mask=0x8c5)
case('test_preserves_operand','a9 ff 00',{'ax':0x8000},flags=0x46,mask=0x8c5)
case('clc_clears_only_carry','f8',{'flags':0x403},flags=0x402)
case('stc_sets_only_carry','f9',flags=3)
case('cmc_toggles_carry','f5',{'flags':3},flags=2)
case('cld_clears_direction','fc',{'flags':0x402},flags=2)
case('std_sets_direction','fd',flags=0x402)
case('push_ax','50',{'ax':0x1234},{'sp':0x7ffe},writes=[{'address':0x7ffe,'hex':'34 12'}])
case('pop_bx','5b',changes={'bx':0x5678,'sp':0x8002},memory=[{'address':0x8000,'hex':'78 56'}])
case('jmp_short_forward','eb 02',changes={'ip':0x1004})
case('jz_taken','74 02',{'flags':0x42},{'ip':0x1004},0x42)
case('jz_not_taken','74 02')
case('jnz_taken','75 02',changes={'ip':0x1004})
case('loop_taken','e2 fe',{'cx':2},{'cx':1,'ip':0x1000})
case('loop_fallthrough','e2 fe',{'cx':1},{'cx':0})
case('mov_store_ds_offset','a3 00 20',{'ax':0xbeef},writes=[{'address':0x2000,'hex':'ef be'}])
case('mov_load_ds_offset','a1 00 20',changes={'ax':0xcafe},memory=[{'address':0x2000,'hex':'fe ca'}])
case('mov_load_nonzero_ds','a1 00 20',{'ds':0x100},{'ax':0x4321},memory=[{'address':0x3000,'hex':'21 43'}])
case('mov_load_bx_si','8b 00',{'bx':0x2000,'si':0x10},{'ax':0x9abc},memory=[{'address':0x2010,'hex':'bc 9a'}])
case('mov_bp_defaults_ss','8b 46 00',{'bp':0x200,'ss':0x100,'ds':0x300},{'ax':0x5678},memory=[{'address':0x1200,'hex':'78 56'}])
case('call_relative_pushes_return','e8 02 00',changes={'ip':0x1005,'sp':0x7ffe},writes=[{'address':0x7ffe,'hex':'03 10'}])
case('ret_pops_ip','c3',changes={'ip':0x1234,'sp':0x8002},memory=[{'address':0x8000,'hex':'34 12'}])
case('neg_word_minimum','f7 d8',{'ax':0x8000},{'ax':0x8000},0x887,0x8d5)
case('not_preserves_flags','f7 d0',{'ax':0x0f0f},{'ax':0xf0f0})

if __name__ == '__main__':
    target = Path(__file__).with_name('vectors.json')
    target.write_text(json.dumps({'schema':'astervm.single_step.v0.1','cases':CASES},ensure_ascii=False,indent=2)+'\n')
    print(f'Wrote {len(CASES)} explicit golden vectors')
