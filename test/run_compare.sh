#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
echo "================================================================="
echo "   XMAMBA STAGE-BY-STAGE VALIDATION: REFERENCE vs CFU HARDWARE   "
echo "================================================================="
python3 "$ROOT/test/compare.py" "$ROOT/results/ref_dump.json" "$ROOT/results/dut_dump.json"
