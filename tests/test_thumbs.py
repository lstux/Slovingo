"""Tests for src/thumbs.py (square thumbnails; Pillow is optional)."""

import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "src"))

import thumbs  # noqa: E402


def make_data(*srcs):
    return {"groups": [{"sheets": [
        {"id": f"s{i}", "image": ({"src": src, "caption": ""} if src else None)}
        for i, src in enumerate(srcs)
    ]}]}


class ThumbsTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        root = Path(self.tmp.name)
        self.img = root / "img"
        (self.img / "vocabulary").mkdir(parents=True)
        self.dist = root / "dist"
        self.dist.mkdir()

    def tearDown(self):
        self.tmp.cleanup()

    @unittest.skipUnless(thumbs.PIL_AVAILABLE, "Pillow not installed")
    def test_square_thumbnail_is_made(self):
        from PIL import Image
        Image.new("RGB", (1280, 720), "orange").save(self.img / "vocabulary" / "a.jpg")
        data = make_data("img/vocabulary/a.jpg", None, "https://example.org/x.jpg")
        n = thumbs.add_thumbnails(data, self.img, self.dist)
        self.assertEqual(n, 1)
        sheets = data["groups"][0]["sheets"]
        self.assertTrue(sheets[0]["thumb"].startswith("thumbs/vocabulary__a."))
        self.assertNotIn("thumb", sheets[1])
        self.assertNotIn("thumb", sheets[2])  # remote image: left alone
        with Image.open(self.dist / sheets[0]["thumb"]) as im:
            self.assertEqual(im.size, (thumbs.THUMB_SIZE, thumbs.THUMB_SIZE))

    @unittest.skipUnless(thumbs.PIL_AVAILABLE, "Pillow not installed")
    def test_svg_and_missing_files_are_skipped(self):
        (self.img / "logo.svg").write_text("<svg xmlns='http://www.w3.org/2000/svg'/>")
        data = make_data("img/logo.svg", "img/missing.jpg")
        self.assertEqual(thumbs.add_thumbnails(data, self.img, self.dist), 0)

    def test_without_pillow_nothing_is_made(self):
        data = make_data("img/vocabulary/a.jpg")
        with mock.patch.object(thumbs, "PIL_AVAILABLE", False):
            self.assertEqual(thumbs.add_thumbnails(data, self.img, self.dist), 0)
        self.assertNotIn("thumb", data["groups"][0]["sheets"][0])
        self.assertFalse((self.dist / "thumbs").exists())


if __name__ == "__main__":
    unittest.main()
