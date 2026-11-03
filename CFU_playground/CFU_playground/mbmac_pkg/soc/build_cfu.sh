#!/bin/bash
# Copy the RTL and the test menu into a CFU Playground project.
# usage: soc/build_cfu.sh ~/CFU-Playground/proj/mb_cfu
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PROJ="${1:?usage: $0 <CFU-Playground project dir>}"
if [[ ! -f "$PROJ/Makefile" || ! -d "$PROJ/src" ]]; then
	echo "Expected a CFU Playground project with Makefile and src/: $PROJ" >&2
	exit 1
fi
(cd "$ROOT" && python3 -m mbmac.xmamba --check)
{
	cat "$ROOT/rtl/mb_mac_unit.v"
	awk -v table="$ROOT/rtl/xmamba_tables.vh" '
		/`include "xmamba_tables.vh"/ {
			while ((getline line < table) > 0) print line
			close(table)
			next
		}
		{ print }
	' "$ROOT/rtl/mb_xmamba_unit.v"
	cat "$ROOT/rtl/cfu_top.v"
} > "$PROJ/cfu.v"
cp "$ROOT/soc/proj_menu.cc" "$PROJ/src/proj_menu.cc"
cp "$ROOT/soc/"xmamba*.h "$ROOT/soc/xmamba_token.cc" "$PROJ/src/"
echo "Installed into $PROJ. Run make renode there, then Project menu -> t or s."
