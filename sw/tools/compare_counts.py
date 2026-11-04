#!/usr/bin/env python3
"""Turn the logs of the four builds (baseline, +MAC, +MAC+NL, full Xmamba) into
the instruction-count table: per kernel and end to end, with reductions.

Usage: compare_counts.py log_baseline.txt log_mac.txt log_macnl.txt log_xmamba.txt [--csv out.csv]
"""
import re
import sys

NAMES = ["baseline", "+MAC", "+MAC+NL", "Xmamba"]


def parse(path):
    text = open(path).read()
    if "mamba_fixed: PASS" not in text:
        sys.exit(f"{path}: run did not PASS bit-exact check")
    head = re.search(r"mamba_fixed: (d=.*)", text).group(1)
    prof = {}
    for m in re.finditer(r"^  (\S.*?)\s+(\d+)\s+\d+\.\d%$", text, re.M):
        prof[m.group(1).strip()] = int(m.group(2))
    prof["total"] = int(re.search(r"^  total\s+(\d+)", text, re.M).group(1))
    return head, prof


def main():
    args = sys.argv[1:]
    csv = None
    if "--csv" in args:
        i = args.index("--csv"); csv = args[i + 1]; del args[i:i + 2]
    runs = [parse(p) for p in args]
    print(f"Instructions per token ({runs[0][0]}); all four builds bit-exact with mamba_fixed.py\n")
    kernels = [k for k in runs[0][1] if k != "total"] + ["total"]
    hdr = f"{'kernel':18s}" + "".join(f"{n:>12s}" for n in NAMES) + f"{'reduction':>11s}"
    print(hdr); print("-" * len(hdr))
    rows = []
    for k in kernels:
        v = [r[1].get(k, 0) for r in runs]
        red = f"{v[0] / v[-1]:.1f}x" if v[-1] else "-"
        sep = "-" * len(hdr) + "\n" if k == "total" else ""
        print(sep + f"{k:18s}" + "".join(f"{x:12d}" for x in v) + f"{red:>11s}")
        rows.append((k, v))
    tot = [r[1]["total"] for r in runs]
    print("\nend-to-end reduction vs baseline: " +
          ", ".join(f"{n} {tot[0] / t:.2f}x" for n, t in zip(NAMES[1:], tot[1:])))
    if csv:
        with open(csv, "w") as f:
            f.write("kernel," + ",".join(NAMES) + "\n")
            for k, v in rows:
                f.write(k + "," + ",".join(map(str, v)) + "\n")


if __name__ == "__main__":
    main()
