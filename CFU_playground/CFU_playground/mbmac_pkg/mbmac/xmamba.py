"""Independent integer oracle and table generator for the token bring-up ISA."""

import argparse
import math
from pathlib import Path


LAYERS, CHANNELS, STATES = 2, 8, 4
ONE = 256


def sat(value, bits=32):
    return max(-(1 << (bits - 1)), min((1 << (bits - 1)) - 1, value))


def qmul(left, right):
    return sat((left * right) // ONE)


def tables():
    grid = [index / 2 for index in range(-16, 17)]
    return {
        "exp": [int(math.floor(math.exp(-index / 2) * ONE + 0.5))
                for index in range(17)],
        "silu": [int(math.floor(value / (1 + math.exp(-value)) * ONE + 0.5))
                 for value in grid],
        "softplus": [int(math.floor(math.log1p(math.exp(value)) * ONE + 0.5))
                     for value in grid],
    }


TABLES = tables()


def nonlinear(operation, value):
    if operation == "exp":
        if value >= 0:
            return ONE
        if value <= -2048:
            return 0
        position = -value
    else:
        if value <= -2048:
            return 0
        if value >= 2048:
            return value
        position = value + 2048
    index, fraction = divmod(position, 128)
    values = TABLES[operation]
    return values[index] + ((values[index + 1] - values[index]) * fraction // 128)


def signed(value, bits):
    value &= (1 << bits) - 1
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


class Reference:
    def __init__(self):
        self.acc = 0
        self.layer = 0
        self.constants = [[0] * (CHANNELS * STATES + CHANNELS) for _ in range(LAYERS)]
        self.hidden = [[[0] * STATES for _ in range(CHANNELS)] for _ in range(LAYERS)]
        self.bc = [[(0, 0)] * STATES for _ in range(LAYERS)]

    def execute(self, function, operand0=0, operand1=0):
        group, operation = function & 7, function >> 3
        if group == 0:
            if operation == 0:
                total = sum(signed(operand0 >> (8 * lane), 8) *
                            signed(operand1 >> (8 * lane), 8) for lane in range(4))
                self.acc = signed(self.acc + total, 32)
            elif operation == 1:
                shift = operand0 & 31
                result = sat((self.acc + ((1 << (shift - 1)) if shift else 0)) >> shift, 8)
                self.acc = 0
                return result
        elif group == 1 and operation < 3:
            return nonlinear(("exp", "silu", "softplus")[operation], signed(operand0, 32))
        elif group == 2:
            if operation == 0 and operand0 < len(self.constants[0]):
                self.constants[self.layer][operand0] = signed(operand1, 16)
            elif operation == 1 and operand0 < LAYERS:
                self.layer = operand0
            elif operation == 2 and operand0 < STATES:
                self.bc[self.layer][operand0] = (signed(operand1, 16), signed(operand1 >> 16, 16))
            elif operation == 3 and operand1 < CHANNELS:
                delta, activation = signed(operand0, 16), signed(operand0 >> 16, 16)
                output = 0
                for state in range(STATES):
                    weight = self.constants[self.layer][operand1 * STATES + state]
                    decay = nonlinear("exp", qmul(delta, weight))
                    coeff_b, coeff_c = self.bc[self.layer][state]
                    hidden = sat(qmul(decay, self.hidden[self.layer][operand1][state]) +
                                 qmul(qmul(delta, coeff_b), activation))
                    self.hidden[self.layer][operand1][state] = hidden
                    output = sat(output + qmul(coeff_c, hidden))
                return sat(output + qmul(self.constants[self.layer][CHANNELS * STATES + operand1], activation))
            elif operation == 4:
                self.hidden = [[[0] * STATES for _ in range(CHANNELS)] for _ in range(LAYERS)]
        return 0


def generated_files():
    root = Path(__file__).resolve().parents[1]
    header = ["#ifndef XMAMBA_TABLES_H", "#define XMAMBA_TABLES_H", "#include <stdint.h>"]
    verilog = []
    for name, values in TABLES.items():
        header.append(f"static const int32_t xm_{name}_table[{len(values)}] = {{" +
                      ", ".join(map(str, values)) + "};")
        verilog.extend([f"function signed [31:0] {name}_table;", "  input [5:0] index;",
                        "  begin", "    case (index)"])
        for index, value in enumerate(values):
            literal = f"-32'sd{-value}" if value < 0 else f"32'sd{value}"
            verilog.append(f"      6'd{index}: {name}_table = {literal};")
        verilog.extend([f"      default: {name}_table = 32'sd0;", "    endcase", "  end", "endfunction"])
    header.append("#endif")
    return {root / "soc/xmamba_tables.h": "\n".join(header) + "\n",
            root / "rtl/xmamba_tables.vh": "\n".join(verilog) + "\n"}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    for path, content in generated_files().items():
        if args.check:
            if not path.exists() or path.read_text() != content:
                raise SystemExit(f"Stale generated table: {path}")
        else:
            path.write_text(content)
    print("PASS Xmamba tables")


if __name__ == "__main__":
    main()