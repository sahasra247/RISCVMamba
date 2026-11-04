# hw/ — Xmamba Hardware (CFU Playground Project)

Verilog RTL, testbenches, software drivers, synthesis scripts, and Python models for the Xmamba Custom Function Unit (CFU).  
**Owner:** Member A (Hardware — RTL & FPGA).

## Directory Structure

```
hw/
├── rtl/               # Verilog hardware design
│   ├── cfu.v          # Monolithic, self-contained CFU (MAC + NL + Scan) with inlined tables
│   ├── cfu_top.v      # Top-level CFU wrapper routing to MAC or Xmamba units
│   ├── mb_mac_unit.v  # MAC unit (mb.mac4, mb.racc)
│   ├── mb_xmamba_unit.v # Selective scan & nonlinear unit
│   └── xmamba_tables.vh # Piecewise-linear LUTs for exp, silu, softplus
├── tb/                # SystemVerilog testbenches
│   └── tb_mb_mac.sv   # Self-checking testbench with random stalls & response checks
├── soc/               # CFU Playground SoC integration
│   ├── build_cfu.sh   # Installer script for CFU Playground projects
│   ├── proj_menu.cc   # Interactive UART menu (single token 't', 10-token 's')
│   ├── xmamba_token.cc/h # C++ token execution and comparison harness
│   └── xmamba.h / xmamba_ref.h / xmamba_tables.h # Intrinsics, oracle, tables
├── mbmac/             # Python reference & verification modules
│   ├── ref.py         # Golden arithmetic reference for mb.mac4 / mb.racc
│   ├── vectors.py     # MAC test vector generator (347k commands)
│   ├── verify.py      # Icarus Verilog simulation runner for MAC unit
│   ├── xmamba.py      # Xmamba instruction oracle
│   └── verify_token.py # End-to-end token co-simulation (Icarus + Verilator)
├── tests/             # Unit tests and Verilator test hosts
│   ├── test_ref.py    # Unit tests for MAC reference
│   ├── test_xmamba.py # Unit tests for Xmamba oracle
│   ├── rtl_model.py   # Cycle-accurate Python model of RTL logic
│   └── token_test.cc  # C++ host for Verilator co-simulation
├── synth/             # FPGA synthesis
│   └── synth.tcl      # Vivado out-of-context synthesis for Artix-7 (XC7A100T)
├── tools/             # Build scripts
│   └── generate_cfu_v.py # Generator for monolithic cfu.v
├── scripts/           # Automation scripts
│   └── run_renode_demo.py # Headless Renode simulation and UART capture
├── demo/              # Visualisation and diagnostics
│   └── demo_dashboard.py # Terminal and dashboard metrics monitor
└── mb_mac -> .        # Backwards-compatibility symlink
```

---

## Instructions & Custom-0 Operations

All instructions use RISC-V `custom-0` (opcode `0x0B`):
- `mb.mac4` (`funct3=0, funct7=0`): Accumulate four `int8` lane products into 32-bit accumulator.
- `mb.racc` (`funct3=0, funct7=1`): Rescale accumulator by shift, saturate, return result, and clear accumulator.
- `mb.exp`, `mb.silu`, `mb.softplus` (`funct3=1..3`): Piecewise-linear nonlinear activation lookup.
- `mb.lda`, `mb.layer`, `mb.ldbc`, `mb.scan`, `mb.rst`: Scan engine configuration, parameter loading, and state recurrence update.

---

## Running Verification

From the `hw/` directory:

```bash
# 1. Run all unit tests
python3 -m unittest discover -s tests -v

# 2. Check lookup tables consistency
python3 -m mbmac.xmamba --check

# 3. Run quick RTL simulation with Icarus Verilog
python3 -m mbmac.verify --quick

# 4. Run end-to-end token pipeline verification
python3 -m mbmac.verify_token
```

---

## Integrating into CFU Playground

To build and run on a simulated VexRiscv SoC in Renode:

```bash
export CFU_ROOT="$HOME/CFU-Playground"
cp -r "$CFU_ROOT/proj/proj_template_v" "$CFU_ROOT/proj/xmamba"
bash soc/build_cfu.sh "$CFU_ROOT/proj/xmamba"
cd "$CFU_ROOT/proj/xmamba"
make renode
```

In the UART console menu:
- Choose `3` for **Project menu**
- Press `t` to run a **single token** (software vs CFU comparison)
- Press `s` to run a **10-token sequence and reset**
