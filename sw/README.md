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
exits with 127. Stock Spike traps on our custom-0 instructions (mcause=2) until
the Xmamba extension is added in Stage 2.

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
