"""One adapter process per vector, one JSON request/response, no shell parsing."""
import argparse
import json
from pathlib import Path
import subprocess
import sys

def validate(case):
    assert case['mode'] == 'real16'
    assert case['budget']['instructions'] == 1
    assert case['budget']['timeout_seconds'] > 0
    assert bytes.fromhex(case['code'])
    for name,value in case['initial']['registers'].items():
        assert isinstance(value,int) and 0 <= value <= 0xffff, name
    for value in case['expected']['registers'].values():
        assert isinstance(value,int) and 0 <= value <= 0xffff
    for block in case['initial']['memory']+case['expected']['memory']:
        assert 0 <= block['address'] < 0x100000
        assert block['address'] + len(bytes.fromhex(block['hex'])) <= 0x100000

def compare(case, actual):
    differences=[]
    if actual.get('reason') != case['expected']['reason']:
        differences.append({'field':'reason','expected':case['expected']['reason'],'actual':actual.get('reason')})
    for key,value in case['expected']['registers'].items():
        got=actual.get('registers',{}).get(key)
        if type(got) is not int or got != value:
            differences.append({'field':'registers.'+key,'expected':value,'actual':got})
    mask=case['expected']['flags_mask']
    flags=actual.get('registers',{}).get('flags')
    if type(flags) is not int or flags & mask != case['expected']['flags'] & mask:
        differences.append({'field':'flags','mask':mask,'expected':case['expected']['flags'] & mask,'actual':flags})
    # Adapter must return the complete 1 MiB physical RAM, not selected golden bytes.
    try:
        memory=bytes.fromhex(actual.get('memory_hex',''))
        if len(memory)!=0x100000:
            raise ValueError('RAM snapshot must be exactly 1 MiB')
        baseline=bytearray(0x100000)
        for block in case['initial']['memory']:
            raw=bytes.fromhex(block['hex']); baseline[block['address']:block['address']+len(raw)]=raw
        regs=case['initial']['registers']; start=(regs['cs'] << 4)+regs['ip']
        raw=bytes.fromhex(case['code']); baseline[start:start+len(raw)]=raw
        for block in case['expected']['memory']:
            raw=bytes.fromhex(block['hex']); baseline[block['address']:block['address']+len(raw)]=raw
        bad=[i for i,(a,b) in enumerate(zip(baseline,memory)) if a!=b]
        if bad:
            differences.append({'field':'memory','mismatch_count':len(bad),'first_addresses':bad[:16]})
    except (ValueError,TypeError) as exc:
        differences.append({'field':'memory_hex','error':str(exc)})
    return differences

def request_for(case):
    # Never disclose golden expected state to the implementation under test.
    return {key:case[key] for key in ['id','mode','code','initial','budget']}

def execute(case, command):
    try:
        proc=subprocess.run(command,input=json.dumps(request_for(case)),text=True,capture_output=True,timeout=case['budget']['timeout_seconds'])
        if proc.returncode:
            return [{'field':'adapter_exit','code':proc.returncode,'stderr':proc.stderr[-2000:]}]
        actual=json.loads(proc.stdout)
        return compare(case,actual)
    except (subprocess.TimeoutExpired,OSError,ValueError,TypeError,AttributeError) as exc:
        return [{'field':'adapter_error','error':str(exc)}]

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--vectors',default=str(Path(__file__).with_name('vectors.json')))
    parser.add_argument('--report',default='report.json')
    parser.add_argument('--label',required=True,choices=['reference','astervm'])
    parser.add_argument('adapter',nargs=argparse.REMAINDER)
    args=parser.parse_args()
    command=args.adapter
    if command[:1]==['--']: command=command[1:]
    if not command: parser.error('adapter command required after --')
    cases=json.loads(Path(args.vectors).read_text())['cases']
    ids=set(); results=[]
    for case in cases:
        validate(case)
        if case['id'] in ids: raise ValueError('duplicate case id')
        ids.add(case['id'])
        diff=execute(case,command)
        results.append({'id':case['id'],'pass':not diff,'differences':diff})
        print(('PASS ' if not diff else 'FAIL ')+case['id'])
    failed=sum(not r['pass'] for r in results)
    report={'label':args.label,'total':len(results),'passed':len(results)-failed,'failed':failed,'results':results}
    Path(args.report).write_text(json.dumps(report,indent=2)+'\n')
    print(f"{args.label}: {len(results)-failed}/{len(results)} passed")
    return 1 if failed else 0

if __name__=='__main__': sys.exit(main())
