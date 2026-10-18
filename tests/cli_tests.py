import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

CLI = str(Path(sys.argv.pop(1)).resolve())
ROOT = Path(__file__).resolve().parents[1]

class CliTests(unittest.TestCase):
    def run_cli(self, *args):
        return subprocess.run([CLI, *map(str, args)], capture_output=True, text=True, timeout=20)

    def test_demo_and_trace(self):
        with tempfile.TemporaryDirectory() as directory:
            trace, snapshot = Path(directory)/"trace.ndjson", Path(directory)/"state.json"
            result = self.run_cli("--demo", "--trace", trace, "--snapshot", snapshot)
            self.assertEqual(result.returncode, 0, result.stderr)
            state = json.loads(result.stdout)
            self.assertEqual(state["tty"], "AsterVM M0\r\n")
            self.assertEqual(state["steps"], 25)
            self.assertEqual(state, json.loads(snapshot.read_text()))
            records = [json.loads(line) for line in trace.read_text().splitlines()]
            self.assertEqual(len(records), 25)
            self.assertTrue(all(record["retired"] for record in records))
            self.assertEqual(records[-1]["outcome"], "halted")

    def test_raw_image_and_custom_entry(self):
        with tempfile.TemporaryDirectory() as directory:
            image = Path(directory)/"program.bin"
            image.write_bytes(bytes.fromhex("b8 34 12 f4"))
            result = self.run_cli("--load", image, "--entry", "1000:0020")
            self.assertEqual(result.returncode, 0, result.stderr)
            state = json.loads(result.stdout)["state"]
            self.assertEqual((state["eax"],state["cs"],state["eip"]),(0x1234,0x1000,0x24))

    def test_unsupported_is_not_success(self):
        with tempfile.TemporaryDirectory() as directory:
            image = Path(directory)/"unsupported.bin"
            image.write_bytes(b"\x0f")
            result = self.run_cli("--load",image)
            self.assertEqual(result.returncode,3)
            state = json.loads(result.stdout)
            self.assertEqual((state["stop"],state["steps"],state["state"]["eip"]),("unsupported",0,0x7c00))

    def test_budget_termination(self):
        with tempfile.TemporaryDirectory() as directory:
            image = Path(directory)/"loop.bin"
            image.write_bytes(b"\xeb\xfe")
            result = self.run_cli("--load",image,"--max-steps","17")
            self.assertEqual(result.returncode,5)
            self.assertEqual(json.loads(result.stdout)["steps"],17)

    def test_invalid_arguments(self):
        cases=[[],["--demo","--load","x"],["--demo","--max-steps","-1"],
               ["--demo","--entry","10000:0"],["--demo","--entry","wrong"],
               ["--demo","--max-steps","10000001"],["--load","/definitely/missing/astervm.bin"],
               ["--unknown"],["--load"]]
        for args in cases:
            with self.subTest(args=args):
                result=self.run_cli(*args)
                self.assertEqual(result.returncode,2)
                self.assertTrue(result.stderr)

    def test_empty_image_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            image=Path(directory)/"empty.bin"; image.write_bytes(b"")
            self.assertEqual(self.run_cli("--load",image).returncode,2)

    def test_expected_state_pass_and_mismatch(self):
        with tempfile.TemporaryDirectory() as directory:
            expectation=Path(directory)/"expected.json"
            command=[sys.executable,str(ROOT/"tools/verify.py"),"--cli",CLI,"--demo","--expect",str(expectation)]
            expectation.write_text(json.dumps({"stop":"halted","steps":25,"state":{"eflags":{"mask":2,"value":2}}}))
            result=subprocess.run(command,capture_output=True,text=True,timeout=20)
            self.assertEqual(result.returncode,0,result.stderr)
            expectation.write_text(json.dumps({"stop":"halted","steps":999}))
            result=subprocess.run(command,capture_output=True,text=True,timeout=20)
            self.assertEqual(result.returncode,1)
            expectation.write_text("{}")
            result=subprocess.run(command,capture_output=True,text=True,timeout=20)
            self.assertEqual(result.returncode,2)

    def test_trace_output_failure_is_error(self):
        with tempfile.TemporaryDirectory() as directory:
            self.assertEqual(self.run_cli("--demo","--trace",directory).returncode,2)

    def test_call_stack_image_and_trace(self):
        with tempfile.TemporaryDirectory() as directory:
            trace=Path(directory)/"stack.ndjson"
            result=self.run_cli("--load",ROOT/"examples/stack-call.bin","--trace",trace)
            self.assertEqual(result.returncode,0,result.stderr)
            snapshot=json.loads(result.stdout)
            self.assertEqual((snapshot["contract"],snapshot["stop"],snapshot["steps"]),("astervm.m0.v2","halted",8))
            state=snapshot["state"]
            self.assertEqual((state["eax"],state["ebx"],state["esp"],state["eip"]),(0,0x1234,0x8000,0x7c0a))
            records=[json.loads(line) for line in trace.read_text().splitlines()]
            self.assertEqual(len(records),8)
            self.assertEqual([w["address"] for w in records[2]["writes"]],[0x7ffe,0x7fff])
            self.assertEqual(records[6]["after"]["eip"],0x7c09)

if __name__=="__main__":
    unittest.main(verbosity=2)
