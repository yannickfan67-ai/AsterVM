"""Dependency-free bootstrap build for GCC/Clang or an MSVC developer prompt."""
import os
from pathlib import Path
import shlex
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
build = root / "build"
build.mkdir(exist_ok=True)
compiler = shlex.split(os.environ.get("CXX", "cl" if os.name == "nt" else "g++"))
msvc = Path(compiler[0]).stem.lower() == "cl"
common = [str(root / "src" / name) for name in ("machine.cpp", "decode16.cpp", "execute16.cpp", "json.cpp")]
for target, main in (("astervm_cli", "src/main.cpp"), ("astervm_step", "src/step_bridge.cpp"), ("astervm_tests", "tests/core_tests.cpp")):
    output = build / (target + (".exe" if os.name == "nt" else ""))
    flags = ["/nologo", "/std:c++17", "/EHsc", "/W4", "/permissive-", "/I" + str(root / "include")] if msvc else ["-std=c++17", "-O2", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-I", str(root / "include")]
    command = compiler + flags + common + [str(root / main)]
    command += ["/Fe:" + str(output)] if msvc else ["-o", str(output)]
    print(shlex.join(command), flush=True)
    subprocess.run(command, cwd=build, check=True)
if "--test" in sys.argv:
    suffix = ".exe" if os.name == "nt" else ""
    subprocess.run([str(build / ("astervm_tests" + suffix))], check=True)
    subprocess.run([sys.executable, str(root / "tests/cli_tests.py"), str(build / ("astervm_cli" + suffix))], check=True)
