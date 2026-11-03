# RISCVMamba / Xmamba

A custom RISC-V ISA extension (Xmamba) for Mamba inference, proven on Spike,
implemented in Verilog on a VexRiscv core (CFU Playground), and demoed on an
Arty A7-100T. See [Milestones.md](Milestones.md) for the plan and owners.

## Layout

| Directory | Contents | Owner |
| --- | --- | --- |
| `model/` | Weight export, NumPy reference, `gen_luts.py`, accuracy script | C |
| `spec/` | `xmamba-isa.md`, the hardware/software contract | B |
| `sw/` | Bare-metal C runtime, inference engine, `xmamba.h`, kernels | B |
| `sim/spike/` | Spike extension, `xmamba_ref.c` | B |
| `hw/` | RTL, testbenches, CFU Playground project (`hw/mb_mac/`: mb.mac4 + mb.racc) | A |
| `test/` | Test vectors, directed tests, `run_all.sh`, `compare.py` | D |
| `results/` | `results.csv` and charts | D |
| `docs/` | Report, slides, figures, meeting notes | E |
| `old/` | Earlier prototypes (SPE/SSA RTL, mambax Python, Vivado project), kept for reference | |

`MambaX.ipynb` is the Kaggle notebook that fine-tunes mamba-130m and dumps scan vectors.

## Quick start

```bash
cd sw && make run      # bare-metal hello + counter test on Spike (setup: sw/README.md)
```
