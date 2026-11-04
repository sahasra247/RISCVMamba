# test/

Master regression test runner, bit-exact trace comparator, and test vector verification.  
**Owner:** D (Testing & Benchmarks).

## Scripts in this Directory

| Script | Purpose |
| --- | --- |
| `run_all.sh` | Bash master regression: runs model self-checks, Spike C reference check, Spike bare-metal C compare, hardware unit tests, and RTL simulation. |
| `run_all.py` | Python cross-platform master regression suite. |
| `compare.py` | Trace comparator: checks layer stage tensors, scan states, and logits across Python, Spike, Renode/Verilator, and FPGA UART dumps. |

## Quick Run

```bash
bash test/run_all.sh
```
