"""Deterministic test-vector generation for tb_mb_mac.

Each record is one CFU command, packed MSB-first as 36 hex digits:

    header[15:0] | rs1[31:0] | rs2[31:0] | expected_rsp[31:0] | expected_acc[31:0]

header[9:0]  = function id {funct7, funct3}
header[15]   = 1: after the response, also compare the accumulator with expected_acc
               (mb.racc and reserved ops always set it; exhaustive sweeps set it)

Commands are issued in order; the testbench waits for each response, like the CPU does.
"""

import argparse
import json
from pathlib import Path
import random

from . import ref

FORCE = 1 << 15
EDGE = [-128, -127, -64, -1, 0, 1, 63, 64, 126, 127]


class Gen:
    def __init__(self, out_bits: int):
        self.out_bits = out_bits
        self.acc = 0
        self.rows: list[tuple[int, int, int, int, int]] = []
        self.counts = {"mac4": 0, "racc": 0, "reserved": 0}

    def mac4(self, rs1: int, rs2: int, force: bool = False):
        self.acc = ref.mac4(self.acc, rs1, rs2)
        hdr = ref.function_id(ref.F7_MAC4) | (FORCE if force else 0)
        self.rows.append((hdr, rs1, rs2, 0, self.acc & ref.MASK32))
        self.counts["mac4"] += 1

    def racc(self, rs1: int):
        rd, self.acc = ref.racc(self.acc, rs1, self.out_bits)
        hdr = ref.function_id(ref.F7_RACC) | FORCE
        self.rows.append((hdr, rs1 & ref.MASK32, 0, rd, 0))
        self.counts["racc"] += 1

    def reserved(self, fid: int, rs1: int, rs2: int):
        """Unknown operation: must answer 0 and leave the accumulator alone."""
        self.rows.append((fid | FORCE, rs1, rs2, 0, self.acc & ref.MASK32))
        self.counts["reserved"] += 1


def _product_table():
    table = {}
    for a in range(-128, 128):
        for b in range(-128, 128):
            table.setdefault(a * b, (a, b))
    return table


def _build(g: Gen, target: int, table):
    """Make the accumulator move by `target` using lane-0 products (splits if needed)."""
    if target in table:
        a, b = table[target]
        g.mac4(ref.pack(a, 0, 0, 0), ref.pack(b, 0, 0, 0))
    else:
        assert target % 2 == 0, target
        _build(g, target // 2, table)
        _build(g, target // 2, table)


def generate(output: Path, *, seed: int = 247, random_sequences: int = 3000,
             lanes_swept: int = 4, out_bits: int = 8, wrap_test: bool = True) -> dict:
    if not 1 <= lanes_swept <= 4:
        raise ValueError("lanes_swept must be 1..4")
    rng = random.Random(seed)
    g = Gen(out_bits)
    table = _product_table()

    # A. Edge values in every lane, one lane at a time, and in all lanes together.
    for lane in range(4):
        for a in EDGE:
            for b in EDGE:
                w1 = [0] * 4
                w2 = [0] * 4
                w1[lane], w2[lane] = a, b
                g.mac4(ref.pack(*w1), ref.pack(*w2), force=True)
        g.racc(0)
    for a in EDGE:
        for b in EDGE:
            g.mac4(ref.pack(a, a, a, a), ref.pack(b, b, b, b), force=True)
            g.mac4(ref.pack(a, b, a, b), ref.pack(b, a, b, a), force=True)
    g.racc(0)

    # B. Rounding ties and every shift amount: acc = +/-2^m, then racc(shift).
    for m in range(0, 15):
        for sign in (1, -1):
            for shift in range(32):
                _build(g, sign * (1 << m), table)
                g.racc(shift)

    # C. Saturation at both ends, every shift. k mac4 ops of an extreme lane pattern.
    patterns = [
        (ref.pack(127, 127, 127, 127), ref.pack(127, 127, 127, 127)),
        (ref.pack(-128, -128, -128, -128), ref.pack(-128, -128, -128, -128)),
        (ref.pack(-128, -128, -128, -128), ref.pack(127, 127, 127, 127)),
    ]
    for rs1, rs2 in patterns:
        for k in (1, 2, 3, 6, 20):
            for shift in (0, 1, 7, 8, 9, 12, 16, 17, 20, 31):
                for _ in range(k):
                    g.mac4(rs1, rs2)
                g.racc(shift)

    # D. Random sequences, including garbage in rs1[31:5] for racc.
    for _ in range(random_sequences):
        for _ in range(rng.randint(0, 8)):
            if rng.random() < 0.3:
                w1 = ref.pack(*(rng.choice(EDGE) for _ in range(4)))
                w2 = ref.pack(*(rng.choice(EDGE) for _ in range(4)))
            else:
                w1, w2 = rng.getrandbits(32), rng.getrandbits(32)
            g.mac4(w1, w2)
        shift = rng.randint(0, 20) if rng.random() < 0.7 else rng.randint(0, 31)
        upper = rng.getrandbits(27) << 5 if rng.random() < 0.5 else 0
        g.racc(upper | shift)

    # E. Reserved function ids must answer 0 and not disturb the accumulator.
    g.mac4(ref.pack(3, -4, 5, -6), ref.pack(7, 8, -9, 10), force=True)
    for _ in range(200):
        if rng.random() < 0.5:
            fid = (rng.randint(2, 127) << 3) | 0
        else:
            fid = (rng.randint(0, 127) << 3) | rng.randint(1, 7)
        g.reserved(fid, rng.getrandbits(32), rng.getrandbits(32))
    g.racc(0)

    # F. 32-bit accumulator wrap: 2^15 mac4 ops of 4 * (-128)*(-128) = 2^16 each.
    if wrap_test:
        big = ref.pack(-128, -128, -128, -128)
        for _ in range(1 << 15):
            g.mac4(big, big)
        g.racc(0)          # wrapped to -2^31 -> saturates to the negative limit
        for _ in range((1 << 15) - 1):
            g.mac4(big, big)
        g.racc(0)          # 2^31 - 2^16, still positive -> positive limit

    # G. Exhaustive signed 8x8 sweep in each lane; accumulator checked on every row.
    for lane in range(lanes_swept):
        count = 0
        for a in range(-128, 128):
            for b in range(-128, 128):
                w1 = [0] * 4
                w2 = [0] * 4
                w1[lane], w2[lane] = a, b
                g.mac4(ref.pack(*w1), ref.pack(*w2), force=True)
                count += 1
                if count % 256 == 0:
                    g.racc(0)

    output.mkdir(parents=True, exist_ok=True)
    with (output / "mac_vectors.hex").open("w") as f:
        for hdr, rs1, rs2, rsp, acc in g.rows:
            f.write(f"{hdr:04x}{rs1:08x}{rs2:08x}{rsp:08x}{acc:08x}\n")
    manifest = {
        "record_format": "header16 | rs1 | rs2 | expected_rsp | expected_acc (36 hex digits)",
        "count": len(g.rows),
        "seed": seed,
        "out_bits": out_bits,
        "lanes_swept": lanes_swept,
        "random_sequences": random_sequences,
        "wrap_test": wrap_test,
        "commands": g.counts,
    }
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    return manifest


if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("output", type=Path)
    p.add_argument("--seed", type=int, default=247)
    args = p.parse_args()
    print(json.dumps(generate(args.output, seed=args.seed), indent=2))
