"""Tests for tools/slovingo_push.py: when each notification is due."""

import sys
import unittest
from datetime import datetime, timezone
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))

import slovingo_push as sp  # noqa: E402

# Friday 2026-11-20, 18:03 in Paris (17:03 UTC).
NOW = datetime(2026, 11, 20, 17, 3, tzinfo=timezone.utc)


def cfg(**kw):
    base = {"tz": "Europe/Paris", "lang": "fr", "course": {"title": "Ahoj", "url": "https://x.org/sk-fr/"},
            "reminder": {"on": False}, "streak": {"on": False}, "training": []}
    base.update(kw)
    return base


def kinds(c, visits=(), state=None, now=NOW):
    return [k for k, _, _ in sp.due(c, set(visits), now, state if state is not None else {})]


class TrainingTest(unittest.TestCase):
    def test_sent_in_window_on_chosen_day_once(self):
        c = cfg(training=[{"time": "18:00", "days": [4]}])   # Friday
        self.assertEqual(kinds(c), ["training"])
        state = {}
        for _, _, upd in sp.due(c, set(), NOW, state):
            state.update(upd)
        self.assertEqual(kinds(c, state=state), [])

    def test_wrong_day_or_too_late(self):
        self.assertEqual(kinds(cfg(training=[{"time": "18:00", "days": [0]}])), [])
        self.assertEqual(kinds(cfg(training=[{"time": "17:50", "days": [4]}])), [])
        self.assertEqual(kinds(cfg(training=[{"time": "18:05", "days": [4]}])), [])

    def test_payload_opens_review(self):
        p = sp.payload("training", cfg(), {})
        self.assertEqual(p["url"], "https://x.org/sk-fr/#/exercises/review?n=5")
        self.assertEqual(p["title"], "Ahoj")


class ReminderTest(unittest.TestCase):
    R = {"on": True, "days": 3, "time": "18:00"}

    def test_after_n_idle_days(self):
        self.assertEqual(kinds(cfg(reminder=self.R), ["2026-11-17"]), ["reminder"])
        self.assertEqual(kinds(cfg(reminder=self.R), ["2026-11-18"]), [])
        self.assertEqual(kinds(cfg(reminder=self.R), []), [])   # never used: nothing

    def test_not_again_before_n_days(self):
        self.assertEqual(kinds(cfg(reminder=self.R), ["2026-11-10"], {"reminder": "2026-11-18"}), [])
        self.assertEqual(kinds(cfg(reminder=self.R), ["2026-11-10"], {"reminder": "2026-11-17"}), ["reminder"])


class StreakTest(unittest.TestCase):
    S = {"on": True, "time": "18:00"}

    def test_running_streak_not_done_today(self):
        out = sp.due(cfg(streak=self.S), {"2026-11-18", "2026-11-19"}, NOW, {})
        self.assertEqual([(k, f) for k, f, _ in out], [("streak", {"streak": 2})])

    def test_done_today_or_no_streak(self):
        self.assertEqual(kinds(cfg(streak=self.S), ["2026-11-19", "2026-11-20"]), [])
        self.assertEqual(kinds(cfg(streak=self.S), ["2026-11-17"]), [])


class TestTest(unittest.TestCase):
    def test_once_per_request(self):
        self.assertEqual(kinds(cfg(testAt=5)), ["test"])
        self.assertEqual(kinds(cfg(testAt=5), state={"test": 5}), [])

    def test_bad_timezone_falls_back_to_utc(self):
        self.assertEqual(kinds(cfg(tz="Nowhere/Land", training=[{"time": "17:00", "days": [4]}])), ["training"])


if __name__ == "__main__":
    unittest.main()
