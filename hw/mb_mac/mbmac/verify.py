"""Generate vectors, compile the RTL with Icarus Verilog, run the testbench, save results.

    python3 -m mbmac.verify                 # full run (about 350k commands)
    python3 -m mbmac.verify --quick         # lane-0 sweep only, no wrap test (fast)
"""

import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess

from .vectors import generate

ROOT = Path(__file__).resolve().parents[1]


def executable(name: str, env_name: str) -> str:
    candidate = os.environ.get(env_name) or shutil.which(name)
    if candidate:
        return candidate
    local = Path.home() / "tools/iverilog/usr/bin" / name
    if local.is_file():
        return str(local)
    raise SystemExit(f"{name} not found. Install it with: sudo apt install -y iverilog")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", type=Path, default=ROOT / "build")
    parser.add_argument("--seed", type=int, default=247)
    parser.add_argument("--out-bits", type=int, default=8)
    parser.add_argument("--stall-pct", type=int, default=30)
    parser.add_argument("--quick", action="store_true")
    args = parser.parse_args()

    build = args.build.resolve()
    vec_dir = build / "vectors"
    manifest = generate(vec_dir, seed=args.seed, out_bits=args.out_bits,
                        lanes_swept=1 if args.quick else 4,
                        random_sequences=500 if args.quick else 3000,
                        wrap_test=not args.quick)
    print(f"vectors: {manifest['count']} commands {manifest['commands']}")

    ivl, vvp = executable("iverilog", "IVERILOG"), executable("vvp", "VVP")
    image = build / "tb_mb_mac.vvp"
    command = [ivl, "-g2012", "-Wall", "-s", "tb_mb_mac",
               f"-Ptb_mb_mac.OUT_BITS={args.out_bits}",
               f"-Ptb_mb_mac.STALL_PCT={args.stall_pct}",
               "-o", str(image), str(ROOT / "rtl/mb_mac_unit.v"), str(ROOT / "tb/tb_mb_mac.sv")]
    compiled = subprocess.run(command, text=True, capture_output=True)
    if compiled.returncode:
        print(compiled.stdout, compiled.stderr)
        raise SystemExit("COMPILE FAILED")
    if compiled.stderr:
        print(compiled.stderr, end="")

    result = subprocess.run([vvp, str(image), f"+VECTORS={vec_dir / 'mac_vectors.hex'}",
                             f"+COUNT={manifest['count']}"], text=True, capture_output=True)
    print(result.stdout, end="")
    if result.returncode or "PASS " not in result.stdout:
        print(result.stderr)
        raise SystemExit("SIMULATION FAILED (see the FATAL line above)")
    (build / "rtl_results.json").write_text(json.dumps(
        {"manifest": manifest, "stdout": result.stdout, "compile_stderr": compiled.stderr}, indent=2) + "\n")
    print(f"Results: {build / 'rtl_results.json'}")


if __name__ == "__main__":
    main()
