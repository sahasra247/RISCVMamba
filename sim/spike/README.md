# sim/spike/ — Xmamba reference semantics and Spike extension

Owner: B (ISA & Software). The ISA itself is described in [`spec/xmamba-isa.md`](../../spec/xmamba-isa.md).

| File | Purpose |
| --- | --- |
| `xmamba_ref.h/.c` | **Single source of truth**: what each Xmamba instruction computes, bit for bit. Plain C99, builds for the host and for RV32. |
| `gen_luts_h.py` | Writes `xmamba_luts.h` (exp, sigmoid, softplus tables) from `model/mamba_fixed.py`. |
| `xmamba_spike.cc` | Spike extension: decodes custom-0, calls `xm_exec()`, keeps the unit state per hart, adds disassembly names (`mb.scan a4, a4, a1`). |
| `check_ref.py` | Proves `xmamba_ref.c` = `model/mamba_fixed.py` on edge and random inputs (via ctypes). |

```bash
make            # build/libxmamba.so
make check      # C reference vs Python reference: PASS for all 5 functions + scan
spike --isa=rv32im_zicsr --extlib=build/libxmamba.so --extension=xmamba prog.elf
spike -l ... 2>&1 | grep mb.      # instruction trace shows mb.* mnemonics
```

The library is built against the installed Spike headers (`SPIKE_PREFIX`, default
`/usr/local`) and is **not** linked with `libriscv`: the `spike` binary already
contains it, and a second copy aborts with "Plugin ... already registered".

**For RTL test vectors** (Testing & Benchmarks, Hardware): `make build/libxmamba_ref.so`
gives a host library exporting `xm_exec()` and the per-function entry points;
`check_ref.py` shows how to drive it from Python.
