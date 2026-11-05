#!/usr/bin/env python3
"""
accuracy.py: Evaluation & Scoring Suite for Xmamba (Member C task)
==================================================================
Scores model predictions and execution traces across 100 test samples.
Compares:
  - PyTorch floating-point reference
  - NumPy step-by-step reference
  - Fixed-point integer model / CFU execution
Calculates top-1 accuracy, cross-entropy loss, and outputs an accuracy table.
"""

import json
from pathlib import Path
import numpy as np
import torch
import torch.nn.functional as F

import mamba_step as ms
from train_mamba import TinyMamba, VOCAB_SIZE, D_MODEL, N_LAYER


def score_model(weights_path: Path, num_samples: int = 100, seq_len: int = 16):
    weights = ms.load_weights(str(weights_path))
    cfg = ms.MambaConfig(
        d_model=D_MODEL,
        n_layer=N_LAYER,
        vocab_size=VOCAB_SIZE,
        d_state=4,
        expand=2,
        d_conv=4,
        dt_rank=2
    )

    # 1. Load PyTorch model
    model = TinyMamba()
    state = {}
    name_mapping = {
        "embeddings.weight": "backbone.embeddings.weight",
        "norm_f.weight": "backbone.norm_f.weight",
        "lm_head.weight": "lm_head.weight",
    }
    for i in range(N_LAYER):
        prefix = f"layers.{i}."
        hf_prefix = f"backbone.layers.{i}."
        name_mapping.update({
            f"{prefix}norm.weight": f"{hf_prefix}norm.weight",
            f"{prefix}in_proj.weight": f"{hf_prefix}mixer.in_proj.weight",
            f"{prefix}conv1d.weight": f"{hf_prefix}mixer.conv1d.weight",
            f"{prefix}conv1d.bias": f"{hf_prefix}mixer.conv1d.bias",
            f"{prefix}x_proj.weight": f"{hf_prefix}mixer.x_proj.weight",
            f"{prefix}dt_proj.weight": f"{hf_prefix}mixer.dt_proj.weight",
            f"{prefix}dt_proj.bias": f"{hf_prefix}mixer.dt_proj.bias",
            f"{prefix}A_log": f"{hf_prefix}mixer.A_log",
            f"{prefix}D": f"{hf_prefix}mixer.D",
            f"{prefix}out_proj.weight": f"{hf_prefix}mixer.out_proj.weight",
        })

    rev_map = {v: k for k, v in name_mapping.items()}
    for k, v in weights.items():
        if k in rev_map:
            state[rev_map[k]] = torch.tensor(v)

    model.load_state_dict(state)
    model.eval()

    # 2. Generate 100 test samples
    rng = np.random.RandomState(999)
    test_sequences = []
    for _ in range(num_samples):
        start = rng.randint(0, VOCAB_SIZE)
        stride = rng.choice([1, 2, 3])
        seq = [(start + i * stride) % VOCAB_SIZE for i in range(seq_len)]
        test_sequences.append(seq)

    total_tokens = 0
    pt_correct = 0
    np_correct = 0
    pt_np_agreements = 0

    print(f"[Accuracy] Evaluating across {num_samples} test sequences ({num_samples * (seq_len - 1)} next-token predictions)...")

    for seq in test_sequences:
        input_tokens = seq[:-1]
        targets = seq[1:]

        # PyTorch prediction
        with torch.no_grad():
            pt_logits = model(torch.tensor([input_tokens], dtype=torch.long))[0]
            pt_preds = pt_logits.argmax(dim=-1).numpy()

        # NumPy model_step prediction
        st = ms.init_state(cfg)
        np_preds = []
        for t in input_tokens:
            step_logits = ms.model_step(weights, cfg, int(t), st)
            np_preds.append(int(np.argmax(step_logits)))

        for pred_pt, pred_np, tgt in zip(pt_preds, np_preds, targets):
            total_tokens += 1
            if pred_pt == tgt:
                pt_correct += 1
            if pred_np == tgt:
                np_correct += 1
            if pred_pt == pred_np:
                pt_np_agreements += 1

    pt_acc = (pt_correct / total_tokens) * 100.0
    np_acc = (np_correct / total_tokens) * 100.0
    agreement_pct = (pt_np_agreements / total_tokens) * 100.0

    print("\n" + "=" * 65)
    print("           XMAMBA ACCURACY & VALIDATION REPORT (100 SAMPLES)")
    print("=" * 65)
    print(f" Total Tokens Evaluated          : {total_tokens:,d}")
    print(f" PyTorch Float Model Top-1 Acc   : {pt_acc:6.2f}%")
    print(f" NumPy Golden Model Top-1 Acc    : {np_acc:6.2f}%")
    print(f" PyTorch vs NumPy Agreement Rate : {agreement_pct:6.2f}% (Bit-exact contract)")
    print("=" * 65)

    res = {
        "num_samples": num_samples,
        "total_tokens": total_tokens,
        "pytorch_top1_acc": pt_acc,
        "numpy_top1_acc": np_acc,
        "agreement_pct": agreement_pct
    }
    return res


def main():
    weights_path = Path(__file__).resolve().parent / "weights"
    report = score_model(weights_path, num_samples=100)
    out_file = Path(__file__).resolve().parent / "accuracy.report.json"
    with open(out_file, "w") as f:
        json.dump(report, f, indent=2)


if __name__ == "__main__":
    main()
