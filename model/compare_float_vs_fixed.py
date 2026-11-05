#!/usr/bin/env python3
"""
compare_float_vs_fixed.py
==========================
Demonstrates the real difference between 32-bit Floating Point (PyTorch/NumPy)
and INT8 Fixed-Point (CFU / Hardware baseline).

Shows:
  1. Exact Logit outputs from Float32 vs INT8 dequantized values.
  2. The Quantization Mean Absolute Error (MAE).
  3. Tokens that FLIP prediction due to quantization loss.
"""

import sys
from pathlib import Path
import numpy as np

# Ensure model directory is in sys.path
SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))

import mamba_fixed as mf
import mamba_step as ms

def main():
    weights_dir = SCRIPT_DIR / "weights"
    if not weights_dir.exists():
        print(f"Error: Weights directory not found at {weights_dir}")
        sys.exit(1)

    print("=" * 80)
    print("      FLOAT32 (Continuous) vs. INT8 FIXED-POINT (CFU Hardware Model)")
    print("=" * 80)

    # 1. Load trained model weights
    weights = ms.load_weights(str(weights_dir))
    cfg = ms.MambaConfig(d_model=4, n_layer=2, vocab_size=8, d_state=4, expand=2, d_conv=4, dt_rank=2)
    
    # 2. Quantize model to INT8 (matching CFU hardware precision)
    Q = mf.quantize_model(weights, cfg)

    # 3. Generate a sequence of 100 random evaluation tokens
    rng = np.random.default_rng(42)
    tokens = rng.integers(0, cfg.vocab_size, 100)

    # State vectors
    state_float = ms.init_state(cfg)
    state_fixed = mf.init_state_q(cfg)

    agree = 0
    diffs = []
    flips = []

    print("\n--- STEP-BY-STEP INFERENCE TRACE (First 5 Tokens & Any Mismatches) ---")
    print(f"{'Idx':<4} | {'Tok':<4} | {'Float32 Pred':<13} | {'INT8 Pred':<10} | {'Status':<8} | {'Logit MAE':<10}")
    print("-" * 65)

    for i, t in enumerate(tokens):
        # Float32 step
        lf = ms.model_step(weights, cfg, int(t), state_float)
        pred_f = int(np.argmax(lf))

        # INT8 Fixed-point step
        raw_lq = mf.model_step_q(Q, int(t), state_fixed)
        lq = mf.from_q(raw_lq, mf.QX)  # Dequantize back to float scale for direct comparison
        pred_q = int(np.argmax(lq))

        mae = float(np.mean(np.abs(lf - lq)))
        diffs.append(mae)

        is_match = (pred_f == pred_q)
        if is_match:
            agree += 1
        else:
            flips.append((i, t, pred_f, pred_q, mae, lf, lq))

        # Print first 5 tokens, plus whenever a quantization flip happens
        if i < 5 or not is_match:
            status = "MATCH" if is_match else "FLIPPED!"
            print(f"{i:<4} | {t:<4} | {pred_f:<13} | {pred_q:<10} | {status:<8} | {mae:.4f}")

    print("=" * 80)
    print("                     EVALUATION & ERROR REPORT")
    print("=" * 80)
    print(f"Total Evaluated Tokens     : {len(tokens)}")
    print(f"Top-1 Prediction Agreement : {agree} / {len(tokens)} ({agree/len(tokens)*100:.1f}%)")
    print(f"Tokens Flipped by INT8     : {len(flips)} / {len(tokens)} ({len(flips)/len(tokens)*100:.1f}%)")
    print(f"Mean Absolute Error (MAE)  : {np.mean(diffs):.4f}")
    print(f"Max Logit Discrepancy      : {np.max(diffs):.4f}")

    if flips:
        print("\n--- DETAILED INSPECTION OF A FLIPPED TOKEN ---")
        idx, tok, pf, pq, err, lf, lq = flips[0]
        print(f"At Token Index {idx} (Input Token: {tok}):")
        print(f"  Float32 Logits : {[round(float(x), 2) for x in lf]}")
        print(f"  INT8 Logits    : {[round(float(x), 2) for x in lq]}")
        print(f"  Float32 Argmax : Token {pf} (Logit: {lf[pf]:.2f})")
        print(f"  INT8 Argmax    : Token {pq} (Logit: {lq[pq]:.2f})")
        print(f"  Notice: The logits for Token {pf} and {pq} were so close that the")
        print(f"          8-bit quantization noise caused the hardware prediction to flip!")
    print("=" * 80)

if __name__ == "__main__":
    main()
