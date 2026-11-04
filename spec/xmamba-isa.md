# Xmamba ISA — v1 draft

| | |
|---|---|
| Status | **Draft for Hardware review** (written 4 Nov 2026). Freeze target: 11 Dec 2026, by A and B together. |
| Owner | B (ISA & Software). After the freeze, changes need A and B and must update `xmamba_ref.c`, Spike, the RTL and the tests together. |
| Executable definition | `sim/spike/xmamba_ref.c`. Where this text and that file disagree, the file is right and the text is a bug. |
| Bit-exact model | `model/mamba_fixed.py` (Python). `make -C sim/spike check` proves the C and Python agree. |

Xmamba adds 11 R-type instructions on the custom-0 opcode in three groups: a **MAC unit** (int8 dot products), a **nonlinear unit** (exp, SiLU, softplus) and a **scan engine** that holds Mamba's state on-chip. The software needs no compiler changes; `sw/engine/xmamba.h` wraps each instruction in an inline function.

## 1. Why these instructions (measured profile)

Instructions per token of the RV32IM fixed-point baseline on Spike, at the planned model size (d=256, 4 layers, N=16, E=2, 1.82M parameters). All builds are bit-exact with `model/mamba_fixed.py`. Data: `results/spike_instr_counts_d256_l4.csv`, from `make -C sw compare D_MODEL=256 N_LAYER=4`.

| Kernel | Baseline | Share | + MAC | + MAC + NL | Full Xmamba | Reduction |
|---|---:|---:|---:|---:|---:|---:|
| in_proj | 5,141,005 | 38.0% | 1,655,293 | 1,655,293 | 1,655,293 | 3.1× |
| out_proj | 2,600,445 | 19.2% | 871,917 | 871,917 | 871,917 | 3.0× |
| x_proj + dt_proj | 791,688 | 5.8% | 312,619 | 312,619 | 312,619 | 2.5× |
| final norm + head | 337,853 | 2.5% | 119,993 | 119,993 | 119,993 | 2.8× |
| **selective scan** | **3,991,110** | **29.5%** | 3,991,110 | 1,626,336 | **16,980** | **235×** |
| softplus (Δ) | 327,768 | 2.4% | 327,768 | 14,384 | 14,384 | 22.8× |
| conv1d + SiLU | 192,692 | 1.4% | 192,692 | 100,456 | 100,456 | 1.9× |
| gate SiLU(z) | 105,584 | 0.8% | 105,584 | 32,820 | 32,820 | 3.2× |
| norms, embed, residual | 52,152 | 0.4% | 52,152 | 52,152 | 52,152 | 1.0× |
| **Total** | **13,541,056** | | **7,629,887** | **4,786,727** | **3,177,371** | **4.26×** |
| End-to-end vs baseline | 1.00× | | 1.77× | 2.83× | **4.26×** | |

Reading the profile:
- The work splits two ways, as the plan predicted: linear layers (~66%) and the scan (~30%). Each needs its own unit, so both are in the ISA. By Amdahl's law, either unit alone is capped (MAC alone: 1.77×).
- The scan engine removes almost all scan work (235×), because h, A and D never cross the register interface: one `mb.scan` replaces 16 software exponentials and their arithmetic.
- After Xmamba, **93% of what remains is the linear layers**. Each `mb.mac4` needs two 32-bit loads (4 weights, 4 activations), so the 64-bit operand interface, not the multiplier, is now the limit. Options for v2 are in §9.
- The software baseline was not left slow on purpose: shared helpers are inlined, and the int8 inner loop is unrolled ×4. Speed-ups are against this optimized fixed-point software, never against floating point (float is ~430M instructions/token; quoting against it would inflate everything ~30×).

These are **instruction counts** (Spike is functional). Cycle counts come from the SoC simulation (Stage 3) and the board (Stage 4), where multi-cycle instructions stall the CPU.

## 2. Encoding

All instructions are R-type on **custom-0** (`opcode = 0001011`, 0x0B):

```
 31      25 24   20 19   15 14  12 11    7 6       0
[ funct7  ][  rs2  ][  rs1  ][funct3][  rd   ][0001011]
```

`funct3` selects the group, `funct7` the operation. In the CFU Playground interface the 10-bit function id is `{funct7, funct3}`, so A's existing RTL decode (`function_id[2:0] = funct3`, `function_id[9:3] = funct7`) is unchanged.

| Instruction | funct3 | funct7 | match (mask 0xFE00707F) | CFU function id | Operands |
|---|---|---|---|---|---|
| mb.mac4 | 0 | 0 | 0x0000000B | 0x000 | rs1, rs2 |
| mb.racc | 0 | 1 | 0x0200000B | 0x008 | rd, rs1 |
| mb.exp | 1 | 0 | 0x0000100B | 0x001 | rd, rs1 |
| mb.silu | 1 | 1 | 0x0200100B | 0x009 | rd, rs1 |
| mb.softplus | 1 | 2 | 0x0400100B | 0x011 | rd, rs1 |
| mb.rst | 2 | 0 | 0x0000200B | 0x002 | — |
| mb.layer | 2 | 1 | 0x0200200B | 0x00A | rs1 |
| mb.lda | 2 | 2 | 0x0400200B | 0x012 | rs1, rs2 |
| mb.ldbc | 2 | 3 | 0x0600200B | 0x01A | rs1, rs2 |
| mb.scan | 2 | 4 | 0x0800200B | 0x022 | rd, rs1, rs2 |
| mb.rdh | 2 | 5 | 0x0A00200B | 0x02A | rd, rs1 |

Unused operand fields are ignored. Instructions without a result write 0 to rd (software uses rd = x0). Any other funct3/funct7 under custom-0 is reserved: Spike raises illegal-instruction; the RTL must answer 0 and change nothing, so a wrong opcode never hangs the CPU.

Assembler form (no toolchain changes): `.insn r CUSTOM_0, funct3, funct7, rd, rs1, rs2`.

## 3. Number formats

"Qn" means a signed two's-complement integer with n fractional bits (real = integer / 2ⁿ).

| Quantity | Format | Why |
|---|---|---|
| Residual stream x, u, z, B, C, y | Q16 int32 | range ±32768, step 1.5e-5 |
| Linear-layer weights | int8, per-row power-of-two scale | one shift per output element (mb.racc) |
| Linear-layer inputs | int8, power-of-two scale chosen per vector at run time | no calibration data needed; shift folds into mb.racc |
| Δ (delta) | Q24 int32 | step sizes are ~0.001–0.1; Q16 would lose ~1.5% |
| exp(Δ·A) | Q30 int32 | decay is often 0.999…; its distance from 1 needs bits |
| Scan state h | Q20 int32 | accumulates small increments over many tokens |
| A, D, conv weights, biases, norm weights | Q16 int32 | |

## 4. Arithmetic rules (apply everywhere)

```
rshr(v, s) = (v + 2^(s-1)) >> s     for s > 0   (arithmetic shift: round half up, ties toward +inf)
           =  v << -s               for s <= 0
wrap32(v)  = low 32 bits of v as signed          (C cast; no saturation)
sat32(v)   = clamp(v, -2^31, 2^31 - 1)
```

- Products are formed exactly in 64 bits, then rounded with `rshr`. No intermediate truncation unless written.
- **Rounding is round-half-up** (the cheapest in hardware, and what A's mb.racc RTL already does). Software baseline, Python, Spike and RTL all use it, which is what makes them bit-identical.
- Stored 32-bit values **wrap**, except mb.racc, which **saturates**. With the chosen formats no value overflowed in any test (`mamba_fixed.py` reports overflows).

## 5. Instruction semantics

Pseudo-code; `xmamba_ref.c` is the exact version. Unit state is listed in §6.

### MAC group (funct3 = 0)

**mb.mac4 rs1, rs2** — `acc = wrap32(acc + Σ_{l=0..3} int8(rs1[8l+7:8l]) × int8(rs2[8l+7:8l]))`. Lane 0 is bits 7:0. rd ← 0.

**mb.racc rd, rs1** — `sh = signext(rs1[5:0]) clamped to [-31, 31]`; `rd = sat32(rshr(acc, sh))`; `acc = 0`.
A matrix-vector product is, per output row r: `mb.mac4` over the row's packed weights and the packed int8 inputs, then `mb.racc` with `sh = weight_shift[r] − input_shift`, giving the output in Q16.

### Nonlinear group (funct3 = 1), rd = f(rs1)

All three are piecewise-linear lookup tables with one linear interpolation. The tables are in `xmamba_luts.h`, generated from `model/mamba_fixed.py` by `sim/spike/gen_luts_h.py`. They are part of the ISA, the same for every model.

| Instruction | In | Out | Table | Max error (measured) |
|---|---|---|---|---|
| **mb.exp** | Q24, x ≤ 0 (x > 0 is treated as 0) | Q30 | range reduction to 2^f, f ∈ [0,1): 64 segments, 65 entries Q29 | 1.5e-5 relative (x > −1) |
| **mb.silu** | Q16 | Q16 | sigmoid on [0,16): 128 segments, 129 entries Q30; symmetric; 1.0 beyond 16 | 3.3e-4 absolute |
| **mb.softplus** | Q16 | Q24 | exp(−\|v\|)·r(\|v\|), r(a) = log1p(e^−a)/e^−a on [0,16): 64 segments, Q30; v > 20 → v | 7.6e-4 relative (v > −10) |

mb.exp:
```
x  = min(x, 0)
t  = rshr(x * round(log2(e) * 2^30), 30)          # Q24, t <= 0
n  = t >> 24 ;  f = t - (n << 24)                 # n = floor(t), f in [0, 2^24)
v  = T[f>>18] + rshr((T[(f>>18)+1] - T[f>>18]) * (f & 0x3FFFF), 18)   # 2^f in Q29
rd = (n+1 <= -62) ? 0 : rshr(v, -(n+1))           # Q29 * 2^n -> Q30
```
mb.silu: `s = sigmoid_table(|x|)` (Q30; `2^30 − s` for x < 0); `rd = rshr(x * s, 30)`.
mb.softplus: `a = |v|`; `e = mb.exp(−a << 8)`; `r = r_table(a)`; `sp = rshr(e * r, 36)`; `rd = (v <= 0) ? sp : (v << 8) + sp`; `rd = v << 8` if v > 20.0; saturate to INT32_MAX.

### Scan group (funct3 = 2)

**mb.rst** — h ← 0 for every channel; channel counter ← 0. Once per sequence.

**mb.layer rs1** — base ← rs1 (first channel of this layer = layer × E·d); counter ← 0. Once per layer per token.

**mb.lda rs1, rs2** — load a constant. rs1[31] = 0: `A[rs1[30:0]] = rs2` (index = channel × 16 + n). rs1[31] = 1: `D[rs1[30:0]] = rs2` (index = channel). At start-up only.

**mb.ldbc rs1, rs2** — this token's B or C: rs1[8] = 0: `B[rs1[3:0]] = rs2`; rs1[8] = 1: `C[rs1[3:0]] = rs2` (Q16). 32 per layer per token. (The plan's v0 packed B and C as 16-bit pairs; our formats keep them 32-bit, so one value per instruction. Cost: 32 instructions per layer, negligible.)

**mb.scan rd, rs1, rs2** — rs1 = Δ_i (Q24), rs2 = u_i (Q16); channel i = base + counter; counter += 1.
```
du = wrap32(rshr(Δ * u, 16))                                  # Q24
acc = 0
for n in 0..15:
    a   = max(rshr(Δ * A[i,n], 16), -2^31)                    # Q24, <= 0
    dA  = mb.exp(a)                                           # Q30
    h[i,n] = wrap32(rshr(dA * h[i,n], 30) + rshr(du * B[n], 20))   # Q20
    acc += h[i,n] * C[n]                                      # 64-bit
rd = wrap32(rshr(acc, 20) + rshr(D[i] * u, 16))               # y_i, Q16
```
(The plan's v0 passed the channel in rs2 and packed Δ and u into rs1. Δ needs 24 fractional bits and u 32 bits, so v1 passes them in full and uses an auto-incrementing channel counter set by mb.layer.)

**mb.rdh rd, rs1** — `rd = h[rs1]` (index = channel × 16 + n). For tests and debugging: the bit-exact check reads h back through it. Not used in the inference loop.

## 6. Unit state and capacity

| State | Size | Written by | Read by |
|---|---|---|---|
| acc | 32 bit | mb.mac4, mb.racc (clear) | mb.racc |
| base, counter | 2 × 11 bit | mb.layer, mb.scan, mb.rst | mb.scan |
| B[16], C[16] | 32 × 32 bit (registers) | mb.ldbc | mb.scan |
| A[MAX_CH × 16] | 32 bit each | mb.lda | mb.scan |
| D[MAX_CH] | 32 bit each | mb.lda | mb.scan |
| h[MAX_CH × 16] | 32 bit each | mb.scan, mb.rst | mb.scan, mb.rdh |

MAX_CH is a synthesis parameter (`XM_MAX_CH`), the number of channels over all layers. **v1 default: 2048**, which fits the planned model exactly (4 layers × 512) and costs 128 KiB (A) + 128 KiB (h) + 8 KiB (D) = **264 KiB of the board's 607.5 KiB block RAM**. Indices out of range trap in Spike and are undefined in the RTL.

## 7. Software sequence

```c
/* start-up */
mb_rst();
for (layer, channel i, n)  mb_lda(ch*16 + n, A[layer][i][n]);
for (layer, channel i)     mb_lda(MB_LDA_D | ch, D[layer][i]);

/* per token, per layer */
for (row r) { for (word c) mb_mac4(w[r][c], x8[c]); out[r] = mb_racc(sw[r] - s); }   /* each linear layer */
u[i] = mb_silu(conv_i);  delta[i] = mb_softplus(dt_i);                                /* activations */
mb_layer(layer * ED);
for (n < 16) { mb_ldbc(n, B[n]); mb_ldbc(n | MB_LDBC_C, C[n]); }
for (i < ED) y[i] = mb_scan(delta[i], u[i]);
```
The full engine is `sw/engine/mamba_q.c`; compile with `-DXM_MAC`, `-DXM_NL`, `-DXM_SCAN` to enable each group.

## 8. Latency targets (proposed — for Hardware to confirm)

| Instruction | Target | Notes |
|---|---|---|
| mb.mac4 | issue every cycle | A's RTL currently accepts one every 2 cycles (cmd_ready low while a response is pending) |
| mb.racc | ≤ 4 cycles | waits for in-flight mac4s (as A's RTL already does) |
| mb.exp, mb.silu | ≤ 4 cycles | 1 BRAM read, 1 multiply, shifts |
| mb.softplus | ≤ 8 cycles | exp + second table + multiply |
| mb.layer, mb.lda, mb.ldbc, mb.rdh | 1–2 cycles | |
| mb.scan | ≤ 16 + 8 cycles | one state per cycle after the pipeline fills; dual-port BRAM reads h[n+1] while writing h[n] |
| mb.rst | once per sequence | 32,768 words; a 1-bit-per-channel "valid" vector clears in 1 cycle if the loop is too slow |

## 9. Open questions for the Hardware review

1. **mb.racc output: int32 with a signed shift.** A's RTL saturates to int8 (`OUT_BITS = 8`) with an unsigned 5-bit shift. The baseline keeps activations in Q16 between layers and re-quantizes each input vector, so racc must return 32 bits and sometimes shift left (measured with random weights at the planned size: shifts from −1 to +4; trained weights may spread wider, hence ±31). Proposed: `OUT_BITS = 32`, `sh = signed rs1[5:0]` clamped to ±31. This answers open question 1 in `hw/mb_mac/README.md`.
2. **Rounding = round half up**, as A's RTL. Answers open question 2. No per-channel scale/bias in racc (question 3): not needed.
3. **mac4 throughput** (question 4): one per cycle would help, since linear layers are now 93% of the instruction count.
4. **Scan datapath cost.** Per state: Δ×A (32×32), the exp table (one 32×18 multiply), dA×h (31×32), du×B (32×32), h×C (32×32). About 5 wide multiplies ≈ 20 DSP48 slices for one state per cycle (board has 240). Option before the freeze: store A·log₂e instead of A, removing one multiply from the exp. It changes the bits, so it has to be decided before 11 Dec.
5. **A in 16 bits** would save 64 KiB of BRAM if Person C confirms the trained A values' range.
6. Keep **mb.rdh**? It costs one BRAM read port mux and is what makes h checkable in system tests.
7. **v2 candidates** (not for this freeze), aimed at the linear-layer bottleneck:
   - Keep the int8 input vector inside the unit (written once per matvec), so each `mb.mac4w` streams 8 weights against it: half the instructions in every linear layer.
   - An `mb.q8` instruction that rescales and packs four Q16 values into int8 lanes. Quantization runs once per input element per matvec (a max pass plus a round-and-clip pass), so it matters most for the narrow-output x_proj; it should be profiled separately before deciding.

## 10. Verification

| Check | Command | Result (4 Nov) |
|---|---|---|
| xmamba_ref.c = mamba_fixed.py, every function, edge + random inputs | `make -C sim/spike check` | PASS (≈200k inputs per function, 3,000 scan updates incl. extremes) |
| Baseline C = mamba_fixed.py, every integer of every layer, h and logits | `make -C sw mamba-fixed` | PASS, 0 mismatches |
| +MAC, +MAC+NL, full Xmamba on Spike = same reference | `make -C sw compare` | PASS, 0 mismatches, both sizes |
| Fixed-point vs float (random weights) | `python3 model/mamba_fixed.py` | worst element 6–9% relative, same top-1 token 45/48; real accuracy needs the trained model |

RTL test vectors come from the same code: `make -C sim/spike build/libxmamba_ref.so` gives a host library with `xm_exec()`, which Python can call through ctypes (`sim/spike/check_ref.py` shows how).
