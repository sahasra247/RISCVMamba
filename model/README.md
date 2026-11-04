# model/

Weight export, NumPy reference, gen_luts.py, accuracy script. Owner: C (Model & Reference).

| File | What it is | Status |
| --- | --- | --- |
| `mamba_step.py` | Float golden model, one token at a time (recurrent step form), HF `MambaForCausalLM` weight names; random weights until the real ones exist; per-stage trace for test vectors. `python3 mamba_step.py` self-checks against an independent whole-sequence form. | Written by B. **C:** `load_weights()`, PyTorch comparison (20 sequences, < 1e-4), test-vector export. |
| `mamba_fixed.py` | Bit-exact fixed-point model: the integer arithmetic the RV32IM baseline, Spike and the RTL reproduce exactly. Holds the lookup tables. `python3 mamba_fixed.py` prints table errors and fixed-vs-float error. | Written by B. **C:** run the accuracy script on its outputs with the trained model. |

The tables of the nonlinear unit are defined in `mamba_fixed.py` (`make_tables()`),
and `sim/spike/gen_luts_h.py` turns them into C. The plan's `gen_luts.py` task (C)
can build on `mamba_fixed.table_report()`, which already measures their error.
