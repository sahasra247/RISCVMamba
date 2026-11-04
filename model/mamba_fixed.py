"""Bit-exact fixed-point Mamba step: the integer arithmetic the RV32IM baseline
(and later Xmamba) must reproduce exactly.

Every value is an integer. "Qn" means a signed integer read as having n
fractional bits (real = int / 2**n). All stored values fit in int32;
intermediate products use int64. Rounding is round-half-up done by shifts,
rshr(v, s) = (v + 2**(s-1)) >> s, the same rule as the mb.racc RTL.

Formats (proposal for ISA v0, Decision 3 in Milestones.md)
----------------------------------------------------------
  residual x, xn, z, u_in, u, B, C, y     Q16  int32   (range +-32768, step 1.5e-5)
  linear-layer weights                     int8 per row, power-of-two row scale 2**-sw
  linear-layer inputs                      int8, power-of-two scale chosen per vector at run time
  accumulators                             int32 (exact int8 x int8 dot products, mb.mac4)
  conv1d weights and bias, norm weights,   Q16  int32
  dt_proj bias, A, D
  delta                                    Q24  int32   (small step sizes need the extra bits)
  exp(delta*A)                             Q30  int32   (in (0, 1]; decay close to 1 needs bits)
  scan state h                             Q20  int32
Nonlinear functions are piecewise-linear tables with linear interpolation:
  exp(x), x <= 0     range-reduced to 2**f, f in [0,1): EXP_SEGS segments, Q29 table
  sigmoid (SiLU)     |x| in [0, 16): SIG_SEGS segments, Q30 table, symmetric; 1.0 beyond
  softplus           exp(-|v|) * r(|v|), r(a) = log1p(e^-a)/e^-a, |v| in [0,16): SP_SEGS, Q30

Run:  python3 model/mamba_fixed.py   (table errors + fixed vs float on random weights)
"""

from __future__ import annotations

import math
from dataclasses import dataclass, field

import numpy as np

import mamba_step as ms

I64 = np.int64
INT32_MIN, INT32_MAX = -(1 << 31), (1 << 31) - 1

QX, QDT, QH, QDA = 16, 24, 20, 30           # see table above
EXP_SEGS, SIG_SEGS, SP_SEGS = 64, 128, 64
SIG_RANGE, SP_RANGE = 16, 16                 # table domains (powers of two)
LOG2E_Q30 = int(round(math.log2(math.e) * (1 << 30)))


# ------------------------------------------------------------------ helpers

def rshr(v, s):
    """Round-half-up arithmetic shift right by s (s may be an array; s <= 0 shifts left)."""
    v = np.asarray(v, I64)
    s = np.asarray(s, I64)
    pos = np.maximum(s, 1)
    right = (v + (I64(1) << (pos - 1))) >> pos
    left = v << np.maximum(-s, 0)
    return np.where(s > 0, right, left)


OVERFLOWS: dict[str, int] = {}


def chk32(name, v):
    """A value stored as int32: wrap like C (two's complement) and record any
    value that did not fit, since that means a format is too narrow."""
    v = np.asarray(v, I64)
    bad = int(np.count_nonzero((v < INT32_MIN) | (v > INT32_MAX)))
    if bad:
        OVERFLOWS[name] = OVERFLOWS.get(name, 0) + bad
    return ((v + (1 << 31)) & 0xFFFFFFFF) - (1 << 31)


def sat32(v):
    return np.clip(np.asarray(v, I64), INT32_MIN, INT32_MAX)


RACC_SH_MIN, RACC_SH_MAX = -31, 31          # mb.racc shift range


def bitlen(v: int) -> int:
    return int(v).bit_length()


def isqrt(v: int) -> int:
    return math.isqrt(int(v))


def to_q(x, q):
    return np.clip(np.rint(np.asarray(x, np.float64) * (1 << q)), INT32_MIN, INT32_MAX).astype(I64)


def from_q(v, q):
    return np.asarray(v, np.float64) / (1 << q)


# ------------------------------------------------------- lookup tables (PWL)

def _seg_bits(range_, segs, q_in):
    """Bits of the input below the segment index, for a domain [0, range_)."""
    b = q_in + int(math.log2(range_)) - int(math.log2(segs))
    assert (1 << int(math.log2(segs))) == segs and (1 << int(math.log2(range_))) == range_
    return b


def make_tables():
    xe = np.arange(EXP_SEGS + 1) / EXP_SEGS
    xs = np.arange(SIG_SEGS + 1) * SIG_RANGE / SIG_SEGS
    xp = np.arange(SP_SEGS + 1) * SP_RANGE / SP_SEGS
    r = np.where(xp == 0, math.log(2), np.log1p(np.exp(-xp)) / np.exp(-xp))
    return {
        "exp2": to_q(2.0 ** xe, 29),                 # [1, 2]
        "sig": to_q(1 / (1 + np.exp(-xs)), 30),      # [0.5, 1)
        "spr": to_q(r, 30),                          # [ln2, 1)
    }


T = make_tables()


def pwl(table, idx, frac, fb):
    """table[idx] + (table[idx+1]-table[idx]) * frac / 2**fb, rounded half up."""
    a, b = table[idx], table[idx + 1]
    return a + rshr((b - a) * frac, fb)


def exp_q(x24):
    """exp(x) for x <= 0. In: Q24. Out: Q30 in [0, 2**30]."""
    x = np.clip(np.asarray(x24, I64), INT32_MIN, 0)    # exp(-128) is 0 in Q30 anyway
    t = rshr(x * LOG2E_Q30, 30)                        # Q24, t <= 0
    n = t >> 24                                        # floor
    f = t - (n << 24)                                  # Q24 in [0, 1)
    fb = _seg_bits(1, EXP_SEGS, 24)
    v = pwl(T["exp2"], f >> fb, f & ((1 << fb) - 1), fb)   # Q29, 2**f
    sh = -(n + 1)                                      # Q29 * 2**n -> Q30
    return np.where(sh >= 62, 0, rshr(v, np.minimum(sh, 62)))


def sigmoid_q(x16):
    """In: Q16. Out: Q30."""
    x = np.asarray(x16, I64)
    a = np.abs(x)
    fb = _seg_bits(SIG_RANGE, SIG_SEGS, QX)
    inside = a < (SIG_RANGE << QX)
    ai = np.where(inside, a, 0)
    s = np.where(inside, pwl(T["sig"], ai >> fb, ai & ((1 << fb) - 1), fb), 1 << 30)
    return np.where(x < 0, (1 << 30) - s, s)


def silu_q(x16):
    """In/out: Q16."""
    x = np.asarray(x16, I64)
    return rshr(x * sigmoid_q(x), 30)


def softplus_q(v16):
    """softplus(v), torch threshold 20. In: Q16. Out: Q24 (saturates at int32)."""
    v = np.asarray(v16, I64)
    a = np.abs(v)
    e = exp_q(-(a << (QDT - QX)))                      # Q30, exp(-|v|)
    fb = _seg_bits(SP_RANGE, SP_SEGS, QX)
    inside = a < (SP_RANGE << QX)
    ai = np.where(inside, a, 0)
    r = np.where(inside, pwl(T["spr"], ai >> fb, ai & ((1 << fb) - 1), fb), T["spr"][-1])
    sp_neg = rshr(e * r, 60 - QDT)                     # Q24, softplus(-|v|)
    vq = v << (QDT - QX)
    out = np.where(v <= 0, sp_neg, vq + sp_neg)
    out = np.where(v > (20 << QX), vq, out)
    return np.minimum(out, INT32_MAX)


# --------------------------------------------------------- quantized weights

@dataclass
class QLinear:
    q: np.ndarray        # int8 values [rows, cols] (held as int64 here)
    sw: np.ndarray       # per-row shift: real weight = q * 2**-sw


def quant_linear(W) -> QLinear:
    W = np.asarray(W, np.float64)
    m = np.maximum(np.abs(W).max(axis=1), 1e-30)
    sw = np.floor(np.log2(127.0 / m)).astype(I64)      # largest s with max * 2**s <= 127
    q = np.clip(np.rint(W * np.exp2(sw)[:, None]), -127, 127).astype(I64)
    return QLinear(q, sw)


@dataclass
class QLayer:
    norm: np.ndarray
    in_proj: QLinear
    conv_w: np.ndarray   # [Ed, K] Q16
    conv_b: np.ndarray
    x_proj: QLinear
    dt_proj: QLinear
    dt_b: np.ndarray
    A: np.ndarray        # [Ed, N] Q16, = -exp(A_log)
    D: np.ndarray
    out_proj: QLinear


@dataclass
class QModel:
    cfg: ms.MambaConfig
    embed: QLinear       # rows = tokens; also the tied output head
    layers: list
    norm_f: np.ndarray
    head: QLinear
    eps_q32: int


def quantize_model(w, cfg: ms.MambaConfig) -> QModel:
    layers = []
    for i in range(cfg.n_layer):
        g = lambda n: np.asarray(w[ms.P(i, n)], np.float64)
        cw = g("mixer.conv1d.weight")
        layers.append(QLayer(
            norm=to_q(g("norm.weight"), QX),
            in_proj=quant_linear(g("mixer.in_proj.weight")),
            conv_w=to_q(cw.reshape(cw.shape[0], -1), QX),
            conv_b=to_q(g("mixer.conv1d.bias"), QX),
            x_proj=quant_linear(g("mixer.x_proj.weight")),
            dt_proj=quant_linear(g("mixer.dt_proj.weight")),
            dt_b=to_q(g("mixer.dt_proj.bias"), QX),
            A=to_q(-np.exp(g("mixer.A_log")), QX),
            D=to_q(g("mixer.D"), QX),
            out_proj=quant_linear(g("mixer.out_proj.weight")),
        ))
    embed = quant_linear(w["backbone.embeddings.weight"])
    head = quant_linear(w["lm_head.weight"]) if "lm_head.weight" in w else embed
    return QModel(cfg, embed, layers, to_q(w["backbone.norm_f.weight"], QX), head,
                  int(round(cfg.eps * 2.0 ** 32)))


# ------------------------------------------------------------------ kernels

def quant_act(v16):
    """Q16 vector -> (int8 values, shift s) with real = q * 2**(s - 16)."""
    v = np.asarray(v16, I64)
    m = int(np.abs(v).max())
    s = bitlen(m) - 7
    if int(rshr(m, s)) > 127:
        s += 1
    return np.clip(rshr(v, s), -127, 127), s


def matvec_q(L: QLinear, v16, name="matvec"):
    """int8 x int8 -> int32 accumulator (mb.mac4) -> Q16 output by a per-row
    signed shift with rounding and int32 saturation (mb.racc)."""
    q, s = quant_act(v16)
    acc = chk32(name + ".acc", L.q @ q)
    sh = L.sw - s
    if sh.min() < RACC_SH_MIN or sh.max() > RACC_SH_MAX:
        OVERFLOWS[name + ".shift"] = OVERFLOWS.get(name + ".shift", 0) + 1
    return sat32(rshr(acc, np.clip(sh, RACC_SH_MIN, RACC_SH_MAX)))


def rmsnorm_q(x16, w16, eps_q32):
    x = np.asarray(x16, I64)
    ms_ = int(np.sum(x * x)) // len(x) + eps_q32      # Q32 mean square (floor), > 0
    if ms_ >= (1 << 63):
        OVERFLOWS["rmsnorm.ss"] = OVERFLOWS.get("rmsnorm.ss", 0) + 1
    k = min(14, (62 - bitlen(ms_)) // 2)
    r = isqrt(ms_ << (2 * k))                          # rms in Q(16+k)
    inv = (1 << (48 + k)) // r                         # 1/rms in Q32
    xw = rshr(x * w16, QX)
    return chk32("rmsnorm", rshr(xw * inv, 32))


@dataclass
class QLayerState:
    conv: np.ndarray     # [Ed, K-1] Q16
    h: np.ndarray        # [Ed, N]   Q20


def init_state_q(cfg):
    return [QLayerState(np.zeros((cfg.d_inner, cfg.d_conv - 1), I64),
                        np.zeros((cfg.d_inner, cfg.d_state), I64)) for _ in range(cfg.n_layer)]


def block_step_q(Q: QModel, i, x, st: QLayerState, trace=None):
    cfg, L = Q.cfg, Q.layers[i]
    Ed, N, R = cfg.d_inner, cfg.d_state, cfg.dt_rank

    xn = rmsnorm_q(x, L.norm, Q.eps_q32)
    xz = matvec_q(L.in_proj, xn, "in_proj")
    u_in, z = xz[:Ed], xz[Ed:]

    window = np.concatenate([st.conv, u_in[:, None]], axis=1)            # Q16
    u_conv = chk32("conv", rshr(np.sum(window * L.conv_w, axis=1), QX) + L.conv_b)
    st.conv = window[:, 1:].copy()
    u = silu_q(u_conv)

    xdbl = matvec_q(L.x_proj, u, "x_proj")
    dt_low, B, C = xdbl[:R], xdbl[R:R + N], xdbl[R + N:]
    delta = softplus_q(chk32("dt_proj", matvec_q(L.dt_proj, dt_low, "dt_proj") + L.dt_b))   # Q24

    # scan: h = exp(delta*A)*h + (delta*u)*B ; y = C.h + D*u
    dA_in = np.maximum(rshr(delta[:, None] * L.A, QX), INT32_MIN)        # Q24, <= 0
    dA = exp_q(dA_in)                                                    # Q30
    du = chk32("du", rshr(delta * u, QX))                                # Q24
    dBu = rshr(du[:, None] * B[None, :], QDT + QX - QH)                  # Q20
    st.h = chk32("h", rshr(dA * st.h, QDA) + dBu)
    y = chk32("y", rshr(st.h @ C, QH) + rshr(L.D * u, QX))              # Q16

    yg = chk32("gate", rshr(y * silu_q(z), QX))
    out = matvec_q(L.out_proj, yg, "out_proj")
    if trace is not None:
        trace.update(xn=xn, u_in=u_in, z=z, u=u, B=B, C=C, delta=delta, dA=dA,
                     h=st.h.copy(), y=y, out=out)
    return chk32("x", x + out)


def model_step_q(Q: QModel, token, state, trace=None):
    E = Q.embed
    x = rshr(E.q[token], E.sw[token] - QX)                               # Q16
    for i in range(Q.cfg.n_layer):
        t = {} if trace is not None else None
        x = block_step_q(Q, i, x, state[i], t)
        if trace is not None:
            t["x_out"] = x
            trace.append(t)
    xf = rmsnorm_q(x, Q.norm_f, Q.eps_q32)
    return matvec_q(Q.head, xf, "head")                                  # Q16 logits


# ------------------------------------------------------------------- report

def table_report():
    rows = []
    x = np.linspace(-20, 0, 200001)
    got = from_q(exp_q(to_q(x, QDT)), QDA)
    rows.append(("exp, x in [-20,0]", np.abs(got - np.exp(x)).max(),
                 np.max(np.abs(got - np.exp(x))[x > -1] / np.exp(x[x > -1]))))
    x = np.linspace(-20, 20, 400001)
    got = from_q(silu_q(to_q(x, QX)), QX)
    rows.append(("silu, x in [-20,20]", np.abs(got - ms.silu(x)).max(), np.nan))
    x = np.linspace(-20, 25, 450001)
    ref = ms.softplus(x)
    got = from_q(softplus_q(to_q(x, QX)), QDT)
    m = x > -10
    rows.append(("softplus, x in [-20,25]", np.abs(got - ref).max(),
                 np.max(np.abs(got - ref)[m] / ref[m])))
    print(f"{'function':26s} {'max abs err':>12s} {'max rel err':>12s}")
    for n, a, r in rows:
        print(f"{n:26s} {a:12.2e} {r:12.2e}")
    print("  (exp rel err for x > -1, where decay is close to 1; softplus rel err for x > -10,\n"
          "   below that softplus < 5e-5 and Q24 resolution dominates)")


def compare(cfg, ntok=32, seed=0):
    OVERFLOWS.clear()
    w = ms.random_weights(cfg, seed)
    Q = quantize_model(w, cfg)
    tokens = np.random.default_rng(seed + 100).integers(0, cfg.vocab_size, ntok)
    sf, sq = ms.init_state(cfg), init_state_q(cfg)
    ex, eh, el, agree = 0.0, 0.0, 0.0, 0
    for t in tokens:
        tf, tq = [], []
        lf = ms.model_step(w, cfg, int(t), sf, tf)
        lq = from_q(model_step_q(Q, int(t), sq, tq), QX)
        for a, b in zip(tf[:-1], tq):
            xf = a["x_in"] + a["out"]
            ex = max(ex, np.abs(from_q(b["x_out"], QX) - xf).max() / np.abs(xf).max())
            eh = max(eh, np.abs(from_q(b["h"], QH) - a["h"]).max() / np.abs(a["h"]).max())
        el = max(el, np.abs(lq - lf).max() / np.abs(lf).max())
        agree += int(np.argmax(lq) == np.argmax(lf))
    print(f"d={cfg.d_model} L={cfg.n_layer} tokens={ntok}: worst rel err  layers {ex:.2e}  "
          f"h {eh:.2e}  logits {el:.2e}   top-1 agree {agree}/{ntok}   "
          f"int32 overflows: {OVERFLOWS or 'none'}")


if __name__ == "__main__":
    table_report()
    print()
    compare(ms.MambaConfig(d_model=64, n_layer=2))
    compare(ms.MambaConfig(d_model=256, n_layer=4), ntok=16)
