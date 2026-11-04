# RISC-V Custom Function Unit (CFU) Interface Handshake Summary

**Prepared for:** RISCVMamba Team (Member A: Sahasra)  
**Target Core:** VexRiscv (RV32IM) with CFU Coprocessor Interface  
**Instruction Mapping:** R-type `CUSTOM_0` (`opcode = 7'b0001011`)

---

## 1. Interface Signal Definitions

The CFU communicates with the VexRiscv pipeline through a strict point-to-point valid/ready request-response handshake.

| Signal Name | Direction (from CFU) | Width | Description |
| :--- | :--- | :--- | :--- |
| `clk` | Input | 1 bit | System clock (50 MHz / 100 MHz from SoC). |
| `reset` | Input | 1 bit | Synchronous active-high reset. Clears all internal pipeline registers, accumulators, and pending state. |
| `cmd_valid` | Input | 1 bit | Asserted by CPU when issuing a custom instruction. |
| `cmd_ready` | Output | 1 bit | Asserted by CFU when ready to accept command operands. |
| `cmd_payload_function_id` | Input | 10 bits | Packed instruction identifier (`funct7` and `funct3`). |
| `cmd_payload_inputs_0` | Input | 32 bits | Source operand register 1 (`rs1`). |
| `cmd_payload_inputs_1` | Input | 32 bits | Source operand register 2 (`rs2`). |
| `rsp_valid` | Output | 1 bit | Asserted by CFU when the result is valid and ready for CPU writeback. |
| `rsp_ready` | Input | 1 bit | Asserted by CPU when it can accept the result. |
| `rsp_payload_outputs_0` | Output | 32 bits | Destination result written back to register `rd`. |

---

## 2. Function ID Decoding & ISA Mapping

The 10-bit `function_id` is partitioned as follows:
- `function_id[2:0]` = `funct3` (Instruction Subgroup / Unit Selector)
- `function_id[9:3]` = `funct7` (Specific Sub-Operation)

```
+------------------------------------+--------------------------+
|  function_id[9:3] (funct7: 7 bits) | function_id[2:0] (funct3)|
+------------------------------------+--------------------------+
```

| Instruction | `funct3` | `funct7` | `function_id` | Operation Summary | Latency (cycles) | Pattern |
| :--- | :---: | :---: | :---: | :--- | :---: | :--- |
| `mb.mac4` | `0` | `0` | `0` | 4-lane int8 dot product accumulated into int32 | 1 | Registered Multi-Cycle (Pipelined) |
| `mb.racc` | `0` | `1` | `8` | Rescale, round half-up, saturate to int8, clear acc | 1–2 | Registered Multi-Cycle |
| `mb.exp` | `1` | `0` | `1` | Piecewise-linear exponential ($Q8 \to Q8$) | 1 | Combinational / Pipelined LUT |
| `mb.silu` | `1` | `1` | `9` | Piecewise-linear SiLU activation ($Q8 \to Q8$) | 1 | Combinational / Pipelined LUT |
| `mb.softplus`| `1` | `2` | `17` | Piecewise-linear Softplus ($Q8 \to Q8$) | 1 | Combinational / Pipelined LUT |
| `mb.lda` | `2` | `0` | `2` | Load constant A or D parameter into internal BRAM | 1 | Single-Cycle Registered Write |
| `mb.layer` | `2` | `1` | `10` | Select active layer context (layer 0 or 1) | 1 | Single-Cycle Registered Write |
| `mb.ldbc` | `2` | `2` | `18` | Load packed $B_n$ and $C_n$ for state index $n$ | 1 | Single-Cycle Registered Write |
| `mb.scan` | `2` | `3` | `26` | Multi-cycle selective scan recurrence for channel $i$ | $3N + 1$ | Multi-Cycle Stateful FSM |
| `mb.rst` | `2` | `4` | `34` | Clear hidden states across all layers ($h \leftarrow 0$) | Multi-cycle | Multi-Cycle Background Sweep |

---

## 3. Handshake & Timing Rules

1. **Transfer Handshake:** A transfer occurs on the positive clock edge if and only if both `valid` and `ready` are asserted (`valid && ready == 1`).
2. **Single-Cycle Operand Lifetime:** Operands `cmd_payload_inputs_0` and `inputs_1` are **guaranteed valid only during the cycle `cmd_valid && cmd_ready` is true**. The CFU must latch all operands into internal registers immediately upon acceptance.
3. **CPU Pipeline Stall Behavior:** The VexRiscv core stalls its execution pipeline whenever a custom instruction is dispatched until `rsp_valid && rsp_ready` completes.
4. **Guaranteed Response Rule:** If the CFU fails to assert `rsp_valid`, the CPU core hangs indefinitely. Therefore:
   - Every valid command must eventually produce a matching response.
   - Any unsupported or out-of-range `function_id` immediately answers with `rsp_payload_outputs_0 = 32'd0` without altering architectural state.
5. **Backpressure Handling:** Once `rsp_valid` is asserted, `rsp_valid` and `rsp_payload_outputs_0` remain stable until `rsp_ready` goes high. `cmd_ready` is de-asserted whenever an internal operation is busy or a response is pending, preventing command collisions.

---

## 4. Hardware Implementation Patterns

### Pattern A: Single-Cycle / Pipelined Function (MAC & Nonlinear)
- `cmd_ready` is held high when no response is pending.
- Operands are registered on cycle 0.
- Result is driven and `rsp_valid` asserted on cycle 1.
- `cmd_ready` drops during cycle 1 to enforce single-instruction in-flight execution.

### Pattern B: Multi-Cycle Stateful Engine (`mb.scan`)
- Operands ($\Delta_i, u_i$) and channel index $i$ are latched into state registers.
- An internal finite-state machine (FSM) sequences through $n = 0 \dots N-1$:
  * Reads $h[i, n]$ and $A[i, n]$
  * Computes $\text{decay} = \exp(\Delta_i A_{i,n})$ using shared nonlinear LUT
  * Updates hidden state $h_{new} = \text{decay} \cdot h + \Delta_i B_n u_i$ and writes back
  * Accumulates output $y_i += C_n \cdot h_{new}$
- Adds skip connection $D_i \cdot u_i$.
- Asserts `rsp_valid` with final $y_i$ after $3N + 1$ clock cycles.
