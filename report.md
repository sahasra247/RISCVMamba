# Person B (ISA & Software): work report, 3–4 Nov 2026

## In one paragraph

All of Person B's software work for Stages 0, 1 and 2 is done except two items that need Hardware (sending the ISA draft for review, and the joint freeze on 11 Dec). The model runs on a simulated RISC-V processor in three versions (decimal, whole-number, and whole-number with our new Xmamba instructions), and each version is checked automatically against the Python "answer key". The whole-number versions match it **exactly, every bit**. With random weights at the planned model size, the new instructions cut the work per token from **13.5M to 3.2M instructions (4.26× fewer)**. The scan part shrinks 235×. The numbers are still from random weights, so the main thing we need now is Person C's trained model.

---

## 1. The big picture: where this work sits

The project builds new RISC-V instructions (Xmamba) to run a small Mamba model faster, first in a simulator (Spike), then in hardware on an FPGA. Every version of the model is checked against the one above it:

```
Trained model (PyTorch, Person C)            ← not yet available; random weights used meanwhile
        │ same weights
        ▼
Decimal Python  model/mamba_step.py          "answer key" for correctness            [built]
        │                                     ── C decimal engine matches it to 1e-6  [built, PASS]
        ▼
Whole-number Python  model/mamba_fixed.py    exact integer recipe (what hardware does) [built]
        │  must be close to decimal (accuracy loss < 1 point; needs trained model)
        ▼
C whole-number baseline   sw/engine/mamba_q.c      must equal the recipe EXACTLY      [built, PASS]
        ▼
C + Xmamba on Spike   (+MAC, +MAC+NL, full)        must equal the recipe EXACTLY      [built, PASS]
        ▼
Same program on the RTL / SoC simulation (A, Stage 3), then the FPGA board (Stage 4)
```

The speed-up we report is **only** the bottom comparison: whole-number C with and without Xmamba, on the same processor. Python vs C, and decimal vs whole-number, are never counted.

## 2. What was done, step by step, and why

### Stage 0: setup (done 3 Nov)

| What | Where | Why |
|---|---|---|
| Repo layout from the plan, with an owner README per folder; A's MAC package unzipped into `hw/mb_mac/` (unchanged) | all folders | One place for everyone's work; the plan asks for this layout. |
| Bare-metal runtime: start-up code, linker script, console and exit through Spike's HTIF interface, small `printf`, 64-bit cycle/instruction counters | `sw/common/` | Our 32-bit programs cannot use the installed proxy kernel (`pk` is 64-bit only), and bare metal is what the board runs anyway. The same API will get a UART/timer back end on the SoC, so measurement code is written once. |
| Hello-world and counter test | `sw/tests/` | Proves the toolchain, Spike and counters work: the counter check confirms 10× the work gives 10.0× the instructions, and an int8 dot product costs ~11.5 instructions per multiply-add in plain C. |

### Stage 1: golden reference, whole-number baseline, profile

| What | Where | Why |
|---|---|---|
| **Decimal Python model**, one token at a time, using the HF weight names from our notebook. Self-checks against a second, independent formulation. | `model/mamba_step.py` | The plan's golden model and Person C's template. It defines what "correct" means; whole-number versions are measured against it. |
| **Weight export**: packs `.npy` weights into a file the C program links in, plus expected answers for test tokens | `sw/tools/export_weights.py` | The plan says weights are linked in as a blob, with no file system and no malloc on the board. Works with random weights now and with Person C's `.npy` files later (`WEIGHTS=dir`). |
| **Decimal C engine**, checked on Spike against Python after every layer: PASS (worst error 1.2e-6) | `sw/engine/mamba_f32.c` | Proves the C structure (loops, weight layout, state handling) is right *before* rounding enters the picture, so later errors are easier to locate. |
| **Whole-number recipe in Python**: every multiply, shift and rounding spelled out | `model/mamba_fixed.py` | Fast to try formats and measure their error. It is the exact specification the C code, Spike and the RTL must reproduce. |
| **Whole-number C baseline**, required to match the recipe **bit for bit** (every layer output, scan state and logit): PASS | `sw/engine/mamba_q.c` | This is the baseline we accelerate. Bit-exact equality is achievable with integers and turns any bug into a visible mismatch. |
| **Per-kernel profile** (instructions per token) | printed by `make -C sw mamba-fixed` | The plan says the profile decides the ISA. It did: linear layers ~66%, scan ~30%. |

Decisions in Stage 1 and their justification:
- **Number formats** (`spec/xmamba-isa.md` §3). 8-bit weights and activations for the matrix multiplies, as the plan's Decision 3 specifies and as Person A's MAC unit is built. Wider formats in the scan (Δ 24 fractional bits, decay 30, state 20), because small errors there build up from token to token.
- **The error measured, and where it comes from.** Against the decimal model the worst single value is off by 6–9%, with the same top token 45 of 48 times. Testing each part separately showed the lookup tables add nothing measurable. The error is the 8-bit numbers, half from activations and half from weights; using 16 bits for both drops it to ~0.1% but overflows 32-bit adders. Whether 8-bit is acceptable is decided by Person C's accuracy test on the trained model (plan: < 1 point lost). The plan's fallback (16-bit activations in sensitive layers) is one setting in `mamba_fixed.py`.
- **Rounding = round-half-up**, the same as Person A's RTL, so software and hardware give the same bits.
- **A fair baseline.** One refactor made the baseline 20% slower (shared helpers were no longer inlined). I fixed it, because a slowed baseline would inflate the speed-up. The baseline is now faster than before the refactor.

### Stage 2: ISA, Spike model, Xmamba builds

| What | Where | Why |
|---|---|---|
| **ISA spec v1 draft**: 11 instructions, encodings, operand packing, number formats, rounding/saturation rules, unit state and memory size, latency targets, open questions for A | `spec/xmamba-isa.md` | The contract between hardware and software; Spike and the RTL must produce the same bits. |
| **Single source of truth** `xmamba_ref.c`: what every instruction computes | `sim/spike/xmamba_ref.c` | The plan asks for it: Spike calls it, the baseline uses its maths, and RTL test vectors can be generated from it. |
| **C = Python proof** on ~200k inputs per function (edges, table boundaries, extremes) and 3,000 scan updates: PASS | `sim/spike/check_ref.py`, `make -C sim/spike check` | So nothing in C silently differs from the recipe. |
| **Spike extension**, loaded as a plug-in, with readable disassembly (`mb.scan a4, a4, a1` in traces) | `sim/spike/xmamba_spike.cc` | Runs real programs with the new instructions without modifying Spike. |
| **Intrinsics** `xmamba.h` and **one engine, four builds**: baseline, +MAC, +MAC+nonlinear, full Xmamba | `sw/engine/xmamba.h`, `mamba_q.c` | The four builds are exactly the ablation table the paper section asks for (it separates Mamba-specific gains from generic ML gains). |
| **Instruction-count table**, all four builds bit-exact | `results/spike_instr_counts_d256_l4.csv`, `make -C sw compare` | The plan's Stage 2 deliverable. |

Results at the planned size (d=256, 4 layers, 1.82M parameters, random weights):

| Build | Instructions per token | vs baseline |
|---|---:|---:|
| Software baseline (RV32IM, whole numbers) | 13,541,056 | 1.00× |
| + MAC unit (`mb.mac4`, `mb.racc`) | 7,629,887 | 1.77× |
| + nonlinear unit (`mb.exp`, `mb.silu`, `mb.softplus`) | 4,786,727 | 2.83× |
| Full Xmamba (+ scan engine) | 3,177,371 | **4.26×** |

Per kernel: the scan drops 235× (3.99M → 17K) because h, A and D stay inside the unit. The linear layers drop ~3× and are now 93% of what remains. They are limited by moving operands through registers (two loads per `mb.mac4`), which the plan predicted ("once the multiplies are fast, memory bandwidth becomes the limit"). The spec lists v2 ideas for this.

Changes from the plan's first instruction table (v0), with reasons:
- **`mb.racc` returns 32 bits and accepts a signed shift.** Activations stay in Q16 between layers, and a left shift does occur (measured −1…+4).
- **`mb.scan` takes Δ and u in full registers**, with an auto-incrementing channel counter set by `mb.layer`. Δ needs 24 fractional bits, so it can't be packed in 16.
- **`mb.ldbc` loads one 32-bit value** (B or C), not a 16-bit pair, for the same precision reason.
- **New `mb.rdh` (read h)**, used by tests so the scan state can be checked bit for bit.
- **Scan memory sized at 2,048 channels**, which fits the planned model exactly: 264 KiB of the board's 607.5 KiB block RAM.

Extra check: I ran **Person A's own RTL tests** on a scratch copy (iverilog is installed here). Their README said the Verilog had never been simulated. Result: **the full 347,748-command test PASSES** and the unit tests pass. Verilator lint (`-Wall`) stops on 2 cosmetic warnings (listed for A below). A's files were not changed.

## 3. How it was verified (all re-run from a clean build on 4 Nov)

| Check | Command | Result |
|---|---|---|
| Decimal Python self-check | `python3 model/mamba_step.py` | PASS |
| Whole-number Python: table errors, error vs decimal, overflows | `python3 model/mamba_fixed.py` | exp 1.5e-5, SiLU 3.3e-4, softplus 7.6e-4; no int32 overflows |
| C reference = Python reference | `make -C sim/spike check` | PASS, all 5 functions + scan |
| Hello + counters on Spike | `make -C sw run` | PASS |
| Decimal C engine vs Python | `make -C sw mamba-float` | PASS |
| Four builds, bit-exact, counts | `make -C sw compare [D_MODEL=256 N_LAYER=4]` | PASS, 0 mismatches, both sizes |
| Builds | `make -C sw all`, `make -C sim/spike` | 0 compiler warnings |

Tests were also checked to catch real mistakes: deliberately broken copies (0.1% error in the scan, reversed conv weights, no decay) all fail.

## 4. Limitations

- **All numbers use random weights.** Accuracy, the error of 8-bit arithmetic, and even the instruction split may change with the trained model. Everything reruns with one command once the weights exist.
- **These are instruction counts, not cycles.** Spike is functional. Cycles (and tokens per second) come from the SoC simulation and the board, where multi-cycle instructions stall the CPU.
- **The ISA is a draft.** Latency targets and some choices (§9 of the spec) need Hardware's review before the 11 Dec freeze.
- **The C baseline is reasonably optimized but not fully tuned** (Stage 4 has a tuning task); the same tuning must then go into both builds.
- **Git state:** Stage 0 is committed and pushed (`b/stage0-baremetal`); the NumPy model is committed locally (`d696321`); everything since is **uncommitted** in the worktree `.claude/worktrees/b-stage0-baremetal`, as requested.

## 5. What I need from each teammate

### Person A (Hardware)
1. **Review `spec/xmamba-isa.md`**, especially §8 (latency targets) and §9 (open questions). Reply by the 20 Nov ISA gate so we can freeze on 11 Dec.
2. **Change `mb.racc`** to the v1 semantics: 32-bit output (`OUT_BITS = 32`), signed 6-bit shift `rs1[5:0]` with left shift for negative values, clamped to ±31. Encodings stay the same.
3. **Decide on two hardware-cost questions before the freeze** (they change the bits): store A·log₂e instead of A (saves one multiplier per scan step), and whether A can be 16-bit (saves 64 KiB BRAM).
4. **Fix 2 Verilator lint warnings** (lint fails under `-Wall`): unconnected `dbg_acc` port in `rtl/cfu_top.v:25`, unused bit `sat[32]` in `rtl/mb_mac_unit.v:100`. The functional tests already pass.
5. **mac4 throughput:** can `cmd_ready` allow one `mb.mac4` per cycle? Linear layers are now 93% of the remaining work.
6. **Tell me when CFU Playground builds and simulates the template.** My Stage 3 task (porting the engine to the SoC: start-up, UART, timers, DDR weights) starts from that.

### Person C (Model & Reference)
1. **The trained small model** (most important): about d=256, 4 layers, state size 16, ≤ 5M parameters, on the task the team picks by 20 Nov.
2. **Weights as one `.npy` per tensor**, named with the HF names (`backbone.layers.0.mixer.in_proj.weight`, …), plus the HF `config.json`. My scripts take that folder directly (`WEIGHTS=dir`).
3. **Finish `model/mamba_step.py`**: `load_weights()`, and the comparison with PyTorch on 20 sequences (error < 1e-4).
4. **The accuracy script** (plan Stage 1), so we can measure float vs whole-number accuracy. This decides whether 8-bit is good enough.
5. **The range of the trained A values** (`-exp(A_log)`) and of Δ, to confirm the formats and the A-in-16-bit question.

### Person D (Testing & Benchmarks)
1. **Reproduce the setup** from `sw/README.md` on your laptop and fix any unclear step (plan Stage 0).
2. **Add my checks to `run_all.sh`**: `make -C sim/spike check`, `make -C sw run`, `make -C sw mamba-float`, `make -C sw compare`. They already print PASS/FAIL and return non-zero on failure.
3. **Directed tests per instruction** (Stage 2) from `libxmamba_ref.so`; `sim/spike/check_ref.py` shows how to call it from Python. Include zero, max/min and saturation cases.
4. **Create `results/results.csv`** and import `results/spike_instr_counts_d256_l4.csv` into it.

### Person E (Documentation & Coordination)
1. **Record the decisions** in the weekly notes: retrain a < 5M model; 8-bit linear layers with 16/24/20/30-bit scan formats; round-half-up; ISA v1 changes (racc 32-bit, scan operands, ldbc, rdh, 2,048-channel unit); the speed-up is measured only against the whole-number C baseline.
2. **Typeset `spec/xmamba-isa.md`** as the ISA chapter (Stage 2), and use the tables in §1 of it and §2 here for the first-review slides.
3. **Diagrams:** the verification chain in §1 of this report, and the Xmamba unit (MAC, nonlinear, scan engine with on-chip h/A/D).

## 6. Next steps for Person B

1. **Send the ISA draft to Person A** (20 Nov gate) and update it from the review.
2. **Rerun everything with the trained weights** as soon as Person C delivers. Check accuracy and formats, and refresh the instruction counts.
3. **Stage 3:** port the engine to the CFU Playground SoC (UART output, timers, weights in DDR) and build the cycle-measurement harness and demo app, once A has the SoC template running.
4. **Possible v2 work** for the linear-layer bottleneck (spec §9.7), only if the timeline allows.

## How to reproduce everything

```bash
cd .claude/worktrees/b-stage0-baremetal        # until merged
python3 model/mamba_step.py && python3 model/mamba_fixed.py
make -C sim/spike check
make -C sw run && make -C sw mamba-float
make -C sw compare D_MODEL=256 N_LAYER=4       # ~3 min; prints the table above
```
