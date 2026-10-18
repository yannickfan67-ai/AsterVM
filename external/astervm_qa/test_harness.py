import copy
import sys
import unittest
from generate_vectors import CASES
from runner import compare, execute, request_for, validate

def synthetic(case):
    # Framework fixture only: intentionally constructed from the golden state.
    memory=bytearray(0x100000)
    for b in case['initial']['memory']:
        raw=bytes.fromhex(b['hex']); memory[b['address']:b['address']+len(raw)]=raw
    r=case['initial']['registers']; p=(r['cs']<<4)+r['ip']; raw=bytes.fromhex(case['code']); memory[p:p+len(raw)]=raw
    for b in case['expected']['memory']:
        raw=bytes.fromhex(b['hex']); memory[b['address']:b['address']+len(raw)]=raw
    return {'reason':'step','registers':dict(case['expected']['registers'],flags=case['expected']['flags']),'memory_hex':memory.hex()}

class HarnessTests(unittest.TestCase):
    def test_vectors_valid_unique(self):
        self.assertGreaterEqual(len(CASES),30)
        self.assertEqual(len({c['id'] for c in CASES}),len(CASES))
        for c in CASES: validate(c)
    def test_expected_never_sent(self):
        self.assertNotIn('expected',request_for(CASES[0]))
    def test_fixture_matches(self):
        self.assertEqual(compare(CASES[0],synthetic(CASES[0])),[])
    def test_register_fault_detected(self):
        a=synthetic(CASES[0]); a['registers']['ax']^=1
        self.assertTrue(compare(CASES[0],a))
    def test_unchanged_register_fault_detected(self):
        a=synthetic(CASES[0]); a['registers']['bx']=1
        self.assertTrue(compare(CASES[0],a))
    def test_unexpected_memory_write_detected(self):
        a=synthetic(CASES[0]); m=bytearray.fromhex(a['memory_hex']);m[0x9000]=1;a['memory_hex']=m.hex()
        self.assertTrue(compare(CASES[0],a))
    def test_defined_flag_fault_detected(self):
        a=synthetic(CASES[6]);a['registers']['flags']^=1
        self.assertTrue(compare(CASES[6],a))
    def test_undefined_af_ignored_for_xor(self):
        c=next(c for c in CASES if c['id']=='xor_zero');a=synthetic(c);a['registers']['flags']^=0x10
        self.assertEqual(compare(c,a),[])
    def test_reason_fault_detected(self):
        a=synthetic(CASES[0]);a['reason']='halt';self.assertTrue(compare(CASES[0],a))
    def test_missing_ram_rejected(self):
        a=synthetic(CASES[0]);a.pop('memory_hex');self.assertTrue(compare(CASES[0],a))
    def test_process_failure_detected(self):
        self.assertTrue(execute(CASES[0],[sys.executable,'-c','raise SystemExit(7)']))
    def test_bad_json_detected(self):
        self.assertTrue(execute(CASES[0],[sys.executable,'-c','print("bad")']))
    def test_timeout_detected(self):
        c=copy.deepcopy(CASES[0]);c['budget']['timeout_seconds']=0.05
        self.assertTrue(execute(c,[sys.executable,'-c','import time; time.sleep(1)']))

if __name__=='__main__':unittest.main(verbosity=2)
