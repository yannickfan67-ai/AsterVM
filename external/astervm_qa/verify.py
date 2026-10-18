"""Reproducible verification with evidence captured to disk."""
import json
from pathlib import Path
import platform
import subprocess
import sys
import unicorn

root=Path(__file__).resolve().parent
out=root/'evidence'
out.mkdir(exist_ok=True)
(out/'environment.json').write_text(json.dumps({'platform':platform.platform(),'python':sys.version,'unicorn':unicorn.__version__,'windows_native_tested':False,'astervm_core_tested':False},indent=2)+'\n')
commands=[('generate.log',[sys.executable,'generate_vectors.py']),('framework.log',[sys.executable,'-m','unittest','-v','test_harness']),('reference.log',[sys.executable,'runner.py','--label','reference','--report','evidence/reference-report.json','--',sys.executable,'unicorn_adapter.py'])]
for name,cmd in commands:
    p=subprocess.run(cmd,cwd=root,capture_output=True,text=True)
    text=p.stdout+p.stderr
    (out/name).write_text(text)
    print(text)
    if p.returncode:sys.exit(p.returncode)
fault_cmd=[sys.executable,'runner.py','--label','reference','--report','evidence/injected-fault-report.json','--',sys.executable,'fault_adapter.py']
p=subprocess.run(fault_cmd,cwd=root,capture_output=True,text=True)
report=json.loads((out/'injected-fault-report.json').read_text())
(out/'fault-injection.log').write_text(p.stdout+p.stderr+f'\nObserved exit code: {p.returncode}\nExpected exit code: 1\n')
assert p.returncode==1 and report['failed']==40, 'Fault injection was not detected'
print('Fault injection: 40/40 deliberate AX errors detected; runner exit code 1 verified')
