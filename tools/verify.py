"""Check a partial JSON expectation against a real native AsterVM run.

A dict {"mask": N, "value": V} at an integer leaf compares defined bits.
Exit 0 = matches, 1 = mismatch, 2 = runner/input error.
"""
import argparse
import json
from pathlib import Path
import subprocess
import sys

def differences(actual, expected, path="snapshot"):
    if isinstance(expected, dict):
        if set(expected) == {"mask", "value"} and type(actual) is int:
            mask, value = expected["mask"], expected["value"]
            if type(mask) is not int or type(value) is not int or mask < 0 or value < 0:
                raise ValueError(f"{path}: mask/value must be nonnegative integers")
            return [] if actual & mask == value & mask else [f"{path}: masked mismatch"]
        if not isinstance(actual, dict):
            return [f"{path}: expected object"]
        result = []
        for key, value in expected.items():
            if key not in actual:
                result.append(f"{path}.{key}: missing")
            else:
                result.extend(differences(actual[key], value, f"{path}.{key}"))
        return result
    if type(expected) is not type(actual) or expected != actual:
        return [f"{path}: expected {expected!r}, actual {actual!r}"]
    return []

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cli", required=True)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--load")
    source.add_argument("--demo", action="store_true")
    parser.add_argument("--expect", required=True)
    parser.add_argument("--max-steps", default="100000")
    parser.add_argument("--entry", default="0000:7c00")
    parser.add_argument("--trace")
    args = parser.parse_args()
    expected = json.loads(Path(args.expect).read_text(encoding="utf-8"))
    if not isinstance(expected, dict) or not expected or "stop" not in expected:
        raise ValueError("expectation must be a nonempty object containing stop")
    command = [args.cli] + (["--demo"] if args.demo else ["--load", args.load])
    command += ["--max-steps", args.max_steps, "--entry", args.entry]
    if args.trace:
        command += ["--trace", args.trace]
    completed = subprocess.run(command, capture_output=True, text=True, timeout=60)
    if completed.returncode not in (0, 3, 4, 5):
        raise RuntimeError(f"native CLI failed ({completed.returncode}): {completed.stderr.strip()}")
    actual = json.loads(completed.stdout)
    exit_by_stop = {"halted": 0, "unsupported": 3, "fault": 4, "budget": 5}
    if actual.get("schema") != "astervm.snapshot.v1" or exit_by_stop.get(actual.get("stop")) != completed.returncode:
        raise RuntimeError("native CLI snapshot/exit contract mismatch")
    errors = differences(actual, expected)
    if errors:
        print("FAIL\n" + "\n".join(errors))
        return 1
    print("PASS: native AsterVM snapshot matches expectation")
    return 0

if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError, RuntimeError, subprocess.TimeoutExpired) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        sys.exit(2)
