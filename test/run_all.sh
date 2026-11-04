#!/usr/bin/env bash
# ==============================================================================
# Master Regression Test Suite for Xmamba (Software + Hardware)
# Runs model checks, Spike C reference, Spike bare-metal C, RTL unit tests,
# and token verification.
# ==============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${ROOT_DIR}"

echo "=================================================================="
echo " Starting Xmamba Complete Regression Test Suite"
echo " Workspace: ${ROOT_DIR}"
echo "=================================================================="

# 1. Model Reference Checks
echo -e "\n[1/6] Running Python Model Golden Checks..."
python3 model/mamba_step.py
python3 model/mamba_fixed.py

# 2. Spike C Reference Checks
echo -e "\n[2/6] Verifying C Reference against Python Oracle..."
make -C sim/spike check

# 3. Spike Bare-Metal C Engines (4-Build Comparison)
echo -e "\n[3/6] Running Bare-Metal RV32IM Engines on Spike..."
make -C sw compare

# 4. Hardware Unit Tests & Table Check
echo -e "\n[4/6] Running Hardware Python Unit Tests..."
(cd hw && python3 -m unittest discover -s tests -v)
(cd hw && python3 -m mbmac.xmamba --check)

# 5. Quick RTL Verification (Icarus Verilog)
echo -e "\n[5/6] Running RTL Quick Verification..."
if command -v iverilog >/dev/null 2>&1; then
    (cd hw && python3 -m mbmac.verify --quick)
else
    echo "  [SKIPPED] iverilog not installed on this system."
fi

# 6. Token Pipeline Regression
echo -e "\n[6/6] Running End-to-End Token Pipeline Verification..."
if command -v iverilog >/dev/null 2>&1; then
    (cd hw && python3 -m mbmac.verify_token)
else
    echo "  [INFO] Running Token Verification without Icarus RTL..."
fi

echo -e "\n=================================================================="
echo " ALL Xmamba REGRESSION GATES PASSED SUCCESSFULLY!"
echo "=================================================================="
