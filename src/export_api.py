#!/usr/bin/env python3
"""export_api.py

Export a language dir as a compact, per-sheet JSON set: a small static API
for light clients (smartwatch, widget, bot...) that cannot afford to download
and parse the full dist/data.json. Any static host can serve it, e.g. GitHub
Pages.

Output layout (default: <lang-dir>/dist-api/):

    index.json              course metadata + list of sheets (no content)
    sheets/<id>.json        one sheet, reduced to simple text blocks
    exercises/<id>.json     that sheet's exercises (only if it has any)

Each sheet entry of index.json has a "theme" when lang.json's "subgroup_themes" gives one
to its subgroup (basics, family, house, food...: the themes of the HTML site), so that a
client can pick a pictogram and a colour per series. index.json also carries the optional
"greeting" of lang.json ({"t":"Ahoj, [USER_NAME]!","n":"Salut, [USER_NAME] !"}, welcome
line of the home screen, [USER_NAME] stands for the player's name) and the optional "cheers" of lang.json (little phrases of the
end-of-quiz screen, one list per score tier, see docs/Lang-json.md):

    "cheers":{"perfect":[{"t":"Výborne!","n":"Excellent !"},..],"great":[..],
              "good":[..],"ok":[..],"retry":[..]}   t = target language, n = translation

This is an *additional* output: it reads the same sources as build.py
(md/, exercises/ or json/, lang.json) and touches nothing else.

Sheet block format ("t" = type, minified JSON):

    {"t":"h","l":2,"x":"Title"}                       heading (l = level)
    {"t":"p","x":"Text"}                              paragraph
    {"t":"table","cols":[..],"rows":[[..],..]}        table
    {"t":"card","x":"Ahoj!","n":"Salut !",            audio-card: x = phrase in
               "s":"🐰","lit":[..],"nt":[..]}          target language, n = natural
                                                      translation, s = speaker,
                                                      lit = word-by-word, nt = notes
    {"t":"list","items":[..],"o":false}               list (o = ordered)

    {"t":"img","src":"img/<id>-0.jpg","x":"Caption","w":280,"h":140}
                                                      illustration: a small baseline JPEG
                                                      exported next to the sheets (see below)

Images are optional: only local ones (img/... of the course, not yet an http URL) are
exported, shrunk to fit IMAGE_MAX_W x IMAGE_MAX_H and re-encoded as baseline JPEG into
<out>/img/. Without Pillow, with --no-images, or when a file is missing or unreadable,
the image is left out (with a warning) and the export goes on. Clients that do not know
the "img" block ignore it.

Dropped on purpose: horizontal rules, maps. Inline markup
([[word]], [[fr:text]], **bold**) is flattened to plain text.

Exercises are copied as generated (same fields as exercises.json), minus
the file-level "mode"/"sync" keys.

Usage:
    python3 src/export_api.py --lang-dir langs/sk-fr-kids [-o OUT_DIR]
                                [--max-sheet-bytes 20000]
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from collections import Counter
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent))

from smd2data import build_sheet_json  # noqa: E402

DEFAULT_MAX_SHEET_BYTES = 20_000

# Illustrations are drawn on a 320x240 screen next to a few lines of text.
IMAGE_MAX_W = 280
IMAGE_MAX_H = 140
IMAGE_QUALITY = 72

# [[fr:text]] / [[word]] (speakable markup) and **bold**.
_SPEAKABLE_RE = re.compile(r"\[\[(?:[a-z]{2,3}:)?(.*?)\]\]", re.DOTALL)
_BOLD_RE = re.compile(r"\*\*(.+?)\*\*", re.DOTALL)


def flatten(text: Any) -> str:
    """Strip inline SMD markup, keeping the visible text."""
    if not isinstance(text, str):
        return ""
    text = _SPEAKABLE_RE.sub(r"\1", text)
    text = _BOLD_RE.sub(r"\1", text)
    return text.strip()


class ImageExporter:
    """Shrinks the local illustrations of a course into <out>/img/ (optional, never fatal)."""

    def __init__(self, lang_dir: Path, out_dir: Path, enabled: bool = True) -> None:
        self.lang_dir = lang_dir
        self.out_dir = out_dir
        self.enabled = enabled
        self.count = 0
        self.bytes = 0

    def export(self, src: str, caption: str, name: str) -> dict[str, Any] | None:
        """The "img" block for one illustration, or None when it cannot be exported."""
        if not self.enabled or not src:
            return None
        if re.match(r"^[a-z][a-z0-9+.-]*://", src, re.IGNORECASE):
            print(f"warning: {name}: remote image {src} not exported (fetch_images.py brings it home)", file=sys.stderr)
            return None
        path = (self.lang_dir / src).resolve()
        try:
            path.relative_to(self.lang_dir.resolve())
        except ValueError:
            print(f"warning: {name}: image outside the course dir skipped: {src}", file=sys.stderr)
            return None
        if not path.is_file():
            print(f"warning: {name}: image not found: {src}", file=sys.stderr)
            return None
        try:
            from PIL import Image, ImageOps  # imported late: Pillow is optional here
        except ImportError:
            print("warning: Pillow is not installed, images are not exported", file=sys.stderr)
            self.enabled = False
            return None
        try:
            with Image.open(path) as img:
                img = ImageOps.exif_transpose(img)
                if img.mode in ("RGBA", "LA", "P"):
                    img = img.convert("RGBA")
                    background = Image.new("RGB", img.size, (255, 255, 255))
                    background.paste(img, mask=img.getchannel("A"))
                    img = background
                else:
                    img = img.convert("RGB")
                img.thumbnail((IMAGE_MAX_W, IMAGE_MAX_H), Image.LANCZOS)
                target = self.out_dir / "img" / f"{name}.jpg"
                target.parent.mkdir(parents=True, exist_ok=True)
                # baseline (not progressive): the light clients decode it with a tiny decoder
                img.save(target, "JPEG", quality=IMAGE_QUALITY, optimize=True, progressive=False)
                width, height = img.size
        except Exception as exc:  # a broken picture must not break the export
            print(f"warning: {name}: image {src} skipped ({exc})", file=sys.stderr)
            return None
        self.count += 1
        self.bytes += target.stat().st_size
        block: dict[str, Any] = {"t": "img", "src": f"img/{name}.jpg", "w": width, "h": height}
        caption = flatten(caption)
        if caption:
            block["x"] = caption
        return block


def compact_block(block: dict[str, Any]) -> dict[str, Any] | None:
    """Reduce one content block, or None if the API leaves it out."""
    kind = block.get("type")
    if kind == "heading":
        return {"t": "h", "l": block.get("level", 2), "x": flatten(block.get("text"))}
    if kind == "paragraph":
        text = flatten(block.get("text"))
        return {"t": "p", "x": text} if text else None
    if kind == "table":
        return {
            "t": "table",
            "cols": [flatten(c) for c in block.get("columns", [])],
            "rows": [[flatten(c) for c in row] for row in block.get("rows", [])],
        }
    if kind == "audio-card":
        out: dict[str, Any] = {
            "t": "card",
            "x": flatten(block.get("phrase")),
            "n": flatten(block.get("natural")),
        }
        if block.get("speaker"):
            out["s"] = block["speaker"]
        if block.get("literal"):
            out["lit"] = [flatten(x) for x in block["literal"]]
        if block.get("notes"):
            out["nt"] = [flatten(x) for x in block["notes"]]
        return out
    if kind == "list":
        return {
            "t": "list",
            "o": bool(block.get("ordered")),
            "items": [flatten(i) for i in block.get("items", [])],
        }
    return None  # hr, map, anything unknown


def compact_sheet(sheet: dict[str, Any], images: ImageExporter | None = None) -> dict[str, Any]:
    blocks: list[dict[str, Any]] = []
    shown = 0

    def add_image(src: str, caption: str) -> None:
        nonlocal shown
        if images is None:
            return
        out = images.export(src, caption, f"{sheet['id']}-{shown}")
        if out:
            blocks.append(out)
            shown += 1

    lead = sheet.get("image")
    if lead:
        add_image(lead.get("src", ""), lead.get("caption", ""))
    for block in sheet["content"]:
        if block.get("type") == "image":
            add_image(block.get("src", ""), block.get("caption", ""))
            continue
        compact = compact_block(block)
        if compact:
            blocks.append(compact)
    return {"id": sheet["id"], "title": flatten(sheet["title"]), "blocks": blocks}


def dumps(obj: Any) -> str:
    return json.dumps(obj, ensure_ascii=False, separators=(",", ":"))


def load_exercises(lang_dir: Path, md_stem: str, sheet_id: str) -> list[dict[str, Any]]:
    """Exercises for a sheet. exercises/ (committed, may be hand-edited,
    named after the .md file) wins over json/ (build intermediates,
    named after the sheet id)."""
    candidates = (
        lang_dir / "exercises" / f"{md_stem}.exercises.json",
        lang_dir / "exercises" / f"{sheet_id}.exercises.json",
        lang_dir / "json" / f"{sheet_id}.exercises.json",
    )
    for path in candidates:
        if path.exists():
            return json.loads(path.read_text(encoding="utf-8")).get("exercises", [])
    return []


CHEER_TIERS = ("perfect", "great", "good", "ok", "retry")


def compact_cheers(lang_cfg: dict[str, Any]) -> dict[str, list[dict[str, str]]]:
    """The optional "cheers" of lang.json reduced to {tier: [{"t":..,"n":..}]}.

    Unknown tiers and malformed entries are skipped (with a warning) so that a typo
    in a course never breaks the export; empty tiers are left out.
    """
    raw = lang_cfg.get("cheers")
    if not isinstance(raw, dict):
        return {}
    out: dict[str, list[dict[str, str]]] = {}
    for tier in CHEER_TIERS:
        phrases = []
        for entry in raw.get(tier) or []:
            text = flatten(entry.get("t", "")) if isinstance(entry, dict) else ""
            if not text:
                print(f"warning: cheers.{tier}: entry without \"t\" skipped: {entry!r}", file=sys.stderr)
                continue
            phrase = {"t": text}
            translation = flatten(entry.get("n", ""))
            if translation:
                phrase["n"] = translation
            phrases.append(phrase)
        if phrases:
            out[tier] = phrases
    for tier in raw:
        if tier not in CHEER_TIERS:
            print(f"warning: cheers.{tier}: unknown tier ignored", file=sys.stderr)
    return out


def compact_greeting(lang_cfg: dict[str, Any]) -> dict[str, str]:
    """The optional "greeting" of lang.json as {"t":..,"n":..} (n facultative), {} if absent or malformed."""
    raw = lang_cfg.get("greeting")
    if not isinstance(raw, dict):
        return {}
    text = flatten(raw.get("t", ""))
    if not text:
        print(f"warning: greeting without \"t\" ignored: {raw!r}", file=sys.stderr)
        return {}
    greeting = {"t": text}
    translation = flatten(raw.get("n", ""))
    if translation:
        greeting["n"] = translation
    return greeting


def export(lang_dir: Path, out_dir: Path, max_sheet_bytes: int, with_images: bool = True) -> dict[str, Any]:
    lang_cfg = json.loads((lang_dir / "lang.json").read_text(encoding="utf-8"))
    md_files = sorted((lang_dir / "md").glob("*.md"))
    if not md_files:
        raise SystemExit(f"No .md sheets in {lang_dir / 'md'}")

    (out_dir / "sheets").mkdir(parents=True, exist_ok=True)
    (out_dir / "exercises").mkdir(parents=True, exist_ok=True)

    themes = lang_cfg.get("subgroup_themes") or {}
    images = ImageExporter(lang_dir, out_dir, with_images)
    entries = []
    for md in md_files:
        sheet = build_sheet_json(md, lang_cfg)
        compact = compact_sheet(sheet, images)
        body = dumps(compact)
        size = len(body.encode("utf-8"))
        if size > max_sheet_bytes:
            print(f"warning: {sheet['id']} is {size} bytes (> {max_sheet_bytes})", file=sys.stderr)
        (out_dir / "sheets" / f"{sheet['id']}.json").write_text(body, encoding="utf-8")

        exercises = load_exercises(lang_dir, md.stem, sheet["id"])
        if exercises:
            (out_dir / "exercises" / f"{sheet['id']}.json").write_text(
                dumps({"id": sheet["id"], "exercises": exercises}), encoding="utf-8"
            )

        entry = {
            "id": sheet["id"],
            "cat": sheet["category"],
            "sub": sheet["subgroup"],
            "sub_label": sheet["subgroup_label"],
            "title": compact["title"],
            "bytes": size,
            "ex": dict(Counter(e["type"] for e in exercises)),
        }
        theme = themes.get(sheet["subgroup"]) if sheet["category"] == "series" else None
        if isinstance(theme, str) and theme:
            entry["theme"] = theme
        entries.append(entry)

    site = lang_cfg.get("site", {})
    index = {
        "version": 1,
        "course": site.get("storage_prefix") or lang_dir.name,
        "title": site.get("title", lang_dir.name),
        "target": {k: lang_cfg["target_lang"].get(k) for k in ("code", "name", "tts_code")},
        "native": {k: lang_cfg["native_lang"].get(k) for k in ("code", "name", "tts_code")},
        "sheets": entries,
    }
    greeting = compact_greeting(lang_cfg)
    if greeting:
        index["greeting"] = greeting
    cheers = compact_cheers(lang_cfg)
    if cheers:
        index["cheers"] = cheers
    (out_dir / "index.json").write_text(dumps(index), encoding="utf-8")
    if images.count:
        print(f"API export: {images.count} image(s), {images.bytes} bytes -> {out_dir / 'img'}")
    return index


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--lang-dir", type=Path, required=True)
    parser.add_argument("-o", "--out-dir", type=Path, default=None,
                        help="Output dir (default: <lang-dir>/dist-api)")
    parser.add_argument("--max-sheet-bytes", type=int, default=DEFAULT_MAX_SHEET_BYTES,
                        help="Warn when a compact sheet exceeds this size")
    parser.add_argument("--no-images", action="store_true",
                        help="Do not export the illustrations (no img blocks, no img/ folder)")
    args = parser.parse_args()

    out_dir = args.out_dir or (args.lang_dir / "dist-api")
    index = export(args.lang_dir, out_dir, args.max_sheet_bytes, with_images=not args.no_images)
    total = sum(s["bytes"] for s in index["sheets"])
    print(f"API export: {len(index['sheets'])} sheet(s), {total} bytes of sheets -> {out_dir}")


if __name__ == "__main__":
    main()
