#!/usr/bin/env python3
"""
dataset_mitbih.py
=================
Loads real clinical ECG records from the PhysioNet MIT-BIH Arrhythmia Database.
Segments heartbeats around R-peaks and digitizes continuous Lead-II voltage waveforms
into 8 discrete amplitude bins (V=8, 3-bit ADC quantization for edge RISC-V IoT cores).
"""

from pathlib import Path
import numpy as np
import torch

DATA_DIR = Path(__file__).resolve().parent / "data" / "mitbih"


def extract_heartbeat_tokens(records=("100", "101", "200"), seq_len=16):
    """
    Extracts segmented heartbeats from MIT-BIH records and converts them to token sequences.
    Returns:
      sequences: list of integer arrays [seq_len], values in 0..7
      labels: list of clinical annotation strings ('N' for normal, 'V' for arrhythmia, etc.)
    """
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    all_sequences = []
    all_labels = []

    for r in records:
        sig_file = DATA_DIR / f"{r}_signal.npy"
        ann_file = DATA_DIR / f"{r}_ann_sample.npy"
        sym_file = DATA_DIR / f"{r}_ann_symbol.npy"

        if not (sig_file.exists() and ann_file.exists() and sym_file.exists()):
            raise FileNotFoundError(f"Record {r} not found in {DATA_DIR}. Please run download first.")

        sig = np.load(sig_file)
        r_peaks = np.load(ann_file)
        symbols = np.load(sym_file)

        # Robust normalization based on percentiles to avoid motion artifact outliers
        p1 = np.percentile(sig, 1.0)
        p99 = np.percentile(sig, 99.0)
        denom = max(p99 - p1, 1e-4)

        # 360 Hz: Window ~250 samples (~0.7s per heartbeat)
        half_win_pre = 100
        half_win_post = 150

        for r_idx, sym in zip(r_peaks, symbols):
            # Only consider standard beats
            if sym not in ["N", "V", "A", "L", "R", "/"]:
                continue

            start = r_idx - half_win_pre
            end = r_idx + half_win_post
            if start < 0 or end > len(sig):
                continue

            beat = sig[start:end]
            # Downsample to sequence length
            sub_idx = np.linspace(0, len(beat) - 1, seq_len).astype(int)
            beat_sub = beat[sub_idx]

            # 3-bit quantization (8 bins)
            norm = np.clip((beat_sub - p1) / denom, 0.0, 0.999)
            tokens = (norm * 8.0).astype(np.int64)

            all_sequences.append(tokens)
            all_labels.append(sym)

    return np.array(all_sequences, dtype=np.int64), np.array(all_labels)


def get_mitbih_splits(seq_len=16, test_ratio=0.2, seed=42):
    """
    Returns train and test tensors for next-token prediction:
    x_train, y_train, x_test, y_test, labels_test
    """
    seqs, labels = extract_heartbeat_tokens(seq_len=seq_len)
    rng = np.random.RandomState(seed)
    indices = rng.permutation(len(seqs))

    split = int(len(seqs) * (1.0 - test_ratio))
    train_idx = indices[:split]
    test_idx = indices[split:]

    train_seqs = seqs[train_idx]
    test_seqs = seqs[test_idx]
    test_labels = labels[test_idx]

    x_train = torch.tensor(train_seqs[:, :-1], dtype=torch.long)
    y_train = torch.tensor(train_seqs[:, 1:], dtype=torch.long)

    x_test = torch.tensor(test_seqs[:, :-1], dtype=torch.long)
    y_test = torch.tensor(test_seqs[:, 1:], dtype=torch.long)

    return x_train, y_train, x_test, y_test, test_labels


if __name__ == "__main__":
    x_tr, y_tr, x_te, y_te, labels = get_mitbih_splits()
    print(f"Extracted {len(x_tr)} training heartbeats, {len(x_te)} test heartbeats.")
    print(f"Sample Train Sequence Input : {x_tr[0].tolist()}")
    print(f"Sample Train Sequence Target: {y_tr[0].tolist()}")
    print(f"Test label distribution: {dict(zip(*np.unique(labels, return_counts=True)))}")
