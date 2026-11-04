#!/usr/bin/env python3
"""Check that xmamba_ref.c (C) and model/mamba_fixed.py (Python) compute the
same bits for every Xmamba function, on edge cases plus many random inputs.

Usage: check_ref.py build/libxmamba_ref.so      (or: make check)
"""
import ctypes as C
import os
import sys

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "model"))
import mamba_fixed as mf  # noqa: E402

lib = C.CDLL(os.path.abspath(sys.argv[1]))
for f in ("xm_exp", "xm_silu", "xm_softplus"):
    getattr(lib, f).argtypes, getattr(lib, f).restype = [C.c_int32], C.c_int32
lib.xm_racc_ref.argtypes, lib.xm_racc_ref.restype = [C.c_int32, C.c_int32], C.c_int32
P32 = C.POINTER(C.c_int32)
lib.xm_scan_channel.argtypes = [P32, P32, C.c_int32, P32, P32, C.c_int32, C.c_int32]
lib.xm_scan_channel.restype = C.c_int32

rng = np.random.default_rng(1)
I32MIN, I32MAX = -(1 << 31), (1 << 31) - 1
EDGES = [I32MIN, I32MIN + 1, -1, 0, 1, I32MAX - 1, I32MAX]


def inputs(scale_bits, n=200000, extra=()):
    """Edges, values around table boundaries, and random values at many magnitudes."""
    mags = rng.integers(0, 32, n)
    r = rng.integers(-(1 << 31), 1 << 31, n, dtype=np.int64) >> mags
    b = [(k << scale_bits) + d for k in range(-130, 131) for d in (-1, 0, 1)]
    v = np.concatenate([EDGES, list(extra), b, r]).astype(np.int64)
    return np.clip(v, I32MIN, I32MAX)


def check(name, cfun, pyfun, xs):
    got = np.array([cfun(int(x)) for x in xs], np.int64)
    want = np.asarray(pyfun(xs), np.int64)
    bad = np.nonzero(got != want)[0]
    if len(bad):
        i = bad[0]
        print(f"FAIL {name}: {len(bad)} of {len(xs)} differ; first x={xs[i]} C={got[i]} Python={want[i]}")
        return False
    print(f"PASS {name}: {len(xs)} inputs")
    return True


ok = True
ok &= check("mb.exp", lib.xm_exp, mf.exp_q, inputs(24))
ok &= check("mb.silu", lib.xm_silu, mf.silu_q, inputs(16))
ok &= check("mb.softplus", lib.xm_softplus, mf.softplus_q, inputs(16))

# mb.racc: accumulator x shift
accs = inputs(0, 20000)
shs = rng.integers(-34, 35, len(accs))
got = np.array([lib.xm_racc_ref(int(a), int(s)) for a, s in zip(accs, shs)], np.int64)
want = mf.sat32(mf.rshr(accs, np.clip(shs, mf.RACC_SH_MIN, mf.RACC_SH_MAX)))
bad = np.nonzero(got != want)[0]
print(("FAIL" if len(bad) else "PASS") + f" mb.racc: {len(accs)} inputs" + (f", {len(bad)} differ" if len(bad) else ""))
ok &= not len(bad)


# mb.scan (one channel), Python written from block_step_q's scan lines
def scan_py(h, A, Dk, B, Cc, delta, u):
    dA = mf.exp_q(np.maximum(mf.rshr(delta * A, mf.QX), I32MIN))
    du = mf.chk32("du", mf.rshr(delta * u, mf.QX))
    dBu = mf.rshr(du * B, mf.QDT + mf.QX - mf.QH)
    h = mf.chk32("h", mf.rshr(dA * h, mf.QDA) + dBu)
    y = mf.chk32("y", mf.rshr(np.sum(h * Cc), mf.QH) + mf.rshr(Dk * u, mf.QX))
    return h, int(y)


bad = 0
T = 3000
for t in range(T):
    big = t % 10 == 0                                     # every 10th trial: extreme values
    lim = 1 << 31 if big else 1 << 22
    h = rng.integers(-lim, lim, 16).astype(np.int64)
    A = -rng.integers(1, (1 << 31) if big else (16 << 16), 16).astype(np.int64)
    Dk = int(rng.integers(-lim, lim))
    B = rng.integers(-lim, lim, 16).astype(np.int64)
    Cc = rng.integers(-lim, lim, 16).astype(np.int64)
    delta = int(rng.integers(0, (1 << 31) if big else (1 << 24)))
    u = int(rng.integers(-lim, lim))
    hc = (C.c_int32 * 16)(*h.tolist())
    yc = lib.xm_scan_channel(hc, (C.c_int32 * 16)(*A.tolist()), Dk, (C.c_int32 * 16)(*B.tolist()),
                             (C.c_int32 * 16)(*Cc.tolist()), delta, u)
    hp, yp = scan_py(h, A, Dk, B, Cc, delta, u)
    if yc != yp or list(hc) != hp.tolist():
        bad += 1
        if bad == 1:
            print(f"  first scan mismatch at trial {t}: y C={yc} Python={yp}")
print(("FAIL" if bad else "PASS") + f" mb.scan: {T} channel updates (10% with extreme values)"
      + (f", {bad} differ" if bad else ""))
ok &= not bad

print("xmamba_ref check:", "PASS" if ok else "FAIL")
sys.exit(0 if ok else 1)
