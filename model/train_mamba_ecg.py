#!/usr/bin/env python3
"""
train_mamba_ecg.py
===================
Trains TinyMamba on real clinical ECG heartbeats from the PhysioNet MIT-BIH
Arrhythmia Database (Records 100, 101, 200).

Validates:
  1. FP32 Next-Sample Waveform Prediction on Normal ('N') and Arrhythmia ('V') beats.
  2. Loss and Validation Accuracy over 150 epochs.
  3. Exports trained weights to model/weights/ for INT8 CFU hardware acceleration.
"""

import json
from pathlib import Path
import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F

from train_mamba import TinyMamba, export_weights, D_MODEL, N_LAYER, VOCAB_SIZE
from dataset_mitbih import get_mitbih_splits


def train_ecg_model(epochs=150, lr=0.012):
    torch.manual_seed(42)
    np.random.seed(42)

    print("=" * 75)
    print("      TRAINING TINYMAMBA ON CLINICAL ECG (MIT-BIH ARRHYTHMIA DATABASE)")
    print("=" * 75)

    # 1. Load real clinical dataset
    x_train, y_train, x_val, y_val, val_labels = get_mitbih_splits(seq_len=16)
    print(f"[Dataset] Train Samples: {len(x_train)} heartbeats | Val Samples: {len(x_val)} heartbeats")
    unique_labels, counts = np.unique(val_labels, return_counts=True)
    print(f"[Dataset] Validation Heartbeat Classes: {dict(zip(unique_labels, counts))}")

    # 2. Initialize Model
    model = TinyMamba(vocab_size=VOCAB_SIZE, d_model=D_MODEL, n_layer=N_LAYER)
    total_params = sum(p.numel() for p in model.parameters())
    print(f"[Model] Initialized TinyMamba with {total_params} parameters (Matches CFU Core ISA).")

    optimizer = torch.optim.AdamW(model.parameters(), lr=lr, weight_decay=1e-4)
    loss_fn = nn.CrossEntropyLoss()

    print("\n--- TRAINING PROGRESS ---")
    best_acc = 0.0
    for epoch in range(1, epochs + 1):
        model.train()
        optimizer.zero_grad()
        logits = model(x_train)
        loss = loss_fn(logits.view(-1, VOCAB_SIZE), y_train.view(-1))
        loss.backward()
        optimizer.step()

        if epoch % 25 == 0 or epoch == epochs:
            model.eval()
            with torch.no_grad():
                val_logits = model(x_val)
                val_loss = loss_fn(val_logits.view(-1, VOCAB_SIZE), y_val.view(-1)).item()
                preds = val_logits.argmax(dim=-1)
                acc = (preds == y_val).float().mean().item() * 100.0

            print(f"Epoch {epoch:3d}/{epochs} | Train Loss: {loss.item():.4f} | Val Loss: {val_loss:.4f} | Val Acc: {acc:.2f}%")

    # 3. Export weights to model/weights/
    weights_dir = Path(__file__).resolve().parent / "weights"
    export_weights(model, weights_dir)

    # 4. Clinical Demonstration: Compare Normal Beat vs. Arrhythmia Beat
    print("\n" + "=" * 75)
    print("        CLINICAL ECG INFERENCE DEMONSTRATION ON REAL PATIENT BEATS")
    print("=" * 75)
    model.eval()

    # Find first normal beat ('N') and first ventricular arrhythmia beat ('V')
    idx_normal = list(val_labels).index("N") if "N" in val_labels else 0
    idx_arrhythmia = list(val_labels).index("V") if "V" in val_labels else 1

    for label_name, idx in [("Normal Sinus Beat ('N')", idx_normal), ("Ventricular Ectopic Arrhythmia ('V')", idx_arrhythmia)]:
        inp = x_val[idx].tolist()
        tgt = y_val[idx].tolist()
        with torch.no_grad():
            logits = model(x_val[idx : idx + 1])[0].numpy()
        pred = list(np.argmax(logits, axis=-1))

        matches = sum(1 for p, t in zip(pred, tgt) if p == t)
        print(f"\nHeartbeat Type: {label_name}")
        print(f"  Input Waveform Tokens : {inp}")
        print(f"  Expected Next Tokens  : {tgt}")
        print(f"  Predicted Next Tokens : {pred}")
        print(f"  Waveform Accuracy     : {matches}/{len(tgt)} ({matches/len(tgt)*100:.1f}%)")

    print("=" * 75)
    return model


if __name__ == "__main__":
    train_ecg_model()
