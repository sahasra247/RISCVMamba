# sw/ — bare-metal C for RV32IM

Programs here run on Spike without an operating system or proxy kernel. They use a
small runtime (`common/`) that will later get a CFU Playground back end with the
same API, so kernels and measurement code are written once.

| Path | Purpose |
| --- | --- |
| `common/crt0.S` | Start-up: gp/sp, zero .bss, trap handler, call `main`, exit with its return value |
| `common/link.ld` | Memory map: RAM at 0x80000000 (Spike's DRAM), 64 KiB stack, `.tohost` section |
| `common/bm.h` | API: `bm_printf`, `bm_exit`, `bm_cycles()`, `bm_instret()`, `BM_PERF_BEGIN/END` |
| `common/bm_spike.c` | Spike back end: console and exit through HTIF (fesvr syscall proxy) |
| `tests/hello.c` | Hello world |
| `tests/counters.c` | Reads mcycle/minstret around regions; checks scaling and an int8 dot product |

## Toolchain setup (Ubuntu)

```bash
# Cross compiler with rv32im/ilp32 multilib and newlib headers
sudo apt install -y gcc-riscv64-unknown-elf libnewlib-dev picolibc-riscv64-unknown-elf \
                    device-tree-compiler build-essential git
riscv64-unknown-elf-gcc -print-multi-lib | grep rv32im/   # must print rv32im/ilp32

# Spike from source (installs to /usr/local/bin/spike)
git clone https://github.com/riscv-software-src/riscv-isa-sim.git
cd riscv-isa-sim && mkdir build && cd build
../configure --prefix=/usr/local
make -j"$(nproc)" && sudo make install
```

The proxy kernel `pk` is not needed: a pk built for rv64 cannot run rv32 programs,
and our programs talk to Spike directly through HTIF.

A `riscv32-unknown-elf-` toolchain works too: `make CROSS=riscv32-unknown-elf-`.

## Build and run

```bash
cd sw
make run            # builds every tests/*.c and runs each on spike --isa=rv32im_zicsr
make run-counters   # one program
```

Expected output:

```
== counters
empty       cycles=4 instret=12
spin 1000   cycles=2008 instret=2016
spin 10000  cycles=20009 instret=20017
dot int8x1024 cycles=11788 instret=11796
  result=-497152 expected=-497152  instret/MAC x100=1151
counters: PASS
== hello
Hello from bare-metal RV32IM on Spike!
```

`main`'s return value becomes Spike's exit code, so scripts can check it. A trap
(illegal instruction, bad address) prints `TRAP mcause=... mepc=... mtval=...` and
exits with 127. Stock Spike traps on our custom-0 instructions (mcause=2); load
the Xmamba extension (`--extlib=../sim/spike/build/libxmamba.so --extension=xmamba`)
to run them.

## Mamba engine (float, Stage 1)

| Path | Purpose |
| --- | --- |
| `tools/export_weights.py` | Packs weights into `build/gen/weights.bin`, writes NumPy reference outputs (`ref.bin`), `model_cfg.h` and `blobs.S` |
| `engine/mamba_f32.c` | Float engine, one token at a time; one noinline function per kernel; no malloc |
| `engine/fmath.c` | expf, logf, log1pf, rsqrtf, SiLU, softplus (no libm on bare metal) |
| `apps/mamba_float.c` | Runs the reference tokens; checks each layer's output, scan state h and logits on the target |

```bash
make mamba-float                                  # random weights, d=64, 2 layers, 8 tokens (~2 s)
make mamba-float D_MODEL=256 N_LAYER=4 NTOK=2     # planned size, 1.82M params (~1 min)
make mamba-float WEIGHTS=../model/export          # real weights: <hf name>.npy files + config.json
```

The check passes when every tensor's max |C − NumPy| / max |NumPy| is below 1e-5
(typical: ~1e-6). The scan state h is compared directly because the residual
stream and the D·u term can hide scan errors in the layer outputs.

Float runs entirely in software on RV32IM (~430M instructions per token at the
planned size), so this engine is the correctness reference, not the baseline we
accelerate; the fixed-point engine is.

## Fixed-point engine and Xmamba builds (Stages 1–2)

| Path | Purpose |
| --- | --- |
| `engine/mamba_q.c` | Fixed-point engine; bit-exact with `model/mamba_fixed.py`. One source, four builds via `-DXM_MAC`, `-DXM_NL`, `-DXM_SCAN`. Counts instructions per kernel. |
| `engine/xmamba.h` | Intrinsics: one inline function per Xmamba instruction (`.insn r CUSTOM_0, ...`) |
| `apps/mamba_fixed.c` | Requires **every integer** (each layer's output, h, logits) to equal the Python reference; prints the profile |
| `tools/compare_counts.py` | Turns the four logs into the instruction-count table (`--csv` for the results sheet) |

```bash
make mamba-fixed                      # software baseline (plain spike)
make mamba-x                          # full Xmamba (spike + ../sim/spike/build/libxmamba.so)
make compare                          # baseline, +MAC, +MAC+NL, Xmamba, then the table
make compare D_MODEL=256 N_LAYER=4    # planned size (~3 min)
make mamba-fixed WEIGHTS=../model/export   # trained weights, once Model & Reference exports them
```

Planned size (d=256, 4 layers): baseline 13.54M instructions/token, full Xmamba
3.18M (4.26×), all four builds bit-exact. Table: `results/spike_instr_counts_d256_l4.csv`;
discussion: `spec/xmamba-isa.md` §1.

The engine compiles `sim/spike/xmamba_ref.c` into itself (`#include`) so the
baseline's arithmetic helpers inline; calling them across files made the baseline
20% slower, which would have inflated the speed-up.

## Measuring

```c
bm_perf_t p;
BM_PERF_BEGIN(p);
kernel();
BM_PERF_END(p);
bm_printf("cycles=%llu instret=%llu\n", p.cycles, p.instret);
```

Spike is functional: its mcycle advances once per instruction, so cycles and
instret differ only by where in the macro each counter is read. Real cycle counts
come from the SoC simulation and the FPGA. An empty region costs 12 instructions;
subtract that from small regions. `build/<name>.dis` holds the disassembly.
