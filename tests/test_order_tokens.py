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
        new = ot.rework(self.ex(), [self.PAIRS], (2, 2), random.Random(0))
        self.assertEqual(new["tokens_l2"][:4], ["Ty", "si", "tu?", "V Košiciach?"])
        parasites = new["tokens_l2"][4:]
        self.assertEqual(len(parasites), 2)
        for w in parasites:
            self.assertTrue(w[0].islower() and w in {"nie", "je", "možné"})

    def test_word_level_with_parasites_left_alone(self):
        ex = self.ex()
        ex["tokens_l2"] = ["Ty", "si", "tu?", "V", "Košiciach?", "nie"]
        ex["tokens_l1"] = ["Tu", "es", "là ?", "À", "Košice ?", "pas"]
        self.assertIsNone(ot.rework(ex, [self.PAIRS], (2, 2), random.Random(0)))

    def test_word_level_without_parasites_gets_some(self):
        ex = self.ex()
        ex["tokens_l2"] = ["Ty", "si", "tu?", "V", "Košiciach?"]
        ex["tokens_l1"] = ["Tu", "es", "là ?", "À", "Košice ?"]
        new = ot.rework(ex, [self.PAIRS], (1, 1), random.Random(0))
        self.assertEqual(new["tokens_l2"][:5], ex["tokens_l2"])
        self.assertEqual(len(new["tokens_l2"]), 6)
        self.assertEqual(len(new["tokens_l1"]), 6)

    def test_lone_punctuation_glued(self):
        ex = self.ex()
        ex["tokens_l2"] = ["Ty", "si", "tu?", "V", "Košiciach?", "nie"]
        ex["tokens_l1"] = ["Tu", "es", "là", "?", "À", "Košice", "?", "pas"]
        new = ot.rework(ex, [self.PAIRS], (2, 2), random.Random(0))
        self.assertEqual(new["tokens_l1"], ["Tu", "es", "là ?", "À", "Košice ?", "pas"])

    def test_falls_back_to_other_tiers(self):
        ex = {"type": "order", "id": "y", "l1": "Du pain et du thé", "l2": "Chlieb a čaj",
              "tokens_l1": ["Du pain", "et", "du thé"], "tokens_l2": ["Chlieb", "a", "čaj"]}
        series = [("Une grenouille verte", "Zelená žaba skáče")]
        new = ot.rework(ex, [[("Du pain et du thé", "Chlieb a čaj")], series], (1, 1), random.Random(0))
        self.assertIn(new["tokens_l2"][-1], {"žaba", "skáče"})

    def test_no_other_form_of_a_target_word(self):
        self.assertEqual(ot.parasite_words("Polievka je dobrá.", "Jem polievku dnes.", 3, random.Random(0)), ["dnes"])

    def test_sync_keeps_loose_parasites(self):
        new = ot.rework(self.ex(), [self.PAIRS], (2, 2), random.Random(0))
        self.assertIsNone(sync_exercises.refresh_parasite(new, set(self.PAIRS), random.Random(0)))


if __name__ == "__main__":
    unittest.main()
