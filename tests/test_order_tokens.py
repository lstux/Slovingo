"""Tests for src/order_tokens.py."""

import random
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "src"))

import order_tokens as ot  # noqa: E402
import sync_exercises  # noqa: E402


class OrderTokensTest(unittest.TestCase):
    PAIRS = [("Tu es là ? À Košice ?", "Ty si tu? V Košiciach?"),
             ("Attends... C'est pas possible.", "Počkaj... To nie je možné.")]

    def ex(self):
        return {"type": "order", "id": "x", "l1": self.PAIRS[0][0], "l2": self.PAIRS[0][1],
                "tokens_l1": ["Tu es là ?", "À Košice ?"], "tokens_l2": ["Ty si tu?", "V Košiciach?"]}

    def test_rechip_and_parasites(self):
        new = ot.rework(self.ex(), self.PAIRS, (2, 2), random.Random(0))
        self.assertEqual(new["tokens_l2"][:4], ["Ty", "si", "tu?", "V Košiciach?"])
        parasites = new["tokens_l2"][4:]
        self.assertEqual(len(parasites), 2)
        for w in parasites:
            self.assertTrue(w[0].islower() and w in {"nie", "je", "možné"})

    def test_word_level_left_alone(self):
        ex = self.ex()
        ex["tokens_l2"] = ["Ty", "si", "tu?", "V", "Košiciach?"]
        ex["tokens_l1"] = ["Tu", "es", "là", "?", "À", "Košice", "?"]
        self.assertIsNone(ot.rework(ex, self.PAIRS, (2, 2), random.Random(0)))

    def test_sync_keeps_loose_parasites(self):
        new = ot.rework(self.ex(), self.PAIRS, (2, 2), random.Random(0))
        self.assertIsNone(sync_exercises.refresh_parasite(new, set(self.PAIRS), random.Random(0)))


if __name__ == "__main__":
    unittest.main()
