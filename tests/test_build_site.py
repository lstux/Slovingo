"""Tests for src/build_site.py: the lang.json adaptations done for the bundle."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "src"))

import build_site  # noqa: E402


def make_cfg(native="fr", ui=None):
    cfg = {
        "target_lang": {"code": "sk"},
        "native_lang": {"code": native},
        "site": {"title": "T", "storage_prefix": "sk-fr-test", "url_path": "/old/"},
    }
    if ui is not None:
        cfg["ui"] = ui
    return cfg


class ApplyBundleSettingsTest(unittest.TestCase):
    def test_url_path_and_launcher_url(self):
        cfg = build_site.apply_bundle_settings(make_cfg(), "/Slovingo/", "sk-fr-kids")
        self.assertEqual(cfg["site"]["url_path"], "/Slovingo/sk-fr-kids/")
        self.assertEqual(cfg["site"]["launcher_url"], "../")
        self.assertEqual(cfg["site"]["storage_prefix"], "sk-fr-test")  # untouched

    def test_default_label_follows_native_language(self):
        fr = build_site.apply_bundle_settings(make_cfg("fr"), "/S/", "x")
        sk = build_site.apply_bundle_settings(make_cfg("sk"), "/S/", "x")
        self.assertEqual(fr["ui"]["change_course"], "Changer de cours")
        self.assertEqual(sk["ui"]["change_course"], "Zmeniť kurz")

    def test_course_label_wins(self):
        cfg = build_site.apply_bundle_settings(make_cfg(ui={"change_course": "Autre cours"}), "/S/", "x")
        self.assertEqual(cfg["ui"]["change_course"], "Autre cours")

    def test_unknown_native_language_leaves_label_unset(self):
        cfg = build_site.apply_bundle_settings(make_cfg("xx"), "/S/", "x")
        self.assertNotIn("change_course", cfg["ui"])

    def test_normalize_base_path(self):
        self.assertEqual(build_site.normalize_base_path("Slovingo"), "/Slovingo/")
        self.assertEqual(build_site.normalize_base_path("/Slovingo/"), "/Slovingo/")
        self.assertEqual(build_site.normalize_base_path("/"), "/")


if __name__ == "__main__":
    unittest.main()
