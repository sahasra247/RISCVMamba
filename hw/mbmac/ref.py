"""Golden reference for mb.mac4 and mb.racc (the single source of truth).

State: one signed 32-bit accumulator.

mb.mac4 rs1, rs2 : acc = wrap32(acc + sum_i int8(rs1 lane i) * int8(rs2 lane i)), i = 0..3
                   lane 0 is bits [7:0]. Returns 0 to rd.
mb.racc rd, rs1  : s = rs1 & 31
                   v = floor((acc + (2**(s-1) if s else 0)) / 2**s)     (round half up)
                   rd = clamp(v, -2**(OUT_BITS-1), 2**(OUT_BITS-1) - 1), sign-extended to 32 bits
                   acc = 0

Python integers are unbounded, so intermediate values match the RTL's 33-bit datapath.
"""

MASK32 = 0xFFFFFFFF
DEFAULT_OUT_BITS = 8

F3_MAC = 0
F7_MAC4 = 0
F7_RACC = 1


def s8(x: int) -> int:
    """Interpret the low 8 bits of x as a signed byte."""
    return ((x + 128) & 0xFF) - 128


def s32(x: int) -> int:
    """Wrap to signed 32-bit."""
    return ((x + (1 << 31)) & MASK32) - (1 << 31)


def lanes(word: int) -> list[int]:
    return [s8((word >> (8 * i)) & 0xFF) for i in range(4)]


def pack(l0: int, l1: int, l2: int, l3: int) -> int:
    """Pack four signed bytes into a 32-bit word (lane 0 in bits [7:0])."""
    out = 0
    for i, v in enumerate((l0, l1, l2, l3)):
        if not -128 <= v <= 127:
            raise ValueError("lane out of int8 range")
        out |= (v & 0xFF) << (8 * i)
    return out


def mac4(acc: int, rs1: int, rs2: int) -> int:
    """Return the new accumulator."""
    return s32(acc + sum(a * b for a, b in zip(lanes(rs1), lanes(rs2))))


def racc(acc: int, rs1: int, out_bits: int = DEFAULT_OUT_BITS) -> tuple[int, int]:
    """Return (rd as an unsigned 32-bit pattern, new accumulator)."""
    shift = rs1 & 31
    half = (1 << (shift - 1)) if shift else 0
    value = (acc + half) >> shift            # Python >> on negatives is floor
    low, high = -(1 << (out_bits - 1)), (1 << (out_bits - 1)) - 1
    value = max(low, min(high, value))
    return value & MASK32, 0


def function_id(funct7: int, funct3: int = 0) -> int:
    return ((funct7 & 0x7F) << 3) | (funct3 & 7)
