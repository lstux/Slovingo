"""PocketBase sync is an opt-in build option: absent by default, wired in with --pocketbase."""

import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "src"))

import build  # noqa: E402
import build_site  # noqa: E402
import gen_service_worker  # noqa: E402

LANG = {
    "target_lang": {"code": "sk", "name": "slovaque", "tts_code": "sk-SK", "flag": "🇸🇰", "css_class": "col-target"},
    "native_lang": {"code": "fr", "name": "français", "tts_code": "fr-FR", "flag": "🇫🇷", "css_class": "col-native"},
    "site": {"title": "T", "short_name": "T", "description": "d", "theme_color": "#0b4ea2",
             "storage_prefix": "sk-fr", "url_path": "/", "user_name_default": "", "user_name_placeholder": "x"},
    "generator": {},
}
SHEET = "# Test\n\n| Français | Slovaque |\n|---|---|\n| Bonjour | Dobrý deň |\n"


class ServiceWorkerTest(unittest.TestCase):
    def test_no_placeholder_left_and_extra_assets_listed(self):
        plain = gen_service_worker.render(LANG)
        self.assertNotIn("__EXTRA_ASSETS__", plain)
        self.assertNotIn("pocketbase", plain)
        extra = gen_service_worker.render(LANG, ("pocketbase/a.js", "pocketbase/b.js"))
        self.assertIn('SCOPE + "pocketbase/a.js",', extra)
        self.assertIn('SCOPE + "pocketbase/b.js",', extra)


class BuildTest(unittest.TestCase):
    def build(self, **kwargs):
        (self.lang / "dist").mkdir(exist_ok=True)
        build.run_build(self.lang, ROOT / "static", **kwargs)
        return self.lang / "dist"

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.lang = Path(self.tmp.name) / "sk-fr"
        (self.lang / "md").mkdir(parents=True)
        (self.lang / "lang.json").write_text(json.dumps(LANG), encoding="utf-8")
        (self.lang / "md" / "01_Test_01_Un.md").write_text(SHEET, encoding="utf-8")

    def tearDown(self):
        self.tmp.cleanup()

    def test_default_build_has_no_pocketbase(self):
        dist = self.build()
        self.assertFalse((dist / "pocketbase").exists())
        self.assertNotIn("pocketbase", (dist / "index.html").read_text(encoding="utf-8"))
        self.assertNotIn("pocketbase", (dist / "service-worker.js").read_text(encoding="utf-8"))
        self.assertNotIn("pocketbase", json.loads((dist / "lang.json").read_text(encoding="utf-8")))

    def test_option_includes_scripts_in_order_and_url(self):
        dist = self.build(pocketbase=True, pocketbase_url="https://pb.example")
        html = (dist / "index.html").read_text(encoding="utf-8")
        order = [html.index(s) for s in ("backup.js", "pocketbase/pbstore.js", "pocketbase/pocketbase.js")]
        self.assertEqual(order, sorted(order))
        self.assertTrue((dist / "pocketbase" / "pocketbase.js").exists())
        self.assertIn("pocketbase/pocketbase.js", (dist / "service-worker.js").read_text(encoding="utf-8"))
        self.assertEqual(json.loads((dist / "lang.json").read_text(encoding="utf-8"))["pocketbase"], {"url": "https://pb.example"})

    def test_rebuild_without_option_removes_it(self):
        self.build(pocketbase=True, pocketbase_url="https://pb.example")
        dist = self.build()
        self.assertFalse((dist / "pocketbase").exists())
        self.assertNotIn("pocketbase", (dist / "index.html").read_text(encoding="utf-8"))
        self.assertNotIn("pocketbase", json.loads((dist / "lang.json").read_text(encoding="utf-8")))


class BundleTest(unittest.TestCase):
    """build_site.build_course passes the PocketBase options on to build.py."""

    def run_course(self, **kwargs):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "content" / "sk-fr").mkdir(parents=True)
            (root / "content" / "sk-fr" / "lang.json").write_text(json.dumps(LANG), encoding="utf-8")
            (root / "work").mkdir()
            calls = []
            with mock.patch.object(build_site.subprocess, "run", side_effect=lambda cmd, **kw: calls.append(cmd)), \
                 mock.patch.object(build_site, "read_launcher_entry", create=True, return_value={}):
                try:
                    build_site.build_course("sk-fr", root / "content", ROOT / "static", "/S/", root / "work", root / "out", **kwargs)
                except Exception:
                    pass  # only the first command (build.py) matters here
            return next(c for c in calls if "build.py" in c[1])

    def test_default_has_no_flag(self):
        cmd = self.run_course()
        self.assertNotIn("--pocketbase", cmd)
        self.assertNotIn("--pocketbase-url", cmd)

    def test_flag_and_url(self):
        self.assertIn("--pocketbase", self.run_course(pocketbase=True))
        cmd = self.run_course(pocketbase_url="https://pb.example")
        self.assertEqual(cmd[cmd.index("--pocketbase-url") + 1], "https://pb.example")


if __name__ == "__main__":
    unittest.main()
