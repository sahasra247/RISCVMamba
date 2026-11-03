import math
import unittest

from mbmac.xmamba import Reference, nonlinear, qmul, sat


class XmambaTests(unittest.TestCase):
    def test_tables_against_math(self):
        functions = {"exp": math.exp,
                     "silu": lambda value: value / (1 + math.exp(-value)),
                     "softplus": lambda value: math.log1p(math.exp(value))}
        for name, function in functions.items():
            for value in range(-2048, 1 if name == "exp" else 2049):
                self.assertLess(abs(nonlinear(name, value) / 256 - function(value / 256)), 0.03)

    def test_recurrence_layers_and_reset(self):
        ref = Reference()
        ref.execute(2, 32, 256)
        ref.execute(18, 0, (256 << 16) | 256)
        packed = (256 << 16) | 128
        self.assertEqual(ref.execute(26, packed, 0), 384)
        self.assertEqual(ref.execute(26, packed, 0), 512)
        ref.execute(10, 1)
        self.assertEqual(ref.execute(26, packed, 0), 0)
        ref.execute(10, 0)
        ref.execute(34)
        self.assertEqual(ref.execute(26, packed, 0), 384)

    def test_integer_contract(self):
        self.assertEqual(qmul(-1, 1), -1)
        self.assertEqual(sat(1 << 40), (1 << 31) - 1)
        self.assertEqual(nonlinear("exp", -(1 << 31)), 0)
        self.assertEqual(nonlinear("silu", (1 << 31) - 1), (1 << 31) - 1)
        ref = Reference()
        self.assertEqual(ref.execute(1023, 0xffffffff, 0xffffffff), 0)
        self.assertEqual(ref.execute(26, 0xffffffff, 8), 0)


if __name__ == "__main__":
    unittest.main()