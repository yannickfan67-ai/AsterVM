"""Run the unmodified Genspark vectors against AsterVM, with explicit M0 scope."""
import argparse
import json
from pathlib import Path
import subprocess
import sys

PROFILE={
    "mov_ax_imm16","mov_al_preserves_ah","mov_ah_preserves_al","nop_preserves_state",
    "add_word_carry_zero","add_word_signed_overflow","add_byte_auxiliary_carry",
    "sub_word_borrow","sub_word_signed_overflow","cmp_equal_preserves_ax",
    "inc_preserves_carry","dec_preserves_carry","jmp_short_forward","jz_taken",
    "jz_not_taken","jnz_taken",
}
def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--bridge",required=True)
    parser.add_argument("--report",default="build/external-qa-report.json")
    args=parser.parse_args()
    root=Path(__file__).resolve().parents[1]
    report=Path(args.report).resolve(); report.parent.mkdir(parents=True,exist_ok=True)
    runner=root/"external/astervm_qa/runner.py"
    command=[sys.executable,str(runner),"--label","astervm","--report",str(report),"--",
             sys.executable,str(root/"tools/genspark_adapter.py"),"--bridge",str(Path(args.bridge).resolve())]
    completed=subprocess.run(command,capture_output=True,text=True,timeout=240)
    report.with_suffix(".log").write_text(completed.stdout+completed.stderr,encoding="utf-8")
    if completed.returncode not in (0,1):
        raise RuntimeError(completed.stderr or "external runner failed")
    data=json.loads(report.read_text())
    indexed={item["id"]:item for item in data["results"]}
    if not PROFILE.issubset(indexed) or data["total"]!=40:
        raise RuntimeError("unexpected external QA corpus")
    profile_failed=[name for name in sorted(PROFILE) if not indexed[name]["pass"]]
    unsupported=[]; unexpected=[]
    for item in data["results"]:
        if not item["pass"] and item["id"] not in PROFILE:
            is_unsupported=any(d.get("field")=="reason" and d.get("actual")=="unsupported" for d in item["differences"])
            (unsupported if is_unsupported else unexpected).append(item["id"])
    summary={"backend":"native AsterVM Interpreter16; no external emulator backend",
             "full_corpus_total":data["total"],"full_corpus_passed":data["passed"],
             "full_corpus_failed":data["failed"],"m0_profile_total":len(PROFILE),
             "m0_profile_passed":len(PROFILE)-len(profile_failed),"m0_profile_failed":profile_failed,
             "out_of_scope_unsupported":unsupported,"unexpected_out_of_scope_failures":unexpected}
    report.with_name("external-qa-summary.json").write_text(json.dumps(summary,indent=2)+"\n",encoding="utf-8")
    print(json.dumps(summary,indent=2))
    return 1 if profile_failed or unexpected else 0

if __name__=="__main__":
    try: sys.exit(main())
    except (OSError,ValueError,RuntimeError,subprocess.TimeoutExpired) as error:
        print(f"ERROR: {error}",file=sys.stderr);sys.exit(2)
