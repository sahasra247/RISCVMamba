"""Cycle-accurate Python transcription of rtl/mb_mac_unit.v.

This is NOT a substitute for simulating the Verilog. It checks the design *logic*
(pipeline timing, hold rules, racc waiting, 33-bit rounding) against mbmac.ref
using the same vectors and the same kind of random handshake as the testbench.
"""
import random
from mbmac import ref


def wrap(x, bits):
    return ((x + (1 << (bits - 1))) & ((1 << bits) - 1)) - (1 << (bits - 1))


class Unit:
    def __init__(self, out_bits=8):
        self.out_bits = out_bits
        self.reset()

    def reset(self):
        self.v1 = self.v2 = False
        self.rsp_valid = False
        self.rsp_data = 0
        self.racc_busy = False
        self.racc_shift = 0
        self.acc = 0
        self.prod = [0, 0, 0, 0]
        self.psum = 0

    @property
    def cmd_ready(self):
        return (not self.rsp_valid) and (not self.racc_busy)

    def racc_result(self):
        acc_ext = wrap(self.acc, 33)
        rnd_add = 0 if self.racc_shift == 0 else (1 << (self.racc_shift - 1))
        biased = wrap(acc_ext + rnd_add, 33)
        assert biased == acc_ext + rnd_add, "33-bit bias add overflowed"
        shifted = biased >> self.racc_shift
        smax = (1 << (self.out_bits - 1)) - 1
        smin = -(1 << (self.out_bits - 1))
        sat = smax if shifted > smax else smin if shifted < smin else shifted
        return sat & 0xFFFFFFFF

    def edge(self, cmd_valid, fid, in0, in1, rsp_ready):
        """One rising edge. Returns True if the command was accepted on this edge."""
        accept = cmd_valid and self.cmd_ready
        f3, f7 = fid & 7, fid >> 3
        is_mac4 = f3 == 0 and f7 == 0
        is_racc = f3 == 0 and f7 == 1
        racc_done = self.racc_busy and not self.v1 and not self.v2
        n = dict(v1=self.v1, v2=self.v2, rsp_valid=self.rsp_valid, rsp_data=self.rsp_data,
                 racc_busy=self.racc_busy, racc_shift=self.racc_shift, acc=self.acc)
        n["prod"] = [ref.s8(in0 >> (8 * i)) * ref.s8(in1 >> (8 * i)) for i in range(4)]
        n["psum"] = sum(self.prod)
        n["v1"] = accept and is_mac4
        n["v2"] = self.v1
        if self.rsp_valid and rsp_ready:
            n["rsp_valid"] = False
        if accept:
            if is_racc:
                n["racc_busy"] = True
                n["racc_shift"] = in0 & 31
            else:
                n["rsp_valid"] = True
                n["rsp_data"] = 0
        if self.v2:
            n["acc"] = ref.s32(self.acc + self.psum)
        if racc_done:
            n["rsp_data"] = self.racc_result()
            n["rsp_valid"] = True
            n["racc_busy"] = False
            n["acc"] = 0
        for k, v in n.items():
            setattr(self, k, v)
        return accept


def run(rows, out_bits=8, seed=1, stall_pct=30, verbose=False):
    rng = random.Random(seed)
    u = Unit(out_bits)
    prev = (False, True, 0)      # prev rsp_valid, prev rsp_ready, prev data
    cycles = 0
    min_gap_hazard = 0

    def tick(cmd_valid=False, fid=0, in0=0, in1=0):
        nonlocal prev, cycles
        rsp_ready = rng.randrange(100) >= stall_pct
        # protocol monitor: a pending response must be held unchanged
        if prev[0] and not prev[1]:
            assert u.rsp_valid and u.rsp_data == prev[2], "response dropped/changed while stalled"
        before = (u.rsp_valid, u.rsp_data)
        transferred = u.rsp_valid and rsp_ready
        accepted = u.edge(cmd_valid, fid, in0, in1, rsp_ready)
        prev = (before[0], rsp_ready, before[1])
        cycles += 1
        return accepted, transferred, before[1]

    for idx, (hdr, rs1, rs2, exp_rsp, exp_acc) in enumerate(rows):
        gap = 0 if rng.randrange(100) < 50 else rng.randrange(5)
        for _ in range(gap):
            tick()
        fid = hdr & 0x3FF
        while True:
            accepted, _, _ = tick(True, fid, rs1, rs2)
            if accepted:
                break
        # operands are valid for ONE cycle only: scramble afterwards
        got = None
        for _ in range(2000):
            _, transferred, data = tick(False, rng.getrandbits(10), rng.getrandbits(32), rng.getrandbits(32))
            if transferred:
                got = data
                break
        assert got is not None, f"row {idx}: response never arrived (hang)"
        assert got == exp_rsp, f"row {idx}: rsp {got:08x} expected {exp_rsp:08x}"
        if hdr & 0x8000:
            for _ in range(3):
                tick()
            assert (u.acc & 0xFFFFFFFF) == exp_acc, f"row {idx}: acc {u.acc & 0xFFFFFFFF:08x} expected {exp_acc:08x}"
    return cycles


def load(path):
    rows = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            rows.append((int(line[0:4], 16), int(line[4:12], 16), int(line[12:20], 16),
                         int(line[20:28], 16), int(line[28:36], 16)))
    return rows


if __name__ == "__main__":
    import sys, time
    t = time.time()
    rows = load(sys.argv[1])
    for seed, stall in ((1, 30), (2, 0), (3, 70)):
        c = run(rows, seed=seed, stall_pct=stall)
        print(f"seed {seed} stall {stall}%: {len(rows)} rows OK, {c} cycles")
    print(f"{time.time()-t:.0f}s")
