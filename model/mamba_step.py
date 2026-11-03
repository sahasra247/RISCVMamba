"""NumPy reference for Mamba (v1), one token at a time.

This is the float golden model for the Xmamba project. It is written as the
recurrent "step" form the RISC-V core will run (one state update per token),
with plain operations so each line maps onto a C kernel.

Weight names follow HuggingFace MambaForCausalLM (as used in MambaX.ipynb):
    backbone.embeddings.weight                 [V, d]
    backbone.layers.{i}.norm.weight            [d]
    backbone.layers.{i}.mixer.in_proj.weight   [2*Ed, d]
    backbone.layers.{i}.mixer.conv1d.weight    [Ed, 1, K]
    backbone.layers.{i}.mixer.conv1d.bias      [Ed]
    backbone.layers.{i}.mixer.x_proj.weight    [R + 2N, Ed]
    backbone.layers.{i}.mixer.dt_proj.weight   [Ed, R]
    backbone.layers.{i}.mixer.dt_proj.bias     [Ed]
    backbone.layers.{i}.mixer.A_log            [Ed, N]
    backbone.layers.{i}.mixer.D                [Ed]
    backbone.layers.{i}.mixer.out_proj.weight  [d, Ed]
    backbone.norm_f.weight                     [d]
    lm_head.weight                             [V, d]  (tied to embeddings if absent)
where d = d_model, Ed = expand * d_model, N = d_state, R = dt_rank, K = d_conv.

Status
------
Done (B):  config, random weights, block_step (one fully worked layer),
           model_step, a trace of every intermediate, and a self-check of the
           step form against an independent whole-sequence form.
TODO (C):  load_weights() from the real checkpoint; check model_step against
           PyTorch on 20 sequences (max abs error < 1e-4); export test vectors
           from trace for 10 tokens; head for a classification task if chosen.

Run the self-check:  python3 model/mamba_step.py
"""

from __future__ import annotations

import math
from dataclasses import dataclass, field

import numpy as np


# --------------------------------------------------------------------- config

@dataclass
class MambaConfig:
    d_model: int = 256
    n_layer: int = 4
    vocab_size: int = 256
    d_state: int = 16
    expand: int = 2
    d_conv: int = 4
    dt_rank: int = 0               # 0 means "auto" = ceil(d_model / 16)
    eps: float = 1e-5              # RMSNorm epsilon

    def __post_init__(self):
        if self.dt_rank == 0:
            self.dt_rank = math.ceil(self.d_model / 16)

    @property
    def d_inner(self) -> int:
        return self.expand * self.d_model


def n_params(cfg: MambaConfig, tied: bool = True) -> int:
    d, Ed, N, R, K = cfg.d_model, cfg.d_inner, cfg.d_state, cfg.dt_rank, cfg.d_conv
    per_layer = (d                    # norm
                 + 2 * Ed * d         # in_proj
                 + Ed * K + Ed        # conv1d
                 + (R + 2 * N) * Ed   # x_proj
                 + Ed * R + Ed        # dt_proj
                 + Ed * N + Ed        # A_log, D
                 + d * Ed)            # out_proj
    total = cfg.n_layer * per_layer + cfg.vocab_size * d + d
    return total if tied else total + cfg.vocab_size * d


# -------------------------------------------------------------------- weights

def P(i: int, name: str) -> str:
    """Full weight name of a per-layer tensor, e.g. P(0, 'mixer.D')."""
    return f"backbone.layers.{i}.{name}"


def random_weights(cfg: MambaConfig, seed: int = 0) -> dict[str, np.ndarray]:
    """Weights with the same shapes and roughly the same init as mamba_ssm.
    Used until the real checkpoint exists."""
    rng = np.random.default_rng(seed)
    d, Ed, N, R, K = cfg.d_model, cfg.d_inner, cfg.d_state, cfg.dt_rank, cfg.d_conv
    f32 = np.float32

    def lin(out_f, in_f):
        b = 1.0 / math.sqrt(in_f)
        return rng.uniform(-b, b, (out_f, in_f)).astype(f32)

    w = {"backbone.embeddings.weight": (rng.standard_normal((cfg.vocab_size, d)) * 0.02).astype(f32),
         "backbone.norm_f.weight": np.ones(d, f32)}
    for i in range(cfg.n_layer):
        dt = np.exp(rng.uniform(math.log(1e-3), math.log(1e-1), Ed))        # dt in [0.001, 0.1]
        w |= {
            P(i, "norm.weight"): np.ones(d, f32),
            P(i, "mixer.in_proj.weight"): lin(2 * Ed, d),
            P(i, "mixer.conv1d.weight"): rng.uniform(-0.5, 0.5, (Ed, 1, K)).astype(f32),
            P(i, "mixer.conv1d.bias"): rng.uniform(-0.5, 0.5, Ed).astype(f32),
            P(i, "mixer.x_proj.weight"): lin(R + 2 * N, Ed),
            P(i, "mixer.dt_proj.weight"): rng.uniform(-R ** -0.5, R ** -0.5, (Ed, R)).astype(f32),
            P(i, "mixer.dt_proj.bias"): (dt + np.log(-np.expm1(-dt))).astype(f32),  # softplus^-1(dt)
            P(i, "mixer.A_log"): np.log(np.tile(np.arange(1, N + 1, dtype=f32), (Ed, 1))),
            P(i, "mixer.D"): np.ones(Ed, f32),
            P(i, "mixer.out_proj.weight"): lin(d, Ed),
        }
    return w


def load_weights(path: str) -> dict[str, np.ndarray]:
    """TODO (C): load the trained checkpoint (or the exported .npy folder) into
    a dict with the names above, as float32. Add 'lm_head.weight' only if it is
    not tied to the embeddings."""
    raise NotImplementedError


def check_shapes(w: dict[str, np.ndarray], cfg: MambaConfig) -> None:
    ref = random_weights(cfg)
    for k, v in ref.items():
        assert k in w, f"missing weight {k}"
        assert w[k].shape == v.shape, f"{k}: shape {w[k].shape}, expected {v.shape}"


# ------------------------------------------------------------ math functions
# These are the functions the fixed-point engine replaces with lookup tables.

def silu(x):
    return x / (1.0 + np.exp(-x))


def softplus(x):
    return np.where(x > 20.0, x, np.log1p(np.exp(np.minimum(x, 20.0))))   # torch threshold = 20


def rmsnorm(x, weight, eps):
    return x / np.sqrt(np.mean(x * x) + eps) * weight


# ---------------------------------------------------------------------- state

@dataclass
class LayerState:
    conv: np.ndarray               # [Ed, K-1] last K-1 inputs to conv1d, oldest first
    h: np.ndarray                  # [Ed, N]   SSM hidden state


@dataclass
class State:
    layers: list[LayerState] = field(default_factory=list)


def init_state(cfg: MambaConfig) -> State:
    return State([LayerState(np.zeros((cfg.d_inner, cfg.d_conv - 1), np.float32),
                             np.zeros((cfg.d_inner, cfg.d_state), np.float32))
                  for _ in range(cfg.n_layer)])


# ------------------------------------------------------------ one layer, one token

def block_step(w, cfg: MambaConfig, i: int, x: np.ndarray, st: LayerState,
               trace: dict | None = None) -> np.ndarray:
    """One Mamba block for one token. x: [d] residual stream in, returns [d].
    Updates st in place. If trace is a dict, every intermediate is stored in it
    (these become the per-stage test vectors)."""
    Ed, N, R = cfg.d_inner, cfg.d_state, cfg.dt_rank
    m = lambda name: w[P(i, "mixer." + name)]

    # 1. RMSNorm
    xn = rmsnorm(x, w[P(i, "norm.weight")], cfg.eps)

    # 2. in_proj -> u (to the conv/scan path) and z (gate)
    xz = m("in_proj.weight") @ xn                                  # [2Ed]
    u_in, z = xz[:Ed], xz[Ed:]

    # 3. causal depthwise conv1d over the last K inputs, then SiLU
    window = np.concatenate([st.conv, u_in[:, None]], axis=1)      # [Ed, K], oldest first
    u_conv = np.sum(window * m("conv1d.weight")[:, 0, :], axis=1) + m("conv1d.bias")
    st.conv = window[:, 1:].copy()
    u = silu(u_conv)                                               # [Ed]

    # 4. x_proj -> (dt_low, B, C); dt_proj + softplus -> delta
    xdbl = m("x_proj.weight") @ u                                  # [R + 2N]
    dt_low, B, C = xdbl[:R], xdbl[R:R + N], xdbl[R + N:]
    delta = softplus(m("dt_proj.weight") @ dt_low + m("dt_proj.bias"))   # [Ed]

    # 5. selective scan (one step):  h = exp(delta*A) * h + delta*B*u ;  y = C.h + D*u
    A = -np.exp(m("A_log"))                                        # [Ed, N], all < 0
    dA = np.exp(delta[:, None] * A)                                # [Ed, N], in (0, 1]
    dBu = (delta * u)[:, None] * B[None, :]                        # [Ed, N]
    st.h = dA * st.h + dBu
    y = st.h @ C + m("D") * u                                      # [Ed]

    # 6. gate, out_proj, residual
    yg = y * silu(z)
    out = m("out_proj.weight") @ yg                                # [d]

    if trace is not None:
        trace.update(x_in=x, xn=xn, u_in=u_in, z=z, u_conv=u_conv, u=u, dt_low=dt_low,
                     B=B, C=C, delta=delta, dA=dA, h=st.h.copy(), y=y, yg=yg, out=out)
    return x + out


# ------------------------------------------------------------ whole model, one token

def model_step(w, cfg: MambaConfig, token: int, state: State,
               trace: list | None = None) -> np.ndarray:
    """Embed one token, run all layers, return logits [V]. If trace is a list,
    one dict per layer is appended (plus a final dict with the logits)."""
    x = w["backbone.embeddings.weight"][token].astype(np.float32)
    for i in range(cfg.n_layer):
        t = {} if trace is not None else None
        x = block_step(w, cfg, i, x, state.layers[i], t)
        if trace is not None:
            trace.append(t)
    xf = rmsnorm(x, w["backbone.norm_f.weight"], cfg.eps)
    head = w.get("lm_head.weight", w["backbone.embeddings.weight"])
    logits = head @ xf
    if trace is not None:
        trace.append({"x_final": xf, "logits": logits})
    return logits


# ------------------------------------------- independent whole-sequence version
# Written differently on purpose (full-sequence conv, closed-form scan with
# cumulative sums) so it can check the step form. Not used on the target.

def model_sequence(w, cfg: MambaConfig, tokens: np.ndarray) -> np.ndarray:
    Ed, N, R, K = cfg.d_inner, cfg.d_state, cfg.dt_rank, cfg.d_conv
    L = len(tokens)
    x = w["backbone.embeddings.weight"][tokens].astype(np.float64)            # [L, d]
    for i in range(cfg.n_layer):
        m = lambda name: w[P(i, "mixer." + name)].astype(np.float64)
        xn = x / np.sqrt(np.mean(x * x, axis=1, keepdims=True) + cfg.eps) * w[P(i, "norm.weight")]
        xz = xn @ m("in_proj.weight").T
        u_in, z = xz[:, :Ed], xz[:, Ed:]
        pad = np.vstack([np.zeros((K - 1, Ed)), u_in])                         # causal padding
        cw = m("conv1d.weight")[:, 0, :]                                       # [Ed, K]
        u = silu(sum(pad[k:k + L] * cw[:, k] for k in range(K)) + m("conv1d.bias"))
        xdbl = u @ m("x_proj.weight").T
        dt_low, B, C = xdbl[:, :R], xdbl[:, R:R + N], xdbl[:, R + N:]
        delta = softplus(dt_low @ m("dt_proj.weight").T + m("dt_proj.bias"))   # [L, Ed]
        A = -np.exp(m("A_log"))
        S = np.cumsum(delta[:, :, None] * A[None], axis=0)                     # [L, Ed, N]
        dBu = (delta * u)[:, :, None] * B[:, None, :]                          # [L, Ed, N]
        # h_t = sum_{s<=t} exp(S_t - S_s) * dBu_s
        dec = np.exp(np.clip(S[:, None] - S[None, :], None, 0.0))              # [t, s, Ed, N]
        dec *= np.tril(np.ones((L, L)))[:, :, None, None]
        h = np.einsum("tsen,sen->ten", dec, dBu)
        y = np.einsum("ten,tn->te", h, C) + m("D") * u
        x = x + (y * silu(z)) @ m("out_proj.weight").T
    xf = x / np.sqrt(np.mean(x * x, axis=1, keepdims=True) + cfg.eps) * w["backbone.norm_f.weight"]
    head = w.get("lm_head.weight", w["backbone.embeddings.weight"]).astype(np.float64)
    return xf @ head.T                                                         # [L, V]


# ------------------------------------------------------------------ self-check

def _self_check():
    cfg = MambaConfig(d_model=64, n_layer=2, vocab_size=256)
    w = random_weights(cfg, seed=1)
    check_shapes(w, cfg)
    tokens = np.random.default_rng(2).integers(0, cfg.vocab_size, 24)

    st = init_state(cfg)
    step = np.stack([model_step(w, cfg, int(t), st) for t in tokens])
    seq = model_sequence(w, cfg, tokens)
    err = np.max(np.abs(step - seq))
    rel = err / np.max(np.abs(seq))
    print(f"step vs sequence form: max abs err {err:.2e}, relative {rel:.2e}")

    big = MambaConfig()
    print(f"default config d={big.d_model} L={big.n_layer} N={big.d_state} E={big.expand} "
          f"R={big.dt_rank} V={big.vocab_size}: {n_params(big) / 1e6:.2f}M params")
    ok = rel < 1e-5
    print("mamba_step self-check:", "PASS" if ok else "FAIL")
    return ok


if __name__ == "__main__":
    raise SystemExit(0 if _self_check() else 1)
