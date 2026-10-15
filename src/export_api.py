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

Dropped on purpose: images, horizontal rules, maps. Inline markup
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


def compact_sheet(sheet: dict[str, Any]) -> dict[str, Any]:
    blocks = [b for b in map(compact_block, sheet["content"]) if b]
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


def export(lang_dir: Path, out_dir: Path, max_sheet_bytes: int) -> dict[str, Any]:
    lang_cfg = json.loads((lang_dir / "lang.json").read_text(encoding="utf-8"))
    md_files = sorted((lang_dir / "md").glob("*.md"))
    if not md_files:
        raise SystemExit(f"No .md sheets in {lang_dir / 'md'}")

    (out_dir / "sheets").mkdir(parents=True, exist_ok=True)
    (out_dir / "exercises").mkdir(parents=True, exist_ok=True)

    entries = []
    for md in md_files:
        sheet = build_sheet_json(md, lang_cfg)
        compact = compact_sheet(sheet)
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

        entries.append(
            {
                "id": sheet["id"],
                "cat": sheet["category"],
                "sub": sheet["subgroup"],
                "sub_label": sheet["subgroup_label"],
                "title": compact["title"],
                "bytes": size,
                "ex": dict(Counter(e["type"] for e in exercises)),
            }
        )

    site = lang_cfg.get("site", {})
    index = {
        "version": 1,
        "course": site.get("storage_prefix") or lang_dir.name,
        "title": site.get("title", lang_dir.name),
        "target": {k: lang_cfg["target_lang"].get(k) for k in ("code", "name", "tts_code")},
        "native": {k: lang_cfg["native_lang"].get(k) for k in ("code", "name", "tts_code")},
        "sheets": entries,
    }
    (out_dir / "index.json").write_text(dumps(index), encoding="utf-8")
    return index


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--lang-dir", type=Path, required=True)
    parser.add_argument("-o", "--out-dir", type=Path, default=None,
                        help="Output dir (default: <lang-dir>/dist-api)")
    parser.add_argument("--max-sheet-bytes", type=int, default=DEFAULT_MAX_SHEET_BYTES,
                        help="Warn when a compact sheet exceeds this size")
    args = parser.parse_args()

    out_dir = args.out_dir or (args.lang_dir / "dist-api")
    index = export(args.lang_dir, out_dir, args.max_sheet_bytes)
    total = sum(s["bytes"] for s in index["sheets"])
    print(f"API export: {len(index['sheets'])} sheet(s), {total} bytes of sheets -> {out_dir}")


if __name__ == "__main__":
    main()
