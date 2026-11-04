#!/usr/bin/env python3
"""Pack Mamba weights into a binary blob for the C engine, and write the
NumPy reference outputs the engine is checked against on the target.

Outputs (in --out):
  model_cfg.h   compile-time sizes (MB_D_MODEL, MB_N_LAYER, ...)
  weights.bin   blob header + float32 tensors in the fixed order below
  ref.bin       test tokens + expected per-layer outputs, scan states h and logits
  blobs.S       .incbin wrapper that links both blobs into the ELF

Weights come from either
  --random                      random weights with the given sizes (until C exports the real ones)
  --weights DIR                 DIR/<hf name>.npy for every tensor + DIR/config.json (HF keys)

Blob layout (little-endian):
  header: 16 x uint32  {magic 'MAMB', version 1, d_model, n_layer, vocab, d_state,
                        expand, d_conv, dt_rank, tied, eps (float bits), 0...}
  tensors, float32, row-major, in this order:
    embeddings [V,d]
    per layer i: norm [d], in_proj [2Ed,d], conv1d.weight [Ed,K], conv1d.bias [Ed],
                 x_proj [R+2N,Ed], dt_proj.weight [Ed,R], dt_proj.bias [Ed],
                 A_log [Ed,N], D [Ed], out_proj [d,Ed]
    norm_f [d]
    lm_head [V,d]   (only if not tied)
"""

import argparse
import json
import os
import struct
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "..", "model"))
import mamba_step as ms  # noqa: E402
import mamba_fixed as mf  # noqa: E402

MAGIC = 0x424D414D          # 'MAMB'
VERSION = 1
LAYER_ORDER = ["norm.weight", "mixer.in_proj.weight", "mixer.conv1d.weight", "mixer.conv1d.bias",
               "mixer.x_proj.weight", "mixer.dt_proj.weight", "mixer.dt_proj.bias",
               "mixer.A_log", "mixer.D", "mixer.out_proj.weight"]


def load_npy_dir(path):
    with open(os.path.join(path, "config.json")) as f:
        c = json.load(f)
    cfg = ms.MambaConfig(d_model=c["hidden_size"], n_layer=c["num_hidden_layers"],
                         vocab_size=c["vocab_size"], d_state=c.get("state_size", 16),
                         expand=c.get("expand", 2), d_conv=c.get("conv_kernel", 4),
                         dt_rank=c.get("time_step_rank", 0), eps=c.get("layer_norm_epsilon", 1e-5))
    w = {f[:-4]: np.load(os.path.join(path, f)).astype(np.float32)
         for f in os.listdir(path) if f.endswith(".npy")}
    return cfg, w


def ordered_tensors(w, cfg):
    yield "backbone.embeddings.weight", w["backbone.embeddings.weight"]
    for i in range(cfg.n_layer):
        for name in LAYER_ORDER:
            yield ms.P(i, name), w[ms.P(i, name)]
    yield "backbone.norm_f.weight", w["backbone.norm_f.weight"]
    if "lm_head.weight" in w:
        yield "lm_head.weight", w["lm_head.weight"]


def write_weights(path, w, cfg):
    tied = "lm_head.weight" not in w
    eps_bits = struct.unpack("<I", struct.pack("<f", cfg.eps))[0]
    hdr = [MAGIC, VERSION, cfg.d_model, cfg.n_layer, cfg.vocab_size, cfg.d_state,
           cfg.expand, cfg.d_conv, cfg.dt_rank, int(tied), eps_bits] + [0] * 5
    with open(path, "wb") as f:
        f.write(struct.pack("<16I", *hdr))
        for _, t in ordered_tensors(w, cfg):
            f.write(np.ascontiguousarray(t, dtype="<f4").tobytes())
    return os.path.getsize(path)


def write_ref(path, w, cfg, tokens):
    """Per token: x after each layer [n_layer, d], scan state h after each
    layer [n_layer, Ed, N], then logits [V]. h is checked directly because the
    residual stream and the D*u term can hide scan errors in the outputs."""
    st = ms.init_state(cfg)
    with open(path, "wb") as f:
        f.write(struct.pack("<4I", MAGIC, VERSION, len(tokens), 0))
        f.write(np.asarray(tokens, "<i4").tobytes())
        for t in tokens:
            trace = []
            logits = ms.model_step(w, cfg, int(t), st, trace)
            for layer in trace[:-1]:
                f.write((layer["x_in"] + layer["out"]).astype("<f4").tobytes())
            for layer in trace[:-1]:
                f.write(layer["h"].astype("<f4").tobytes())
            f.write(logits.astype("<f4").tobytes())


# ------------------------------------------------------------ fixed point
# Blob 'MBQ8' (little-endian), sizes from model_cfg.h:
#   header 16 x uint32 {magic, version 1, d, n_layer, vocab, d_state, expand, d_conv,
#                       dt_rank, tied, eps_q32, 0...}
#   QLinear = int8 q[rows][cols rounded up to 4] then int8 sw[rows rounded up to 4]
#   embed QLinear [V,d]
#   per layer: norm i32[d], in_proj QL [2Ed,d], conv_w i32[Ed][K], conv_b i32[Ed],
#              x_proj QL [R+2N,Ed], dt_proj QL [Ed,R], dt_b i32[Ed], A i32[Ed][N],
#              D i32[Ed], out_proj QL [d,Ed]
#   norm_f i32[d];  head QLinear [V,d] only if not tied
# Ref 'MBQR': {magic, 1, ntok, 0}, tokens i32[ntok], then per token
#   x after each layer i32[L][d] (Q16), h after each layer i32[L][Ed][N] (Q20), logits i32[V] (Q16)
MAGIC_Q, MAGIC_QR = 0x3851424D, 0x5251424D        # 'MBQ8', 'MBQR'


def _pad4(n):
    return (n + 3) & ~3


def _qlinear_bytes(L):
    rows, cols = L.q.shape
    q = np.zeros((rows, _pad4(cols)), np.int8)
    q[:, :cols] = L.q
    sw = np.zeros(_pad4(rows), np.int8)
    assert L.sw.min() >= -128 and L.sw.max() <= 127
    sw[:rows] = L.sw
    return q.tobytes() + sw.tobytes()


def _i32(v):
    v = np.asarray(v)
    assert v.min() >= -(1 << 31) and v.max() < (1 << 31)
    return v.astype("<i4").tobytes()


def write_weights_q(path, Q, cfg, tied):
    hdr = [MAGIC_Q, 1, cfg.d_model, cfg.n_layer, cfg.vocab_size, cfg.d_state,
           cfg.expand, cfg.d_conv, cfg.dt_rank, int(tied), Q.eps_q32] + [0] * 5
    with open(path, "wb") as f:
        f.write(struct.pack("<16I", *hdr))
        f.write(_qlinear_bytes(Q.embed))
        for L in Q.layers:
            f.write(_i32(L.norm) + _qlinear_bytes(L.in_proj) + _i32(L.conv_w) + _i32(L.conv_b)
                    + _qlinear_bytes(L.x_proj) + _qlinear_bytes(L.dt_proj) + _i32(L.dt_b)
                    + _i32(L.A) + _i32(L.D) + _qlinear_bytes(L.out_proj))
        f.write(_i32(Q.norm_f))
        if not tied:
            f.write(_qlinear_bytes(Q.head))
    return os.path.getsize(path)


def write_ref_q(path, Q, cfg, tokens):
    st = mf.init_state_q(cfg)
    mf.OVERFLOWS.clear()
    with open(path, "wb") as f:
        f.write(struct.pack("<4I", MAGIC_QR, 1, len(tokens), 0))
        f.write(np.asarray(tokens, "<i4").tobytes())
        for t in tokens:
            trace = []
            logits = mf.model_step_q(Q, int(t), st, trace)
            for layer in trace:
                f.write(_i32(layer["x_out"]))
            for layer in trace:
                f.write(_i32(layer["h"]))
            f.write(_i32(logits))
    if mf.OVERFLOWS:
        print("warning: int32 overflows in the fixed-point reference:", mf.OVERFLOWS)


def write_cfg_h(path, cfg, tied):
    lines = ["/* Generated by sw/tools/export_weights.py. Do not edit. */",
             "#ifndef MODEL_CFG_H", "#define MODEL_CFG_H",
             f"#define MB_D_MODEL {cfg.d_model}", f"#define MB_N_LAYER {cfg.n_layer}",
             f"#define MB_VOCAB   {cfg.vocab_size}", f"#define MB_D_STATE {cfg.d_state}",
             f"#define MB_EXPAND  {cfg.expand}", f"#define MB_D_CONV  {cfg.d_conv}",
             f"#define MB_DT_RANK {cfg.dt_rank}", f"#define MB_TIED    {int(tied)}",
             "#define MB_D_INNER (MB_EXPAND * MB_D_MODEL)", "#endif", ""]
    with open(path, "w") as f:
        f.write("\n".join(lines))


def write_blobs_s(path, out_dir):
    a = os.path.abspath
    with open(path, "w") as f:
        f.write(f"""/* Generated by sw/tools/export_weights.py. Do not edit. */
    .section .rodata.mamba_blobs, "a"
    .balign 16
    .globl mamba_weights
mamba_weights:
    .incbin "{a(os.path.join(out_dir, 'weights.bin'))}"
    .balign 16
    .globl mamba_ref
mamba_ref:
    .incbin "{a(os.path.join(out_dir, 'ref.bin'))}"
""")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    src = ap.add_mutually_exclusive_group(required=True)
    src.add_argument("--random", action="store_true")
    src.add_argument("--weights", metavar="DIR")
    ap.add_argument("--d-model", type=int, default=256)
    ap.add_argument("--n-layer", type=int, default=4)
    ap.add_argument("--vocab", type=int, default=256)
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--tokens", type=int, default=8, help="number of reference tokens")
    ap.add_argument("--out", required=True)
    ap.add_argument("--fixed", action="store_true",
                    help="write the int8/fixed-point blob and bit-exact reference (model/mamba_fixed.py)")
    args = ap.parse_args()

    if args.random:
        cfg = ms.MambaConfig(d_model=args.d_model, n_layer=args.n_layer, vocab_size=args.vocab)
        w = ms.random_weights(cfg, args.seed)
    else:
        cfg, w = load_npy_dir(args.weights)
    ms.check_shapes(w, cfg)
    for i in range(cfg.n_layer):   # conv1d [Ed,1,K] -> [Ed,K]
        k = ms.P(i, "mixer.conv1d.weight")
        if w[k].ndim == 3:
            w[k] = w[k][:, 0, :]
    tied = "lm_head.weight" not in w

    os.makedirs(args.out, exist_ok=True)
    tokens = np.random.default_rng(args.seed + 100).integers(0, cfg.vocab_size, args.tokens)
    # model_step indexes conv1d weight as [Ed,1,K]; give it that view for the reference
    w_ref = dict(w)
    for i in range(cfg.n_layer):
        k = ms.P(i, "mixer.conv1d.weight")
        w_ref[k] = w[k][:, None, :]
    if args.fixed:
        Q = mf.quantize_model(w_ref, cfg)
        nbytes = write_weights_q(os.path.join(args.out, "weights.bin"), Q, cfg, tied)
        write_ref_q(os.path.join(args.out, "ref.bin"), Q, cfg, tokens)
    else:
        nbytes = write_weights(os.path.join(args.out, "weights.bin"), w, cfg)
        write_ref(os.path.join(args.out, "ref.bin"), w_ref, cfg, tokens)
    write_cfg_h(os.path.join(args.out, "model_cfg.h"), cfg, tied)
    write_blobs_s(os.path.join(args.out, "blobs.S"), args.out)
    print(f"export{' (fixed)' if args.fixed else ''}: d={cfg.d_model} L={cfg.n_layer} N={cfg.d_state} V={cfg.vocab_size} "
          f"params={ms.n_params(cfg, tied) / 1e6:.2f}M weights.bin={nbytes / 1024:.0f} KiB "
          f"ref tokens={len(tokens)}")


if __name__ == "__main__":
    main()
