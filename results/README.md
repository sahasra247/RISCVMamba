# results/

Performance logs, cycle counts, instruction-count reductions, and FPGA synthesis resource reports.  
**Owner:** D (Testing & Benchmarks).

## Files in this Directory

| File | Target Platform | Description |
| --- | --- | --- |
| `results.csv` | Renode / CFU Playground SoC & Vivado | Benchmark measurements (sw vs cfu cycles, instret, speed-up per kernel) and Vivado XC7A100T resource utilization. |
| `spike_instr_counts_d256_l4.csv` | Spike RV32IM Simulator | Instruction-count profiling per kernel across four engine configurations (Baseline, +MAC, +MAC+NL, Full Xmamba). |
