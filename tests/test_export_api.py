"""Tests for src/export_api.py (self-contained: builds a tiny course in a tmp dir)."""

import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "src"))

import export_api  # noqa: E402

LANG = {
    "target_lang": {"code": "sk", "name": "slovaque", "tts_code": "sk-SK"},
    "native_lang": {"code": "fr", "name": "français", "tts_code": "fr-FR"},
    "site": {"title": "Test", "storage_prefix": "sk-fr-test"},
}

SHEET = """# Kit de Survie (1/3) — Ahoj!

@ https://example.org/pic.jpg | Une image

{{fr:Pour commencer, touche les mots !}}

---

## Les nouveaux mots

| Slovenčina | Français |
|------------|----------|
| ahoj | salut |

! Ahoj!
> Salut !
> Ahoj = salut
+ Pour un copain.
"""

EXERCISES = {
    "mode": "replace",
    "sync": "full",
    "exercises": [{"id": "qcm-ahoj", "type": "qcm", "l1": "salut", "l2": "ahoj",
                   "choices_l1": ["merci"], "choices_l2": ["ďakujem"]}],
}


class WatchExportTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.lang_dir = Path(self.tmp.name) / "sk-fr-test"
        (self.lang_dir / "md").mkdir(parents=True)
        (self.lang_dir / "exercises").mkdir()
        (self.lang_dir / "lang.json").write_text(json.dumps(LANG), encoding="utf-8")
        stem = "10_Series_00_KitSurvie_01_salutations"
        (self.lang_dir / "md" / f"{stem}.md").write_text(SHEET, encoding="utf-8")
        (self.lang_dir / "exercises" / f"{stem}.exercises.json").write_text(
            json.dumps(EXERCISES), encoding="utf-8")
        self.out = Path(self.tmp.name) / "out"
        self.index = export_api.export(self.lang_dir, self.out, 20_000)

    def tearDown(self):
        self.tmp.cleanup()

    def test_index(self):
        self.assertEqual(self.index["course"], "sk-fr-test")
        self.assertEqual(self.index["target"]["tts_code"], "sk-SK")
        (entry,) = self.index["sheets"]
        self.assertEqual(entry["id"], "series-kitsurvie-salutations")
        self.assertEqual(entry["ex"], {"qcm": 1})
        on_disk = json.loads((self.out / "index.json").read_text(encoding="utf-8"))
        self.assertEqual(on_disk, self.index)

    def test_sheet_is_flat_text_without_images_or_markup(self):
        sheet = json.loads((self.out / "sheets" / "series-kitsurvie-salutations.json")
                           .read_text(encoding="utf-8"))
        types = [b["t"] for b in sheet["blocks"]]
        self.assertEqual(types, ["p", "h", "table", "card"])
        self.assertEqual(sheet["blocks"][0]["x"], "Pour commencer, touche les mots !")
        card = sheet["blocks"][3]
        self.assertEqual((card["x"], card["n"]), ("Ahoj!", "Salut !"))
        self.assertEqual(card["lit"], ["Ahoj = salut"])
        texts = [sheet["blocks"][0]["x"], card["x"], card["n"], *card["lit"], *card["nt"]]
        self.assertFalse(any("[[" in t or "**" in t for t in texts))

    def test_exercises_copied_without_file_level_keys(self):
        ex = json.loads((self.out / "exercises" / "series-kitsurvie-salutations.json")
                        .read_text(encoding="utf-8"))
        self.assertEqual(set(ex), {"id", "exercises"})
        self.assertEqual(ex["exercises"], EXERCISES["exercises"])

    def test_theme_and_greeting(self):
        self.assertNotIn("theme", self.index["sheets"][0])
        self.assertNotIn("greeting", self.index)
        cfg = dict(LANG, subgroup_themes={"kitsurvie": "basics"},
                   greeting={"t": "Ahoj, [USER_NAME]!", "n": "Salut, [USER_NAME] !"})
        (self.lang_dir / "lang.json").write_text(json.dumps(cfg), encoding="utf-8")
        index = export_api.export(self.lang_dir, self.out, 20_000)
        self.assertEqual(index["sheets"][0]["theme"], "basics")
        self.assertEqual(index["greeting"], {"t": "Ahoj, [USER_NAME]!", "n": "Salut, [USER_NAME] !"})

    def test_no_cheers_by_default(self):
        self.assertNotIn("cheers", self.index)

    def test_cheers_are_exported_and_cleaned(self):
        cfg = dict(LANG, cheers={
            "perfect": [{"t": "Výborne!", "n": "Excellent !"}, {"t": "Hotovo"}, {"n": "sans t"}, "texte"],
            "good": [],
            "great": [{"t": "[[Super]]!", "n": "**Super** !"}],
            "bogus": [{"t": "x"}],
        })
        (self.lang_dir / "lang.json").write_text(json.dumps(cfg), encoding="utf-8")
        index = export_api.export(self.lang_dir, self.out, 20_000)
        self.assertEqual(index["cheers"], {
            "perfect": [{"t": "Výborne!", "n": "Excellent !"}, {"t": "Hotovo"}],
            "great": [{"t": "Super!", "n": "Super !"}],
        })

    def _sheet_with_local_image(self, extra=""):
        from PIL import Image
        (self.lang_dir / "img").mkdir(exist_ok=True)
        Image.new("RGB", (800, 600), (200, 30, 30)).save(self.lang_dir / "img" / "pic.png")
        md = next((self.lang_dir / "md").glob("*.md"))
        text = md.read_text(encoding="utf-8").replace("https://example.org/pic.jpg", "img/pic.png")
        md.write_text(text + extra, encoding="utf-8")

    def test_local_image_is_shrunk_to_a_baseline_jpeg(self):
        from PIL import Image
        self._sheet_with_local_image("\n@ img/pic.png | Encore\n")
        out = Path(self.tmp.name) / "out2"
        export_api.export(self.lang_dir, out, 20_000)
        sheet = json.loads((out / "sheets" / "series-kitsurvie-salutations.json").read_text(encoding="utf-8"))
        images = [b for b in sheet["blocks"] if b["t"] == "img"]
        self.assertEqual([b["src"] for b in images],
                         ["img/series-kitsurvie-salutations-0.jpg", "img/series-kitsurvie-salutations-1.jpg"])
        self.assertEqual(sheet["blocks"][0]["t"], "img")
        self.assertEqual(sheet["blocks"][0]["x"], "Une image")
        with Image.open(out / images[0]["src"]) as jpg:
            self.assertEqual(jpg.format, "JPEG")
            self.assertFalse(jpg.info.get("progressive") or jpg.info.get("progression"))
            self.assertEqual(jpg.size, (images[0]["w"], images[0]["h"]))
            self.assertLessEqual(jpg.size[0], export_api.IMAGE_MAX_W)
            self.assertLessEqual(jpg.size[1], export_api.IMAGE_MAX_H)

    def test_images_are_optional(self):
        self._sheet_with_local_image()
        out = Path(self.tmp.name) / "out3"
        export_api.export(self.lang_dir, out, 20_000, with_images=False)
        self.assertFalse((out / "img").exists())
        sheet = json.loads((out / "sheets" / "series-kitsurvie-salutations.json").read_text(encoding="utf-8"))
        self.assertNotIn("img", [b["t"] for b in sheet["blocks"]])

    def test_missing_image_does_not_break_the_export(self):
        md = next((self.lang_dir / "md").glob("*.md"))
        md.write_text(md.read_text(encoding="utf-8").replace("https://example.org/pic.jpg", "img/absent.jpg"),
                      encoding="utf-8")
        out = Path(self.tmp.name) / "out4"
        export_api.export(self.lang_dir, out, 20_000)
        sheet = json.loads((out / "sheets" / "series-kitsurvie-salutations.json").read_text(encoding="utf-8"))
        self.assertEqual([b["t"] for b in sheet["blocks"]], ["p", "h", "table", "card"])

    def test_flatten(self):
        self.assertEqual(export_api.flatten("[[fr:Bravo]] **ok** [[ahoj]]"), "Bravo ok ahoj")


if __name__ == "__main__":
    unittest.main()
