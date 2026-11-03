# mb.mac4 / mb.racc: RTL, reference, tests

Hardware side of the MAC unit for the RISCVMamba (Xmamba) CFU. Same flow as the
Scan-Fuse workspace: Python reference -> packed hex vectors -> SystemVerilog
self-checking testbench -> PASS/FAIL.

## Layout

| File | Purpose |
| --- | --- |
| `rtl/mb_mac_unit.v` | The unit: mb.mac4 + mb.racc with the CFU handshake |
| `rtl/cfu_top.v` | `Cfu` top module CFU Playground expects (wraps the unit) |
| `mbmac/ref.py` | Golden reference (single source of truth for the semantics) |
| `mbmac/vectors.py` | Directed, random, wrap and exhaustive vector generator |
| `tb/tb_mb_mac.sv` | CPU-like testbench with random gaps and `rsp_ready` stalls |
| `mbmac/verify.py` | Generate -> compile (Icarus) -> simulate -> save results |
| `tests/` | Python tests + a cycle-accurate Python model of the RTL logic |
| `soc/proj_menu.cc` | On-SoC C test (Renode) with a software reference |
| `soc/build_cfu.sh` | Installs RTL + menu into a CFU Playground project |

## Proposed semantics (to be frozen in ISA v1 with Ranjith)

| | mb.mac4 rs1, rs2 | mb.racc rd, rs1 |
| --- | --- | --- |
| Encoding | custom-0, funct3 = 0, funct7 = 0 | custom-0, funct3 = 0, funct7 = 1 |
| Effect | acc += sum of four int8 x int8 lane products | rd = sat8((acc + 2^(s-1)) >>> s), s = rs1[4:0]; acc = 0 |
| Result | rd = 0 | sign-extended int8 |
| C call | `cfu_op0(0, rs1, rs2)` | `cfu_op0(1, shift, 0)` |

* Lane 0 = bits [7:0], lane 3 = bits [31:24].
* Accumulator: signed 32-bit, wraps on overflow (needs 2^15 mac4 ops to overflow).
* Rounding: round half up (ties toward +infinity). With s = 0 there is no rounding term.
* Unknown function ids answer 0 and change nothing, so a wrong opcode never hangs the CPU.
* Timing: mac4 responds the cycle after accept; the accumulator is updated 2 edges later.
  racc waits for that, so mac4 followed immediately by racc is correct.

### Open questions for the ISA review
1. **racc output width.** int8 here (next-layer activations). The scan takes 16-bit
   inputs (delta, u); a second variant (funct7 = 2, int16) may be needed. `OUT_BITS` is a parameter.
2. **Rounding mode.** Half-up is cheapest in hardware. The Scan-Fuse contract used floor
   for internal shifts and ties-to-even only for input conversion. Must match Ranjith's C baseline bit for bit.
3. **Per-channel scale / bias** before saturation? Not included.
4. **Throughput.** `cmd_ready` is low while a response is pending, so mac4 issues every 2 cycles
   at best. Allow `cmd_ready = ~busy | rsp_ready` later if the CPU can issue faster.

## Run the RTL tests (WSL)

```bash
sudo apt install -y iverilog          # once
python3 -m unittest discover -s tests -v
python3 -m mbmac.verify --quick       # about 80k commands
python3 -m mbmac.verify               # full: 347,748 commands incl. exhaustive 8x8 sweep in all 4 lanes
```
Expected last line: `PASS mb_mac OUT_BITS=8 records=347748 ...`. On failure the first
FATAL line gives the row number, function id, operands, and got/expected values.

Verilator lint (what CFU Playground uses):
```bash
verilator --lint-only -Wall -Wno-DECLFILENAME rtl/mb_mac_unit.v rtl/cfu_top.v --top-module Cfu
```

## Run it inside the SoC

```bash
cd ~/CFU-Playground/proj && cp -r proj_template_v mb_cfu
~/path/to/mbmac_pkg/soc/build_cfu.sh ~/CFU-Playground/proj/mb_cfu
cd mb_cfu && make renode
```
In the simulator: `3` (Project menu), then `d` (directed), `r` (random), `b` (back-to-back).

## What has and has not been verified

Verified here: the Python reference, the vector generator, and a cycle-accurate Python
transcription of the RTL (all 347,748 commands, three stall patterns; six injected bugs caught).

NOT yet run: the Verilog itself (no simulator was available where this was written).
Run `mbmac.verify` and the lint command above first, and report any compile error or FATAL line.
Also not done: Vivado synthesis (LUT/DSP/timing numbers), and the SoC run.

## Synthesis notes (when Vivado is installed)
Out-of-context synthesis of `mb_mac_unit` gives a first LUT/DSP/slack data point. Four 8x8
multipliers may map to LUTs rather than DSPs; try `(* use_dsp = "yes" *)` on `m0..m3` and compare.
