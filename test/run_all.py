#!/usr/bin/env python3
"""
Cross-Platform Master Regression Suite for Xmamba
Runs model references, C spike checks, RTL unit tests, and token regressions.
"""

from pathlib import Path
import shutil
import subprocess
import sys

def main():
    repo_root = Path(__file__).resolve().parent.parent
    hw_root = repo_root / "hw"
    py_exec = sys.executable

    print("==================================================================")
    print(" Starting Xmamba Complete Regression Test Suite")
    print(f" Python   : {py_exec}")
    print(f" Workspace: {repo_root}")
    print("==================================================================")

    steps = [
        ("Python Float Golden Model", [py_exec, "model/mamba_step.py"], repo_root),
        ("Python Fixed-Point Model", [py_exec, "model/mamba_fixed.py"], repo_root),
        ("Hardware Unit Tests", [py_exec, "-m", "unittest", "discover", "-s", "tests", "-v"], hw_root),
        ("Lookup Table Consistency", [py_exec, "-m", "mbmac.xmamba", "--check"], hw_root),
    ]

    has_iverilog = shutil.which("iverilog") is not None
    if has_iverilog:
        steps.append(("RTL Quick Verification", [py_exec, "-m", "mbmac.verify", "--quick"], hw_root))
        steps.append(("Token Pipeline Regression", [py_exec, "-m", "mbmac.verify_token"], hw_root))
    else:
        print("\n[Notice] iverilog not found in PATH; skipping RTL gate simulation.")

    passed = 0
    for name, cmd, cwd in steps:
        print(f"\n--- Running: {name} ---")
        res = subprocess.run(cmd, cwd=str(cwd))
        if res.returncode == 0:
            print(f">>> [PASS] {name}")
            passed += 1
        else:
            print(f">>> [FAIL] {name} (exit code {res.returncode})")
            sys.exit(res.returncode)

    print("\n==================================================================")
    print(f" SUMMARY: {passed}/{len(steps)} GATES PASSED")
    print("==================================================================")

if __name__ == "__main__":
    main()
