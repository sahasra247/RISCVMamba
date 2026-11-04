#!/bin/bash
# Copy the RTL and the test menu into a CFU Playground project.
# usage: soc/build_cfu.sh ~/CFU-Playground/proj/mb_cfu
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PROJ="${1:?usage: $0 <CFU-Playground project dir>}"
# One cfu.v containing both modules, so it works however the project picks up Verilog files.
cat "$ROOT/rtl/mb_mac_unit.v" "$ROOT/rtl/cfu_top.v" > "$PROJ/cfu.v"
cp "$ROOT/soc/proj_menu.cc" "$PROJ/src/proj_menu.cc"
echo "Installed into $PROJ. Now: cd $PROJ && make renode"
