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

    def test_flatten(self):
        self.assertEqual(export_api.flatten("[[fr:Bravo]] **ok** [[ahoj]]"), "Bravo ok ahoj")


if __name__ == "__main__":
    unittest.main()
