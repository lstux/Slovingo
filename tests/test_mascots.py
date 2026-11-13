"""Tests for src/mascots.py (course mascot pictures) and the SMD "@@" block."""

import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "src"))

import export_api  # noqa: E402
import mascots  # noqa: E402
import smd2data  # noqa: E402


class MascotPicturesTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        root = Path(self.tmp.name)
        self.img = root / "img"
        (self.img / "mascots").mkdir(parents=True)
        self.dist = root / "dist"
        self.dist.mkdir()

    def tearDown(self):
        self.tmp.cleanup()

    def _png(self, name, size=(800, 800)):
        from PIL import Image
        Image.new("RGBA", size, (255, 120, 0, 0)).save(self.img / "mascots" / name)

    def test_keys(self):
        self.assertEqual(mascots.mascot_key("fox-content.png"), "fox-content")
        self.assertEqual(mascots.mascot_key("Rabbit-Bravo.PNG"), "hare-bravo")
        self.assertEqual(mascots.mascot_key("duo-walk.webp"), "duo-walk")
        self.assertIsNone(mascots.mascot_key("cat-content.png"))
        self.assertIsNone(mascots.mascot_key("fox.png"))

    def test_no_folder_no_key(self):
        data = {}
        self.assertEqual(mascots.add_mascots(data, self.img.parent / "nothing", self.dist), 0)
        self.assertNotIn("mascot_pictures", data)

    @unittest.skipUnless(mascots.PIL_AVAILABLE, "Pillow missing")
    def test_shrunk_with_alpha(self):
        from PIL import Image
        self._png("fox-content.png")
        self._png("rabbit-think.png")
        data = {}
        self.assertEqual(mascots.add_mascots(data, self.img, self.dist), 2)
        pics = data["mascot_pictures"]
        self.assertEqual(set(pics), {"fox-content", "hare-think"})
        out = self.dist / pics["fox-content"]
        with Image.open(out) as im:
            self.assertLessEqual(max(im.size), mascots.MAX_SIZE)
            self.assertIn("A", im.getbands())

    def test_without_pillow_original_used(self):
        self._png("hare-bravo.png") if mascots.PIL_AVAILABLE else (self.img / "mascots" / "hare-bravo.png").write_bytes(b"x")
        data = {}
        with mock.patch.object(mascots, "PIL_AVAILABLE", False):
            mascots.add_mascots(data, self.img, self.dist)
        self.assertEqual(data["mascot_pictures"], {"hare-bravo": "img/mascots/hare-bravo.png"})

    def test_hare_wins_over_rabbit(self):
        for name in ("hare-content.svg", "rabbit-content.svg"):
            (self.img / "mascots" / name).write_text("<svg/>")
        data = {}
        mascots.add_mascots(data, self.img, self.dist)
        self.assertEqual(data["mascot_pictures"], {"hare-content": "img/mascots/hare-content.svg"})


class MascotBlockTest(unittest.TestCase):
    def parse(self, text):
        return smd2data.parse_content(text, [], [])

    def test_with_text(self):
        [b] = self.parse("@@ hare:think | Le savais-tu ?")
        self.assertEqual(b, {"type": "mascot", "who": "hare", "mood": "think", "text": "Le savais-tu ?"})

    def test_defaults_and_alias(self):
        [a, b] = self.parse("@@ fox\n@@ Rabbit:BRAVO")
        self.assertEqual((a["who"], a["mood"], a["text"]), ("fox", "content", ""))
        self.assertEqual((b["who"], b["mood"]), ("hare", "bravo"))

    def test_unknown_stays_text(self):
        [b] = self.parse("@@ cat:bravo | x")
        self.assertEqual(b["type"], "paragraph")

    def test_image_line_untouched(self):
        [b] = self.parse("@ img/a.jpg | Légende")
        self.assertEqual(b["type"], "image")

    def test_api_keeps_text(self):
        self.assertEqual(export_api.compact_block({"type": "mascot", "who": "fox", "mood": "think", "text": "Hm"}),
                         {"t": "p", "x": "Hm"})
        self.assertIsNone(export_api.compact_block({"type": "mascot", "who": "fox", "mood": "think", "text": ""}))


if __name__ == "__main__":
    unittest.main()
