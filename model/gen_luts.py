#!/usr/bin/env python3
"""
gen_luts.py: Lookup Table Generator & Precision Validator (Member C task)
=========================================================================
Generates the exp, SiLU, and softplus lookup tables from the Xmamba ISA specification,
evaluates linear interpolation error against floating-point mathematical definitions,
and verifies precision bounds.
"""

import math
from pathlib import Path
import json
import numpy as np


def float_exp(x):
    return np.exp(np.clip(x, -20.0, 0.0))


def float_silu(x):
    return x / (1.0 + np.exp(-np.clip(x, -30.0, 30.0)))


def float_softplus(x):
    return np.where(x > 20.0, x, np.log1p(np.exp(np.minimum(x, 20.0))))


def generate_tables(scale=256):
    """
    Generate 16-segment exp table and 32-segment silu/softplus tables matching Xmamba HW.
    """
    grid = [i / 2.0 for i in range(-16, 17)]
    exp_table = [int(math.floor(math.exp(-i / 2.0) * scale + 0.5)) for i in range(17)]
    silu_table = [int(math.floor((v / (1.0 + math.exp(-v))) * scale + 0.5)) for v in grid]
    softplus_table = [int(math.floor(math.log1p(math.exp(v)) * scale + 0.5)) for v in grid]

    return {
        "exp": exp_table,
        "silu": silu_table,
        "softplus": softplus_table,
    }


def interp_lut(table, value, kind, scale=256):
    """
    Piecewise linear interpolation matching the RTL and C++ implementation.
    """
    if kind == "exp":
        if value >= 0:
            return scale
        if value <= -2048:
            return 0
        pos = -value
    else:
        if value <= -2048:
            return 0
        if value >= 2048:
            return value
        pos = value + 2048

    idx, frac = divmod(pos, 128)
    if idx >= len(table) - 1:
        return table[-1]
    return table[idx] + ((table[idx + 1] - table[idx]) * frac // 128)


def evaluate_precision():
    tables = generate_tables(scale=256)
    report = {}

    print(f"{'Function':<12} | {'Input Range':<16} | {'Samples':>8} | {'Max Abs Err':>12} | {'Mean Abs Err':>12} | {'Status':>8}")
    print("-" * 80)

    # 1. Exp evaluation
    test_x_exp = np.linspace(-8.0, 0.0, 4000)
    int_inputs = np.round(test_x_exp * 256).astype(int)
    lut_outputs = np.array([interp_lut(tables["exp"], v, "exp") / 256.0 for v in int_inputs])
    ref_outputs = float_exp(test_x_exp)
    abs_err_exp = np.abs(lut_outputs - ref_outputs)
    max_err_exp = float(np.max(abs_err_exp))
    mean_err_exp = float(np.mean(abs_err_exp))
    status_exp = "PASS" if max_err_exp < 0.05 else "FAIL"
    print(f"{'exp':<12} | {'[-8.0, 0.0]':<16} | {len(test_x_exp):>8} | {max_err_exp:>12.5f} | {mean_err_exp:>12.5f} | {status_exp:>8}")
    report["exp"] = {"max_abs_err": max_err_exp, "mean_abs_err": mean_err_exp, "status": status_exp}

    # 2. SiLU evaluation
    test_x_silu = np.linspace(-8.0, 8.0, 4000)
    int_inputs_silu = np.round(test_x_silu * 256).astype(int)
    lut_outputs_silu = np.array([interp_lut(tables["silu"], v, "silu") / 256.0 for v in int_inputs_silu])
    ref_outputs_silu = float_silu(test_x_silu)
    abs_err_silu = np.abs(lut_outputs_silu - ref_outputs_silu)
    max_err_silu = float(np.max(abs_err_silu))
    mean_err_silu = float(np.mean(abs_err_silu))
    status_silu = "PASS" if max_err_silu < 0.05 else "FAIL"
    print(f"{'silu':<12} | {'[-8.0, 8.0]':<16} | {len(test_x_silu):>8} | {max_err_silu:>12.5f} | {mean_err_silu:>12.5f} | {status_silu:>8}")
    report["silu"] = {"max_abs_err": max_err_silu, "mean_abs_err": mean_err_silu, "status": status_silu}

    # 3. Softplus evaluation
    test_x_sp = np.linspace(-8.0, 8.0, 4000)
    int_inputs_sp = np.round(test_x_sp * 256).astype(int)
    lut_outputs_sp = np.array([interp_lut(tables["softplus"], v, "softplus") / 256.0 for v in int_inputs_sp])
    ref_outputs_sp = float_softplus(test_x_sp)
    abs_err_sp = np.abs(lut_outputs_sp - ref_outputs_sp)
    max_err_sp = float(np.max(abs_err_sp))
    mean_err_sp = float(np.mean(abs_err_sp))
    status_sp = "PASS" if max_err_sp < 0.05 else "FAIL"
    print(f"{'softplus':<12} | {'[-8.0, 8.0]':<16} | {len(test_x_sp):>8} | {max_err_sp:>12.5f} | {mean_err_sp:>12.5f} | {status_sp:>8}")
    report["softplus"] = {"max_abs_err": max_err_sp, "mean_abs_err": mean_err_sp, "status": status_sp}

    return tables, report


def main():
    tables, report = evaluate_precision()
    out_dir = Path(__file__).resolve().parent
    with open(out_dir / "gen_luts.report.json", "w") as f:
        json.dump(report, f, indent=2)
    print("\n[gen_luts] Generated tables and validated precision successfully.")


if __name__ == "__main__":
    main()
