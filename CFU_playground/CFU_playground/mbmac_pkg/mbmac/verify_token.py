"""Verify the complete Xmamba CFU against the independent integer oracle."""

import argparse
import json
import os
from pathlib import Path
import random
import shutil
import subprocess

from .verify import ROOT, executable
from .xmamba import Reference, generated_files, signed


def instruction_vectors(path):
    reference = Reference()
    records = []

    def emit(function, operand0=0, operand1=0):
        result = reference.execute(function, operand0 & 0xffffffff, operand1 & 0xffffffff)
        records.append(f"{function:04x}{operand0 & 0xffffffff:08x}{operand1 & 0xffffffff:08x}"
                       f"{result & 0xffffffff:08x}00000000\n")

    for operation in range(3):
        for value in [-(1 << 31), -2049, *range(-2048, 2049), 2049, (1 << 31) - 1]:
            emit((operation << 3) | 1, value)
    rng = random.Random(247)
    for layer in range(2):
        emit(10, layer)
        for address in range(40):
            emit(2, address, rng.randrange(-32768, 32768))
        for state in range(4):
            emit(18, state, rng.getrandbits(32))
    for _ in range(400):
        emit(10, rng.randrange(2))
        emit(26, rng.getrandbits(32), rng.randrange(8))
        emit(0, rng.getrandbits(32), rng.getrandbits(32))
        emit(8, rng.randrange(32))
    emit(34)
    for layer in range(2):
        emit(10, layer)
        for channel in range(8):
            emit(26, 0x7fff7fff, channel)
    for function in (2, 10, 18, 26, 42, 1023):
        emit(function, 0xffffffff, 0xffffffff)
    path.write_text("".join(records))
    return len(records)


def check_rtl(build, vectors, count, stall):
    image = build / f"xmamba_{stall}.vvp"
    subprocess.run([executable("iverilog", "IVERILOG"), "-g2012", "-Wall",
                    "-I", str(ROOT / "rtl"), "-s", "tb_mb_mac",
                    "-Ptb_mb_mac.CFU_TOP=1", f"-Ptb_mb_mac.STALL_PCT={stall}",
                    "-o", str(image), str(ROOT / "rtl/mb_mac_unit.v"),
                    str(ROOT / "rtl/mb_xmamba_unit.v"), str(ROOT / "rtl/cfu_top.v"),
                    str(ROOT / "tb/tb_mb_mac.sv")], check=True)
    subprocess.run([executable("vvp", "VVP"), str(image), f"+VECTORS={vectors}",
                    f"+COUNT={count}"], check=True, timeout=120)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", type=Path, default=ROOT / "build/token")
    parser.add_argument("--verilator", action="store_true", help="also run C++ inference against live RTL")
    args = parser.parse_args()
    build = args.build.resolve()
    build.mkdir(parents=True, exist_ok=True)
    for path, content in generated_files().items():
        if not path.exists() or path.read_text() != content:
            raise SystemExit(f"Stale table: {path}; run python3 -m mbmac.xmamba")
    vectors = build / "instructions.hex"
    count = instruction_vectors(vectors)
    for stall in (0, 70):
        check_rtl(build, vectors, count, stall)
    print(f"PASS Xmamba instruction oracle: {count} commands per stall pattern")
    compiler = os.environ.get("CXX") or shutil.which("g++")
    if not compiler:
        raise SystemExit("C++ compiler not found; set CXX or install g++")
    host = build / "token_host"
    subprocess.run([compiler, "-std=c++11", "-O2", "-Wall", "-Wextra", "-Werror",
                    "-I", str(ROOT / "soc"), str(ROOT / "tests/token_test.cc"),
                    "-o", str(host)], check=True)
    pipeline_vectors = build / "pipeline.hex"
    subprocess.run([str(host), str(pipeline_vectors)], check=True, timeout=120)
    reference = Reference()
    records = pipeline_vectors.read_text().splitlines()
    for row, record in enumerate(records):
        function = int(record[:4], 16)
        operand0, operand1 = int(record[4:12], 16), int(record[12:20], 16)
        expected = signed(int(record[20:28], 16), 32)
        got = reference.execute(function, operand0, operand1)
        if got != expected:
            raise SystemExit(f"C++ / Python mismatch at command {row}: {got} != {expected}")
    for stall in (0, 70):
        check_rtl(build, pipeline_vectors, len(records), stall)
    if args.verilator:
        verilator = os.environ.get("VERILATOR") or shutil.which("verilator")
        if not verilator:
            raise SystemExit("Verilator not found; set VERILATOR or install verilator")
        model = build / "verilated"
        subprocess.run([verilator, "--cc", "--exe", "--build", "-j", "2", "-Wall",
                        "-Wno-DECLFILENAME", "-Wno-PINCONNECTEMPTY", "-Wno-UNUSED",
                        "--top-module", "Cfu", f"-I{ROOT / 'rtl'}", "--Mdir", str(model),
                        "-CFLAGS", f'-std=c++11 -DXMAMBA_VERILATOR -I"{ROOT / "soc"}"',
                        str(ROOT / "rtl/mb_mac_unit.v"), str(ROOT / "rtl/mb_xmamba_unit.v"),
                        str(ROOT / "rtl/cfu_top.v"), str(ROOT / "tests/token_test.cc")], check=True)
        subprocess.run([str(model / "VCfu"), str(build / "live.hex")], check=True, timeout=120)
    summary = {"instruction_vectors": count, "pipeline_commands": len(records),
               "tokens": 20, "stall_percentages": [0, 70], "live_verilator": args.verilator,
               "whole_soc_executed": False, "model": "deterministic synthetic fixture"}
    (build / "results.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("PASS token pipeline: Python oracle, C++ baseline, RTL replay" +
          (", live Verilator" if args.verilator else ""))


if __name__ == "__main__":
    main()