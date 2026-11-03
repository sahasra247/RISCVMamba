import unittest

from mbmac import ref
from mbmac.vectors import generate
from tests import rtl_model
import tempfile
from pathlib import Path


class RefTests(unittest.TestCase):
    def test_lane_order_and_signs(self):
        self.assertEqual(ref.lanes(0x80FF7F01), [1, 127, -1, -128])
        self.assertEqual(ref.pack(1, 127, -1, -128), 0x80FF7F01)

    def test_mac4_example(self):
        a = ref.pack(-1, 2, -3, 4)
        b = ref.pack(5, -6, 7, -8)
        self.assertEqual(ref.mac4(0, a, b), -70)       # -5 -12 -21 -32

    def test_mac4_extremes_and_wrap(self):
        big = ref.pack(-128, -128, -128, -128)
        self.assertEqual(ref.mac4(0, big, big), 1 << 16)
        acc = 0
        for _ in range(1 << 15):
            acc = ref.mac4(acc, big, big)
        self.assertEqual(acc, -(1 << 31))              # wraps, does not saturate

    def test_racc_rounding_half_up(self):
        self.assertEqual(ref.racc(1, 1)[0], 1)          # 0.5 -> 1
        self.assertEqual(ref.racc(-1, 1)[0], 0)         # -0.5 -> 0 (toward +inf)
        self.assertEqual(ref.racc(3, 1)[0], 2)          # 1.5 -> 2
        self.assertEqual(ref.racc(-3, 1)[0], 0xFFFFFFFF)  # -1.5 -> -1
        self.assertEqual(ref.racc(5, 0)[0], 5)          # shift 0: no rounding term

    def test_racc_saturation_and_clear(self):
        self.assertEqual(ref.racc(1000, 0), (127, 0))
        self.assertEqual(ref.racc(-1000, 0), (0xFFFFFF80, 0))
        self.assertEqual(ref.racc(1000, 0, out_bits=16), (1000, 0))
        self.assertEqual(ref.racc(5, 0)[1], 0)

    def test_racc_uses_only_low_five_bits(self):
        self.assertEqual(ref.racc(100, 0xABCDE000 | 3), ref.racc(100, 3))

    def test_vector_file_matches_model(self):
        with tempfile.TemporaryDirectory() as d:
            m = generate(Path(d), random_sequences=100, lanes_swept=1, wrap_test=False)
            rows = rtl_model.load(Path(d) / "mac_vectors.hex")
            self.assertEqual(len(rows), m["count"])
            rtl_model.run(rows, seed=7, stall_pct=40)


if __name__ == "__main__":
    unittest.main()
