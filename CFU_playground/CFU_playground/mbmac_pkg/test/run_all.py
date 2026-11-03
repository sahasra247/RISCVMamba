#!/usr/bin/env python3
"""
Cross-Platform Master Regression Suite for Xmamba
Runs all test suites, table checkers, and token regressions.
"""

from pathlib import Path
import shutil
import subprocess
import sys

def main():
    pkg_root = Path(__file__).resolve().parent.parent
    py_exec = sys.executable

    print("==================================================================")
    print(" Starting Xmamba Complete Regression Test Suite")
    print(f" Python   : {py_exec}")
    print(f" Directory: {pkg_root}")
    print("==================================================================")

    steps = [
        ("Python Unit Tests", [py_exec, "-m", "unittest", "discover", "-s", "tests", "-v"]),
        ("Lookup Table Consistency", [py_exec, "-m", "mbmac.xmamba", "--check"]),
    ]

    has_iverilog = shutil.which("iverilog") is not None
    if has_iverilog:
        steps.append(("RTL Quick Verification", [py_exec, "-m", "mbmac.verify", "--quick"]))
        steps.append(("Token Pipeline Regression", [py_exec, "-m", "mbmac.verify_token"]))
    else:
        print("\n[Notice] iverilog not found in PATH; skipping RTL gate simulation.")

    passed = 0
    for name, cmd in steps:
        print(f"\n--- Running: {name} ---")
        res = subprocess.run(cmd, cwd=str(pkg_root))
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
