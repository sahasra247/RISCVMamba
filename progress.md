# Xmamba: Project Progress & Status Report (`progress.md`)

**Repository:** `riscv-mamba`  
**Current Date:** 4 November 2026  
**Target Demo Date:** 26 February 2027  
**Platform:** VexRiscv RV32IM CPU + Xmamba CFU on Arty A7-100T FPGA (CFU Playground)

---

## 1. Executive Summary

This document summarizes the current status of the **Xmamba** project across all workstreams and team members (A through E).

Both major development tracks have been merged into the `main` branch (commit `a804fc7`):
1. **Hardware Track (`main` / Member A):** A complete single-token prototype running on CFU Playground with Verilog RTL for the MAC, nonlinear lookup, and selective-scan units, verified via Icarus Verilog and live Verilator co-simulation with a synthetic test fixture.
2. **Software & ISA Track (`b/stage0-baremetal` / Member B):** Stages 0, 1, and 2 software deliverables are complete. This includes the canonical repository structure, bare-metal RV32IM runtime for Spike (no proxy kernel needed), decimal and fixed-point Python golden models, bit-exact C inference engines, the complete ISA spec v1 draft, a Spike extension simulator, and instruction count profiling showing a **4.26× total instruction reduction** and **235× scan reduction**.

**Current Blocker / Critical Path:** All software and hardware pipelines currently run on synthetic or random weights. An **actual trained model** ($\le 5\text{M}$ parameters) from Member C is the primary dependency required to validate real-world task accuracy and finalize fixed-point quantization.

---

## 2. Overall Stage Status

| Stage | Milestone / Focus | Planned Dates | Status | Completion |
|---|---|---|:---:|:---:|
| **Stage 0** | **Setup & Toolchain Onboarding** | 2–8 Nov 2026 | **On Schedule** | ~85% |
| **Stage 1** | **Golden Reference, Quantization & Profiling** | 9–20 Nov 2026 | **Ahead of Schedule** | ~80% |
| **Stage 2** | **ISA Spec Draft & Spike Emulation** | 23 Nov – 11 Dec 2026 | **Ahead of Schedule** | ~75% |
| **Stage 3** | **RTL Implementation & SoC Simulation** | 23 Nov 2026 – 22 Jan 2027 | **In Progress** | ~40% |
| **Stage 4** | **FPGA Bring-up (Arty A7-100T)** | 4 Jan – 12 Feb 2027 | **Pending Board** | 0% |
| **Stage 5** | **Evaluation, Freeze & Final Submission** | 15–26 Feb 2027 | **Planned** | 0% |

---

## 3. Person-by-Person Task Breakdown

The team consists of five roles (Members A through E) as defined in `Milestones.md`:

### Member A: Hardware (RTL & FPGA)
*Works most with: B (ISA & Software)*

- **Completed:**
  - Implemented the MAC unit (`hw/mb_mac/rtl/mb_mac_unit.v`) supporting `mb.mac4` (four int8 multiply-accumulate) and `mb.racc` (rescale and saturate accumulator).
  - Built exhaustive SystemVerilog testbench (`tb_mb_mac.sv`) passing **347,748 vector regression tests**.
  - Built prototype full CFU (`cfu.v`) implementing MAC, nonlinear table lookups, and the stateful scan engine.
  - Implemented single-token SoC simulation using CFU Playground, Renode, and Verilator (`token_test.cc`, `verify_token.py`).
  - Documented the CPU–CFU handshake interface timing in `CFU_playground/CFU_playground/mbmac_pkg/docs/cfu_handshake_summary.md`.
- **Remaining Tasks:**
  - **Review `spec/xmamba-isa.md`** (§8 latency targets & §9 open architectural questions) before the **20 Nov gate**.
  - Update `mb.racc` RTL to ISA v1 semantics: 32-bit return value (`OUT_BITS=32`), signed 6-bit shift `rs1[5:0]` supporting left-shifting.
  - Settle scan architecture trade-offs: storing $A \cdot \log_2 e$ directly (saves a DSP multiplier) and whether $A$ can be 16-bit (saves 64 KiB BRAM).
  - Resolve 2 Verilator `-Wall` lint warnings (`dbg_acc` in `cfu_top.v`, unused `sat[32]` in `mb_mac_unit.v`).
  - Synthesize the stock VexRiscv SoC for the XC7A100T in Vivado to budget resources (LUTs, DSPs, BRAM) and clock frequency.
  - **Submit lab board request for Arty A7-100T by 27 Nov**.

---

### Member B: ISA & Software
*Works most with: A (Hardware)*

- **Completed:**
  - Built bare-metal C runtime (`sw/common/crt0.S`, `link.ld`, `bm_spike.c`, `bm.h`) for RV32IM on Spike using HTIF (no proxy kernel `pk` required).
  - Implemented decimal Python single-token reference (`model/mamba_step.py`) and decimal C engine (`sw/engine/mamba_f32.c`), matching within $1.2 \times 10^{-6}$.
  - Formulated the exact integer quantization recipe (`model/mamba_fixed.py`) and bit-exact C baseline engine (`sw/engine/mamba_q.c`).
  - Authored the 11-instruction **ISA spec v1 draft** (`spec/xmamba-isa.md`).
  - Implemented single source of truth `sim/spike/xmamba_ref.c` and verified it against Python across 200,000+ points.
  - Built the Spike simulator extension (`sim/spike/xmamba_spike.cc`) providing custom instruction emulation and readable disassembly traces.
  - Implemented C intrinsics (`sw/engine/xmamba.h`) and automated instruction profiling on Spike across 4 build configurations.
- **Remaining Tasks:**
  - Collaborate with Member A to freeze ISA v1 by **11 Dec**.
  - Port the bare-metal C inference engine to the CFU Playground SoC (UART console, timers, weight mapping in DDR3).
  - Integrate real weights from Member C into `sw/tools/export_weights.py` and refresh benchmark tables.

---

### Member C: Model & Reference *(Critical Path)*
*Works most with: B (ISA & Software)*

- **Completed:**
  - Kaggle fine-tuning notebook (`MambaX.ipynb`) created and saved in repository.
  - Verification of mathematical recurrence formulas on random weights.
- **Remaining Tasks:**
  - **Train or select the small model** ($d=256$, 4 layers, $N=16$, $\le 5\text{M}$ parameters) on the evaluation task chosen by **20 Nov**.
  - Complete the **model card** in `Milestones.md` (parameter count, task, target accuracy).
  - Export weights to per-tensor `.npy` files with Hugging Face naming conventions, along with `config.json`.
  - Connect `load_weights()` in `model/mamba_step.py` and verify against PyTorch on 20 test sequences (error $< 10^{-4}$).
  - Implement the **accuracy script** to score float vs. fixed-point outputs on 100 test samples (confirming accuracy degradation $< 1\%$).
  - Measure dynamic ranges of trained $A$ ($- \exp(A_{\text{log}})$) and $\Delta$ to confirm fixed-point bit allocation.
  - **Freeze weights by 11 Dec**.

---

### Member D: Testing & Benchmarks
*Works most with: A (Hardware)*

- **Completed:**
  - Unit tests for MAC golden reference and cycle-accurate RTL model (`hw/mb_mac/tests/`).
  - Comparison scripts and initial results table (`CFU_playground/mbmac_pkg/results/results.csv`).
  - Instruction-counting analysis tools (`sw/tools/compare_counts.py`).
- **Remaining Tasks:**
  - Verify that the toolchain setup from `sw/README.md` reproduces cleanly on a second workstation.
  - Build the unified master test script (`test/run_all.sh`) executing Python, C/Spike, and RTL simulation in a single command.
  - Write directed edge-case tests per instruction (zeros, max/min, saturation, boundary lookup values).
  - Maintain the consolidated results spreadsheet.

---

### Member E: Documentation & Coordination
*Works most with: Everyone*

- **Completed:**
  - Roadmap, milestone gates, and risk management plan compiled in `Milestones.md`.
  - Literature survey comparing Xmamba to MARCA, LightMamba, Mamba-X, and FEMBA.
  - Recorded Person B work log in `report.md`.
- **Remaining Tasks:**
  - Coordinate selection of the evaluation task/dataset by **20 Nov**.
  - Set up the Overleaf / LaTeX report template with chapter outlines and shared bibliography.
  - Typeset `spec/xmamba-isa.md` into the formal project report ISA chapter.
  - Redraw architecture block diagrams (CPU-to-CFU datapath, scan state engine).
  - Prepare presentation slide deck for the first project review.

---

## 4. Key Performance Results (Spike Emulation)

Measurements gathered for a $d=256$, $L=4$, $N=16$, $E=2$, $R=16$ model (1.82M parameters, random weights):

| Build Variant | Instructions / Token | vs. Baseline | Scan Kernel Work |
|---|---:|:---:|---:|
| **1. Baseline RV32IM Software (Fixed-Point)** | 13,541,056 | 1.00× | 3,993,400 instrs |
| **2. + MAC Unit (`mb.mac4`, `mb.racc`)** | 7,629,887 | 1.77× | 3,993,400 instrs |
| **3. + Nonlinear Unit (`mb.exp`, `mb.silu`, `mb.softplus`)** | 4,786,727 | 2.83× | 1,624,928 instrs |
| **4. Full Xmamba (+ Stateful Scan Engine)** | **3,177,371** | **4.26×** | **17,008 instrs** |

### Per-Kernel Breakdown:
- **Selective Scan:** Drops by **235×** ($3.99\text{M} \to 17\text{K}$ instructions) because states $h$, $A$, and $D$ remain inside unit BRAM.
- **Linear Layers (`in_proj`, `out_proj`, `x_proj`, `dt_proj`):** Drop by ~3× and now account for **93% of remaining instructions**. The bottleneck is memory/register bandwidth (fetching weights into registers for `mb.mac4`).

---

## 5. Repository Structure & Branch Integration

The repository layout follows the canonical structure specified in `Milestones.md`:

```
riscv-mamba/
├── model/           # Reference models: mamba_step.py (float), mamba_fixed.py (integer)
├── spec/            # Hardware/software interface contract: xmamba-isa.md
├── sw/              # Bare-metal C runtime, engines (mamba_f32.c, mamba_q.c), intrinsics
├── sim/spike/       # Spike extension (xmamba_spike.cc) and C reference (xmamba_ref.c)
├── hw/mb_mac/       # MAC unit Verilog, testbench, and verification scripts
├── CFU_playground/  # Single-token prototype package, Renode demo, Verilator co-sim
├── test/            # Test scripts and compare utilities
├── results/         # Results logs and instruction count CSVs
├── docs/            # Project documentation and interface summaries
├── Milestones.md    # Master milestone plan, reading list, and task checklist
├── progress.md      # This progress tracking document
└── report.md        # Work log for Stages 0, 1, and 2
```

Both `origin/main` and `origin/b/stage0-baremetal` are unified in `main` (merge commit `a804fc7`).

---

## 6. How to Run & Verify All Tests

### 1. Python Models & Fixed-Point Verification
```bash
python3 model/mamba_step.py      # Float recurrent step self-check (PASS)
python3 model/mamba_fixed.py     # Integer/Q-format recipe & error analysis (PASS)
```

### 2. Spike Simulator Extension & C Reference Checks
```bash
make -C sim/spike check          # Verifies xmamba_ref.c vs Python on 200k+ inputs (PASS)
```

### 3. Bare-Metal RV32IM C Engines on Spike
```bash
make -C sw run                   # Runs hello world and cycle/instret counter tests (PASS)
make -C sw compare               # Runs all 4 builds on Spike and prints ablation table (PASS)
```

### 4. Hardware Unit Tests & RTL Simulation
```bash
cd hw/mb_mac && python3 -m unittest discover -s tests -v   # MAC unit tests (PASS)
cd CFU_playground/CFU_playground/mbmac_pkg && python3 -m unittest discover -s tests -v # Prototype tests (PASS)
```

---

## 7. Critical Upcoming Gates & Deadlines

```
20 Nov 2026:  [GATE 1] Evaluation task chosen (all) & ISA v0 draft review completed (Member A)
27 Nov 2026:  [GATE 2] Arty A7-100T FPGA board request submitted to lab (Member A)
11 Dec 2026:  [GATE 3] ISA v1 frozen (A & B) & Model weights frozen (Member C)
22 Jan 2027:  [GATE 4] Single token running on simulated CFU Playground SoC (A & B)
12 Feb 2027:  [GATE 5] Full model running on the Arty A7-100T FPGA board
26 Feb 2027:  [FINAL DEMO] Project demo-ready freeze
```
