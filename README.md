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
| `hw/` | Verilog RTL, testbenches, CFU Playground project (MAC, nonlinear, scan engine) | A |
| `test/` | Master regression (`run_all.sh`), directed tests, `compare.py` | D |
| `results/` | `results.csv`, Spike profiling counts, and charts | D |
| `docs/` | CFU handshake summary, token plan report, meeting notes | E |
| `old/` | Earlier prototypes (SPE/SSA RTL, mambax Python, Vivado project), kept for reference | |

`MambaX.ipynb` is the Kaggle notebook that fine-tunes mamba-130m and dumps scan vectors.

## Quick start

```bash
bash test/run_all.sh              # run entire regression: model, Spike, and hardware tests
cd sw && make run                 # bare-metal hello + counter test on Spike (setup: sw/README.md)
make -C sim/spike check           # Xmamba reference C == Python fixed-point model
make -C sw compare                # baseline vs +MAC vs +MAC+NL vs full Xmamba on Spike, bit-exact
cd hw && python3 -m unittest discover -s tests -v  # hardware unit tests
```

- **ISA Specification:** [spec/xmamba-isa.md](spec/xmamba-isa.md)
- **Project Progress:** [progress.md](progress.md)
- **Work Log:** [report.md](report.md)
