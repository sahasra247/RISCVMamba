#!/usr/bin/env python3
"""
Stage and Token Trace Comparator (Lakshmi / Sahasra validation chain)
=====================================================================
Compares execution traces, intermediate layer stages, and logits between:
  - Python floating point reference (L1)
  - Python / C++ fixed-point integer model (L2/L3)
  - Spike instruction simulator (L4)
  - Renode / Verilator CFU co-simulation (L4)
  - Physical FPGA UART output (L4)

Usage:
  python test/compare.py <reference_dump> <dut_dump> [--tolerance 0]
"""

import argparse
import json
from pathlib import Path
import sys
from typing import List, Union

def load_data(filepath: Path) -> Union[dict, List[int]]:
    text = filepath.read_text(encoding="utf-8").strip()
    if text.startswith("{") or text.startswith("["):
        try:
            return json.loads(text)
        except json.JSONDecodeError:
            pass
    # Treat as line-separated numbers or whitespace-separated numbers
    values = []
    for line in text.splitlines():
        line = line.split("#")[0].strip()
        if not line:
            continue
        for token in line.replace(",", " ").split():
            try:
                values.append(int(token))
            except ValueError:
                try:
                    values.append(float(token))
                except ValueError:
                    pass
    return values

def compare_traces(ref_data, dut_data, tolerance: float = 0.0) -> bool:
    if isinstance(ref_data, dict) and isinstance(dut_data, dict):
        all_keys = sorted(set(ref_data.keys()) | set(dut_data.keys()))
        overall_match = True
        print(f"{'Stage/Tensor Name':<28} | {'Ref Count':>10} | {'DUT Count':>10} | {'Max Err':>10} | {'Status':>8}")
        print("-" * 75)
        for k in all_keys:
            if k not in ref_data:
                print(f"{k:<28} | {'MISSING':>10} | {len(dut_data[k]):>10} | {'-':>10} | FAIL")
                overall_match = False
                continue
            if k not in dut_data:
                print(f"{k:<28} | {len(ref_data[k]):>10} | {'MISSING':>10} | {'-':>10} | FAIL")
                overall_match = False
                continue
            r_vals = ref_data[k]
            d_vals = dut_data[k]
            if len(r_vals) != len(d_vals):
                print(f"{k:<28} | {len(r_vals):>10} | {len(d_vals):>10} | {'LEN DIFF':>10} | FAIL")
                overall_match = False
                continue
            max_err = max(abs(r - d) for r, d in zip(r_vals, d_vals)) if r_vals else 0
            status = "PASS" if max_err <= tolerance else "FAIL"
            if status == "FAIL":
                overall_match = False
            print(f"{k:<28} | {len(r_vals):>10} | {len(d_vals):>10} | {max_err:>10.4f} | {status}")
        return overall_match

    # Simple list comparison
    if len(ref_data) != len(dut_data):
        print(f"FAIL: Length mismatch. Reference has {len(ref_data)} items; DUT has {len(dut_data)} items.")
        return False

    max_err = 0.0
    sum_err = 0.0
    mismatch_idx = -1
    for idx, (r, d) in enumerate(zip(ref_data, dut_data)):
        err = abs(r - d)
        sum_err += err
        if err > max_err:
            max_err = err
        if err > tolerance and mismatch_idx == -1:
            mismatch_idx = idx

    mean_err = sum_err / len(ref_data) if ref_data else 0.0
    print(f"Total elements compared : {len(ref_data):,d}")
    print(f"Maximum absolute error  : {max_err:.4f}")
    print(f"Mean absolute error     : {mean_err:.4f}")
    print(f"Error tolerance         : {tolerance}")

    if max_err <= tolerance:
        print("\n>>> RESULT: PASS (All values within specified tolerance) <<<")
        return True
    else:
        print(f"\n>>> RESULT: FAIL (First mismatch at index {mismatch_idx}: reference={ref_data[mismatch_idx]}, DUT={dut_data[mismatch_idx]}) <<<")
        return False

def main():
    parser = argparse.ArgumentParser(description="Stage and Token Dump Comparator")
    parser.add_argument("reference", type=Path, help="Reference golden dump (JSON or plain text)")
    parser.add_argument("dut", type=Path, help="Device under test dump (JSON or plain text)")
    parser.add_argument("--tolerance", type=float, default=0.0, help="Allowable error tolerance (default: 0.0 for bit-exact)")
    args = parser.parse_args()

    if not args.reference.exists():
        print(f"Error: Reference file not found: {args.reference}", file=sys.stderr)
        sys.exit(1)
    if not args.dut.exists():
        print(f"Error: DUT file not found: {args.dut}", file=sys.stderr)
        sys.exit(1)

    ref = load_data(args.reference)
    dut = load_data(args.dut)
    ok = compare_traces(ref, dut, args.tolerance)
    sys.exit(0 if ok else 1)

if __name__ == "__main__":
    main()
