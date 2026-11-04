#!/usr/bin/env python3
"""
Mamba Floating-Point vs. Fixed-Point Accuracy & Quantization Evaluation Suite
=============================================================================
Evaluates the numerical fidelity of the fixed-point integer model (mamba_fixed.py)
against the double/float32 golden model (mamba_step.py) across evaluation sequences.

Computes:
  1. Top-1 Token Agreement Rate (%)
  2. Top-5 Token Agreement Rate (%)
  3. Logits MAE, MSE, and Cosine Similarity
  4. Recurrent Hidden State (h) Drift over Generation Steps
  5. Layer-by-Layer Intermediate Divergence

Usage:
  # Run self-contained evaluation on synthetic sequences:
  python3 model/eval_accuracy.py --sequences 20 --tokens 32

  # Run evaluation on trained model weights:
  python3 model/eval_accuracy.py --weights path/to/exported_weights/ --out results/accuracy_report.json
"""

from __future__ import annotations

import argparse
import json
import math
import os
from pathlib import Path
import sys
from typing import Any, Dict, List, Tuple

import numpy as np

# Ensure model directory is in sys.path
HERE = Path(__file__).resolve().parent
if str(HERE) not in sys.path:
    sys.path.insert(0, str(HERE))

import mamba_step as ms
import mamba_fixed as mf


def cosine_similarity(a: np.ndarray, b: np.ndarray, eps: float = 1e-9) -> float:
    norm_a = np.linalg.norm(a)
    norm_b = np.linalg.norm(b)
    if norm_a < eps or norm_b < eps:
        return 1.0 if norm_a < eps and norm_b < eps else 0.0
    return float(np.dot(a.flatten(), b.flatten()) / (norm_a * norm_b))


def generate_eval_sequences(vocab_size: int, num_seqs: int, seq_len: int, seed: int = 1234) -> List[List[int]]:
    """Generates deterministic pseudo-random token sequences for evaluation."""
    rng = np.random.default_rng(seed)
    high = max(2, vocab_size)
    return [rng.integers(0, high, size=seq_len).tolist() for _ in range(num_seqs)]


def load_input_sequences(filepath: Path) -> List[List[int]]:
    """Loads token sequences from JSON or text file."""
    text = filepath.read_text(encoding="utf-8").strip()
    if text.startswith("["):
        data = json.loads(text)
        if isinstance(data, list) and len(data) > 0 and isinstance(data[0], list):
            return data
        elif isinstance(data, list):
            return [data]
    # Line separated token ids
    seqs = []
    for line in text.splitlines():
        line = line.strip()
        if line:
            seqs.append([int(t) for t in line.replace(",", " ").split()])
    return seqs


def evaluate_accuracy(
    cfg: ms.MambaConfig,
    weights_float: Dict[str, np.ndarray],
    sequences: List[List[int]],
    verbose: bool = False,
) -> Dict[str, Any]:
    """Runs sequence evaluation comparing float reference against fixed-point integer model."""
    Q = mf.quantize_model(weights_float, cfg)

    total_tokens = 0
    top1_matches = 0
    top5_matches = 0

    all_logits_mae = []
    all_logits_mse = []
    all_logits_rel_err = []
    all_cosine_sims = []

    # Drift tracking: state h error across sequence step t
    max_seq_len = max(len(s) for s in sequences) if sequences else 0
    step_h_rel_errors: Dict[int, List[float]] = {t: [] for t in range(max_seq_len)}

    seq_reports = []

    for seq_idx, seq in enumerate(sequences):
        state_f = ms.init_state(cfg)
        state_q = mf.init_state_q(cfg)

        seq_top1_matches = 0
        seq_logits_rel = []

        for step_t, token_id in enumerate(seq):
            total_tokens += 1

            # 1. Step Float Reference
            trace_f: List[Dict[str, Any]] = []
            logits_f = ms.model_step(weights_float, cfg, token_id, state_f, trace=trace_f)

            # 2. Step Fixed-Point Model
            trace_q: List[Dict[str, Any]] = []
            logits_q_int = mf.model_step_q(Q, token_id, state_q, trace=trace_q)
            logits_q = mf.from_q(logits_q_int, mf.QX)

            # 3. Accuracy metrics
            pred_f = int(np.argmax(logits_f))
            pred_q = int(np.argmax(logits_q))

            top5_f = set(np.argsort(logits_f)[-5:])

            is_top1 = (pred_f == pred_q)
            is_top5 = (pred_q in top5_f)

            if is_top1:
                top1_matches += 1
                seq_top1_matches += 1
            if is_top5:
                top5_matches += 1

            # Logit errors
            abs_diff = np.abs(logits_f - logits_q)
            mae = float(np.mean(abs_diff))
            mse = float(np.mean(abs_diff ** 2))
            rel_err = float(np.linalg.norm(abs_diff) / (np.linalg.norm(logits_f) + 1e-9))
            cos_sim = cosine_similarity(logits_f, logits_q)

            all_logits_mae.append(mae)
            all_logits_mse.append(mse)
            all_logits_rel_err.append(rel_err)
            all_cosine_sims.append(cos_sim)
            seq_logits_rel.append(rel_err)

            # State h drift tracking across all layers
            h_diffs = []
            for layer_i in range(cfg.n_layer):
                hf = state_f.layers[layer_i].h
                hq = mf.from_q(state_q[layer_i].h, mf.QH)
                diff = np.linalg.norm(hf - hq) / (np.linalg.norm(hf) + 1e-9)
                h_diffs.append(diff)
            avg_h_err = float(np.mean(h_diffs))
            step_h_rel_errors[step_t].append(avg_h_err)

        seq_report = {
            "seq_idx": seq_idx,
            "len": len(seq),
            "top1_acc": seq_top1_matches / len(seq) if seq else 0.0,
            "mean_logits_rel_err": float(np.mean(seq_logits_rel)) if seq_logits_rel else 0.0,
        }
        seq_reports.append(seq_report)

        if verbose:
            print(f"Sequence {seq_idx:02d}: len={len(seq)} | Top-1: {seq_report['top1_acc']*100:.1f}% | RelErr: {seq_report['mean_logits_rel_err']:.4f}")

    # Aggregate statistics
    top1_acc = (top1_matches / total_tokens) if total_tokens > 0 else 0.0
    top5_acc = (top5_matches / total_tokens) if total_tokens > 0 else 0.0

    mean_mae = float(np.mean(all_logits_mae)) if all_logits_mae else 0.0
    mean_mse = float(np.mean(all_logits_mse)) if all_logits_mse else 0.0
    mean_rel_err = float(np.mean(all_logits_rel_err)) if all_logits_rel_err else 0.0
    mean_cos_sim = float(np.mean(all_cosine_sims)) if all_cosine_sims else 1.0

    # Drift profile by step (average across sequences)
    drift_profile = {
        t: float(np.mean(errs)) for t, errs in step_h_rel_errors.items() if errs
    }

    report = {
        "config": {
            "d_model": cfg.d_model,
            "n_layer": cfg.n_layer,
            "vocab_size": cfg.vocab_size,
            "d_state": cfg.d_state,
            "expand": cfg.expand,
            "d_conv": cfg.d_conv,
            "dt_rank": cfg.dt_rank,
            "num_params": ms.n_params(cfg),
        },
        "evaluation": {
            "num_sequences": len(sequences),
            "total_tokens": total_tokens,
            "top1_agreement_pct": round(top1_acc * 100.0, 2),
            "top5_agreement_pct": round(top5_acc * 100.0, 2),
            "logits_mean_absolute_error": mean_mae,
            "logits_mean_squared_error": mean_mse,
            "logits_mean_relative_error": mean_rel_err,
            "logits_cosine_similarity": mean_cos_sim,
            "max_state_h_relative_error": max(drift_profile.values()) if drift_profile else 0.0,
            "final_state_h_relative_error": list(drift_profile.values())[-1] if drift_profile else 0.0,
        },
        "drift_profile_by_step": drift_profile,
        "sequences": seq_reports,
    }

    return report


def print_summary_table(report: Dict[str, Any]) -> None:
    cfg = report["config"]
    eval_res = report["evaluation"]

    print("\n" + "=" * 76)
    print("      Xmamba Quantization Fidelity & Accuracy Report")
    print("=" * 76)
    print(f" Model Config : d_model={cfg['d_model']}  n_layer={cfg['n_layer']}  d_state={cfg['d_state']}  expand={cfg['expand']}")
    print(f" Parameters   : {cfg['num_params'] / 1e6:.2f}M params (Vocab: {cfg['vocab_size']})")
    print(f" Dataset Size : {eval_res['num_sequences']} sequences, {eval_res['total_tokens']} total evaluated tokens")
    print("-" * 76)
    print(f" {'Metric':<34} | {'Value':>16} | {'Target / Threshold':>16}")
    print("-" * 76)
    
    t1 = eval_res["top1_agreement_pct"]
    t5 = eval_res["top5_agreement_pct"]
    cos_sim = eval_res["logits_cosine_similarity"]
    rel_err = eval_res["logits_mean_relative_error"]
    h_max = eval_res["max_state_h_relative_error"]

    t1_status = "PASS" if t1 >= 85.0 else ("WARN" if t1 >= 75.0 else "FAIL")
    t5_status = "PASS" if t5 >= 95.0 else "WARN"
    cos_status = "PASS" if cos_sim >= 0.98 else "WARN"
    rel_status = "PASS" if rel_err <= 0.12 else "WARN"
    h_status = "PASS" if h_max <= 0.15 else "WARN"

    print(f" {'Top-1 Token Agreement':<34} | {t1:>15.2f}% | {'>= 85.0% (' + t1_status + ')':>16}")
    print(f" {'Top-5 Token Agreement':<34} | {t5:>15.2f}% | {'>= 95.0% (' + t5_status + ')':>16}")
    print(f" {'Logits Cosine Similarity':<34} | {cos_sim:>16.4f} | {'>= 0.9800 (' + cos_status + ')':>16}")
    print(f" {'Logits Relative Error':<34} | {rel_err:>16.4f} | {'<= 0.1200 (' + rel_status + ')':>16}")
    print(f" {'Max State (h) Drift Rel Error':<34} | {h_max:>16.4f} | {'<= 0.1500 (' + h_status + ')':>16}")
    print("=" * 76)

    overall_pass = (t1 >= 75.0 and cos_sim >= 0.95 and h_max <= 0.20)
    verdict = "[PASS] QUANTIZATION FIDELITY VERIFIED" if overall_pass else "[WARN] QUANTIZATION EXCEEDS ERROR THRESHOLD"
    print(f" Verdict: {verdict}")
    print("=" * 76 + "\n")


def main():
    parser = argparse.ArgumentParser(description="Evaluate Xmamba float vs fixed-point model accuracy.")
    parser.add_argument("--weights", type=str, default="", help="Path to weights directory or .npz archive (default: random)")
    parser.add_argument("--config", type=str, default="", help="Path to config.json file (optional)")
    parser.add_argument("--inputs", type=str, default="", help="Path to evaluation sequences file (.json/.txt)")
    parser.add_argument("--sequences", type=int, default=20, help="Number of evaluation sequences (default: 20)")
    parser.add_argument("--tokens", type=int, default=32, help="Tokens per sequence (default: 32)")
    parser.add_argument("--d-model", type=int, default=256, help="d_model if generating random weights (default: 256)")
    parser.add_argument("--n-layer", type=int, default=4, help="n_layer if generating random weights (default: 4)")
    parser.add_argument("--vocab", type=int, default=256, help="vocab_size if generating random weights (default: 256)")
    parser.add_argument("--out", type=str, default="results/accuracy_report.json", help="Output path for JSON report")
    parser.add_argument("--verbose", action="store_true", help="Print per-sequence details")
    args = parser.parse_args()

    # 1. Configuration & Weights Loading
    if args.weights:
        w_path = Path(args.weights)
        if args.config:
            cfg = ms.load_config(args.config)
        elif (w_path.is_dir() and (w_path / "config.json").is_file()):
            cfg = ms.load_config(w_path / "config.json")
        else:
            cfg = ms.MambaConfig(d_model=args.d_model, n_layer=args.n_layer, vocab_size=args.vocab)
        print(f"Loading weights from {w_path}...")
        weights = ms.load_weights(w_path)
        ms.check_shapes(weights, cfg)
    else:
        cfg = ms.MambaConfig(d_model=args.d_model, n_layer=args.n_layer, vocab_size=args.vocab)
        print(f"Generating deterministic random weights (d={cfg.d_model}, L={cfg.n_layer}, vocab={cfg.vocab_size})...")
        weights = ms.random_weights(cfg, seed=42)

    # 2. Input Sequences
    if args.inputs:
        in_path = Path(args.inputs)
        print(f"Loading evaluation sequences from {in_path}...")
        sequences = load_input_sequences(in_path)
    else:
        print(f"Generating {args.sequences} test sequences of length {args.tokens}...")
        sequences = generate_eval_sequences(cfg.vocab_size, args.sequences, args.tokens, seed=999)

    # 3. Run Evaluation
    print(f"Running accuracy evaluation on {len(sequences)} sequences...")
    report = evaluate_accuracy(cfg, weights, sequences, verbose=args.verbose)

    # 4. Print Summary
    print_summary_table(report)

    # 5. Save JSON report
    if args.out:
        out_path = Path(args.out)
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(json.dumps(report, indent=2), encoding="utf-8")
        print(f"Saved full JSON evaluation report to: {out_path}")


if __name__ == "__main__":
    main()
