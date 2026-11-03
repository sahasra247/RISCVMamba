# Xmamba: Complete Token Bring-Up on CFU Playground

This package now includes a complete fixed-point recurrent token pipeline and a
CFU implementing MAC, nonlinear and selective-scan instructions. It implements
the single-token simulation bring-up portion of [the project plan](../Milestones.md).

**Model status:** the folder contains no trained checkpoint, tokenizer or model
card. The demo therefore uses a deterministic synthetic fixture: width 4, two
layers, expansion 2, four scan states, convolution width 4, delta rank 2 and
vocabulary size 8. This tests the execution pipeline, not trained-model accuracy.
It is a bring-up ISA contract, not the plan's frozen ISA v1.

## Run a Token

From this directory, with Python 3, g++, Icarus Verilog and Verilator installed:

```bash
python3 -m unittest discover -s tests -v
python3 -m mbmac.verify_token --verilator
```

The second command checks the independent Python instruction oracle, compiles
and runs C++ inference, replays its commands against RTL with response stalls,
and runs inference again with **live Verilator responses**. It compares all
intermediate stage vectors, not just the predicted token. Two ten-token
sequences check recurrent execution and reset reproducibility. Without
`--verilator`, live co-simulation is omitted but Icarus RTL replay still runs.
`IVERILOG`, `VVP`, `VERILATOR` and `CXX` can select non-PATH executables.

The initial input is token ID 1. Its expected logits (signed Q4 integers) are:

```text
-13 -18 -32 19 22 -10 -13 -29
next token: 4
```

Results and command vectors are written under `build/token/`. The JSON summary
explicitly distinguishes CFU co-simulation from whole-SoC execution.

## Run on the RISC-V SoC

Use a fully installed [CFU Playground](https://cfu-playground.readthedocs.io/)
checkout with its RISC-V toolchain, LiteX dependencies and Renode integration.
Activate its environment first. From this package directory:

```bash
export CFU_ROOT="$HOME/CFU-Playground"
cp -r "$CFU_ROOT/proj/proj_template_v" "$CFU_ROOT/proj/xmamba"
bash soc/build_cfu.sh "$CFU_ROOT/proj/xmamba"
cd "$CFU_ROOT/proj/xmamba"
make renode
```

Use a fresh destination project; the installer replaces its CFU and project
menu. It installs one self-contained `cfu.v`, the token implementation, the
intrinsics, and the generated tables. Upstream automatically discovers the
additional `.cc` source. Do not define `CFU_SOFTWARE_DEFINED`: the token demo
deliberately requires real custom instructions instead of a software fallback.

In the UART menu choose `3` (Project menu), then:

| Key | Action |
| --- | --- |
| `t` | One full token, software/CFU stage comparison, logits, next token and profiling |
| `s` | Ten recurrent tokens followed by a sequence-reset replay |
| `d`, `r`, `b` | Existing MAC directed, random and back-to-back tests |

Success markers are `PASS XMAMBA SINGLE TOKEN` and
`PASS XMAMBA SEQUENCE AND RESET`. A mismatch reports layer, stage and element.
The `t` action prints CSV columns for software/CFU cycles and retired
instructions by kernel and for the complete token. Timed sections exclude UART
printing and weight initialization. Per-kernel timing excludes trace copies;
total timing includes trace collection and dispatch overhead. CSR reads use
RV32 high/low/high sampling. Host tests do not present host time or CFU clock
ticks as RISC-V cycles.

The software path is a correctness baseline using an instruction-level oracle,
not the optimized performance baseline required for the final project. Do not
claim an accelerator speed-up from these prototype measurements alone.

## Token Dataflow

Embedding lookup -> two blocks of RMSNorm, in projection, causal depthwise
convolution + SiLU, x projection, delta projection + softplus, selective scan,
SiLU gate multiplication, output projection and residual addition -> final
RMSNorm -> vocabulary projection -> deterministic argmax (lowest index wins ties).

Convolution history lives in CPU memory. Per-layer h, A, D, B and C live in the
CFU. `Pipeline::reset_sequence()` clears both convolution history and CFU h;
clearing only one would contaminate a new sequence. The implementation uses
fixed-size arrays, no heap allocation, and integer arithmetic throughout.

## Bring-Up ISA Contract

All operations use R-type custom-0, opcode `0x0b`. The CFU function ID is
`(funct7 << 3) | funct3`. Lane 0 and packed low halves occupy the least significant bits.

| Operation | funct3 | funct7 | Operands and result |
| --- | --- | --- | --- |
| mb.mac4 | 0 | 0 | Accumulate four signed int8 products; return 0 |
| mb.racc | 0 | 1 | Round/shift by rs1[4:0], saturate to int8, sign extend, clear acc |
| mb.exp | 1 | 0 | Signed Q8 input to Q8 exp, clamped to [0, 256] |
| mb.silu | 1 | 1 | Signed Q8 input/output |
| mb.softplus | 1 | 2 | Signed Q8 input/output |
| mb.lda | 2 | 0 | rs1 = constant address, rs2[15:0] = signed Q8 value |
| mb.layer | 2 | 1 | rs1 selects layer 0 or 1 |
| mb.ldbc | 2 | 2 | rs1 = state index; rs2[15:0] = B, rs2[31:16] = C |
| mb.scan | 2 | 3 | rs1[15:0] = delta, rs1[31:16] = u; rs2 = channel; return y |
| mb.rst | 2 | 4 | Clear h in every layer; preserve constants, B/C, layer and MAC acc |

All delta/u/B/C/A/D operands are signed 16-bit integers with eight fractional
bits. h, y and nonlinear values are signed 32-bit integers with eight
fractional bits. This is a prototype format choice, not a trained-model
quantization decision. In the selected layer, A uses addresses
`channel * 4 + state` (0..31); D uses `32 + channel` (32..39).
Out-of-range addresses, layers and channels, and unknown function IDs return 0
without changing architectural state. Non-result operations return 0.

`qmul(a,b) = sat32(floor(a*b/256))` with a signed 64-bit product;
`qadd(a,b) = sat32(a+b)` with a widened sum. Each scan state executes, in order:

```text
decay = exp(qmul(delta, A[channel,state]))
injection = qmul(qmul(delta, B[state]), u)
h = qadd(qmul(decay, h), injection)
sum = qadd(sum, qmul(C[state], h))
y = qadd(sum, qmul(D[channel], u))
```

`sum` starts at zero for every mb.scan. Saturation occurs at each indicated
operation, not just the final result. Scan accepts signed delta; normal model
execution produces nonnegative delta via softplus and loads negative A.

Nonlinear tables have half-unit knots with nearest-integer table entries
(ties toward positive infinity) and floor-rounded interpolation. exp uses
[-8, 0], returning 0 below -8 and 256 above 0. SiLU and softplus use [-8, 8],
returning 0 below -8 and the input above 8. Measured absolute approximation
error is below 0.03 on all Q8 inputs within those intervals. Generate or check
the shared C/Verilog tables with `python3 -m mbmac.xmamba [--check]`.

Linear weights are signed int8 with six fractional bits, activations int8
with four. Each dot product uses mb.racc shift 6. Projection outputs are
saturated before the next operation; convolution bias is added after its
dot-product saturation. Q8 -> Q4 uses add 8, floor divide by 16, then sat8.
RMSNorm uses integer sqrt of `mean(input_raw**2) + 1`, Q8 gamma, division
toward zero and int8 saturation. Residuals are also saturated to int8.
The deterministic fixture generator in `soc/xmamba_token.h` is deliberately
separate from any future checkpoint exporter.

The top level allows one outstanding command, latches response ownership and
holds responses under backpressure. Scan takes three clocks per state plus a
completion clock after acceptance. mb.rst clears one h element per clock.
Synchronous hardware reset cancels pending operations and clears all unit
state, including constants, B/C and MAC acc.

## Verification Status and Limits

Verified in this workspace: Python tests; Icarus instruction and token-command
regressions at 0% and 70% response stalls; live Verilator execution of 20 tokens
(6,126 CFU commands), with bit-identical stage traces and logits; strict RV32IM
object compilation of the token application and installed project menu using
upstream CFU headers; installation into a fresh upstream template; and lint
of the exact installed RTL.

**Not verified:** whole-SoC execution, linked SoC firmware, FPGA synthesis or
timing, Spike integration, trained-model output/accuracy, or DDR placement.
The local whole-SoC launch requires the full CFU Playground/LiteX/Renode
environment, which was not installed. Live CFU co-simulation is not a CPU run.
The scan prototype uses small resettable register arrays and a multi-cycle
datapath; it is not the plan's pipelined BRAM engine and must not be scaled to
a real model without a memory/timing redesign and resource budget.

To move beyond this fixture, supply the trained checkpoint and model card,
define export scales and accuracy bounds, adapt both model dimensions and
RTL storage, then validate against the trained floating-point reference.

## Original MAC Unit

Hardware side of the MAC unit for the RISCVMamba (Xmamba) CFU. Same flow as the
Scan-Fuse workspace: Python reference -> packed hex vectors -> SystemVerilog
self-checking testbench -> PASS/FAIL.

## Layout

| File | Purpose |
| --- | --- |
| `rtl/mb_mac_unit.v` | The unit: mb.mac4 + mb.racc with the CFU handshake |
| `rtl/cfu_top.v` | `Cfu` top module with MAC/nonlinear/scan routing |
| `rtl/mb_xmamba_unit.v` | Nonlinear and multi-layer selective-scan RTL |
| `mbmac/xmamba.py` | Independent instruction oracle and LUT generator |
| `mbmac/verify_token.py` | Token pipeline and full-CFU regression runner |
| `soc/xmamba_token.h` | Fixed-size recurrent token engine and fixture |
| `soc/xmamba_token.cc` | SoC comparison and profiling application |
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
verilator --lint-only -Wall -Wno-DECLFILENAME -Wno-PINCONNECTEMPTY -Wno-UNUSED \
   -Irtl rtl/mb_mac_unit.v rtl/mb_xmamba_unit.v rtl/cfu_top.v --top-module Cfu
```

## Run it inside the SoC

```bash
cd ~/CFU-Playground/proj && cp -r proj_template_v mb_cfu
~/path/to/mbmac_pkg/soc/build_cfu.sh ~/CFU-Playground/proj/mb_cfu
cd mb_cfu && make renode
```
In the simulator: `3` (Project menu), then `d` (directed), `r` (random), `b` (back-to-back).

The original MAC tests remain available. See the verification status above
for the distinction between RTL validation and whole-SoC/FPGA validation.

## Synthesis notes (when Vivado is installed)
Out-of-context synthesis of `mb_mac_unit` or `Cfu` gives a first LUT/DSP/slack data point. Run:
```bash
cd synth && vivado -mode batch -source synth.tcl
```
Four 8x8 multipliers may map to LUTs rather than DSPs; try `(* use_dsp = "yes" *)` on `m0..m3` and compare.

## Demo & Verification Suite

The repository includes a complete demo dashboard, automated test runner, and trace comparator:

| Component | Path | Description |
| --- | --- | --- |
| **Monolithic CFU** | `rtl/cfu.v` | Self-contained, inlined CFU Verilog ready for CFU-Playground |
| **CFU Builder** | `tools/generate_cfu_v.py` | Inlines tables and bundles RTL without requiring Linux tools |
| **Demo Dashboard** | `demo/demo_dashboard.py` | UART serial monitor, offline simulation, terminal metrics and bar charts |
| **Renode Runner** | `scripts/run_renode_demo.py` | Headless automated simulation and UART log capture |
| **Regression Suite**| `test/run_all.py` / `test/run_all.sh` | One-command validation across unit tests, tables, and RTL |
| **Trace Comparator**| `test/compare.py` | Bit-exact trace verification across Python, Spike, and SoC |
| **Synthesis Tcl** | `synth/synth.tcl` | Vivado Artix-7 100T out-of-context synthesis and resource report |
| **Handshake Spec** | `docs/cfu_handshake_summary.md` | CPU-CFU timing, pinouts, stall behavior and ISA encodings |
| **Results Sheet** | `results/results.csv` | Benchmark and resource utilization tracking table |

### Running the Demo Dashboard
To launch the demo with mock hardware simulation / offline fallback:
```bash
python demo/demo_dashboard.py --demo-mock --save-plots
```
To connect to an active Arty A7-100T FPGA board over serial UART:
```bash
python demo/demo_dashboard.py --port COM3 --baud 115200   # Windows
python demo/demo_dashboard.py --port /dev/ttyUSB1 --baud 115200  # Linux
```

