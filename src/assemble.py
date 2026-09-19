#!/usr/bin/env python3
"""assemble.py

Merge the per-sheet intermediate JSON files (produced by smd2data.py
and smd2exercises.py) into the two files the browser actually fetches:

    dist/data.json       -- all sheet content, grouped and ordered
    dist/exercises.json  -- all sheet exercises, keyed by sheet id

Ordering source of truth: the .md filenames in md_dir. Content and
exercise JSON files intentionally do not carry the numeric order
prefixes (an id must survive renumbering), so this script re-derives
display order from the current filenames each time it runs, the same
way smd2data.py and smd2exercises.py do.

Usage:
    python3 assemble.py --md md_dir/ --json json_dir/ -o dist_dir/
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from collections import Counter, OrderedDict
from pathlib import Path
from typing import Any

from smd2data import SheetNameError, parse_sheet_filename

# A "Les personnages" heading (case/diacritic-insensitive on the key
# word) followed by a list block is the documented convention for
# introducing a dialogue's cast (see Fiches-Serie.txt /
# Fiches-Dialogue.txt) -- used here to harvest an emoji -> name map
# automatically, rather than maintaining one by hand.
CHARACTER_HEADING_RE = re.compile(r"personnages", re.IGNORECASE)
CHARACTER_LIST_ITEM_RE = re.compile(r"^(\S+)\s+(.+)$")


def ordered_sheet_ids(md_dir: Path) -> list[dict[str, Any]]:
    """Return sheet metadata (id, category, subgroup) for every .md
    file in `md_dir`, in filesystem (display) order."""
    metas = []
    for path in sorted(md_dir.glob("*.md")):
        metas.append(parse_sheet_filename(path))
    return metas


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def extract_characters(content: list[dict[str, Any]]) -> dict[str, str]:
    """Find the emoji -> name pairs introduced by a "Les personnages"
    section in one sheet's content blocks.

    Looks for a heading matching CHARACTER_HEADING_RE immediately
    followed by a list block, and parses each item as "EMOJI Name,
    optional trailing description" (e.g. "Babka Zuzana, leur
    grand-mère" -> "Babka Zuzana"). A sheet with no such section
    contributes nothing -- most sheets don't have one.

    Args:
        content: A sheet's `content` block list (see smd2data.py).

    Returns:
        {emoji: name}, name taken as written (before any comma),
        for every character introduced on this sheet.
    """
    characters: dict[str, str] = {}
    for index, block in enumerate(content):
        if block["type"] != "heading" or not CHARACTER_HEADING_RE.search(block["text"]):
            continue
        if index + 1 >= len(content) or content[index + 1]["type"] != "list":
            continue
        for item in content[index + 1]["items"]:
            match = CHARACTER_LIST_ITEM_RE.match(item.strip())
            if not match:
                continue
            emoji, rest = match.groups()
            name = rest.split(",")[0].strip()
            if name:
                characters[emoji] = name
    return characters


def build_characters_map(metas: list[dict[str, Any]], json_dir: Path) -> dict[str, str]:
    """The corpus-wide emoji -> name map, one entry per distinct
    speaker emoji, built by majority vote across every sheet that
    introduces that character -- a stray typo or an inconsistent
    description on one sheet doesn't change the name used everywhere
    else (see extract_characters()).
    """
    votes: dict[str, Counter] = {}
    for meta in metas:
        content_path = json_dir / f"{meta['id']}.content.json"
        if not content_path.exists():
            continue
        sheet = load_json(content_path)
        for emoji, name in extract_characters(sheet["content"]).items():
            votes.setdefault(emoji, Counter())[name] += 1
    return {emoji: counter.most_common(1)[0][0] for emoji, counter in votes.items()}


def build_data_json(metas: list[dict[str, Any]], json_dir: Path) -> dict[str, Any]:
    """Assemble dist/data.json: sheets grouped by (category, subgroup),
    in first-seen order, each group holding its sheets in filesystem
    order. `subgroup` can be None -- sheets without one still share a
    single group per category (see smd2data.py's optional-subgroup
    filename form), which the front-end renders as a flat list with
    no subgroup heading. Also includes `characters`, the corpus-wide
    emoji -> name map (see build_characters_map()), so the Settings
    screen can list dialogue characters by name without re-scanning
    the whole corpus in the browser.
    """
    groups: "OrderedDict[tuple[str, str | None], dict[str, Any]]" = OrderedDict()

    for meta in metas:
        content_path = json_dir / f"{meta['id']}.content.json"
        if not content_path.exists():
            raise FileNotFoundError(
                f"Missing {content_path} -- run smd2data.py for this sheet first."
            )
        sheet = load_json(content_path)

        key = (sheet["category"], sheet["subgroup"])
        if key not in groups:
            groups[key] = {
                "category": sheet["category"],
                "subgroup": sheet["subgroup"],
                "subgroup_label": sheet.get("subgroup_label", ""),
                "sheets": [],
            }
        groups[key]["sheets"].append({
            "id": sheet["id"],
            "title": sheet["title"],
            "image": sheet["image"],
            "content": sheet["content"],
        })

    return {"groups": list(groups.values()), "characters": build_characters_map(metas, json_dir)}


def build_exercises_json(metas: list[dict[str, Any]], json_dir: Path) -> dict[str, Any]:
    """Assemble dist/exercises.json: one entry per sheet id, keyed
    for direct lookup (the exercise-selection screen needs to pick
    individual sheets or whole subgroups, not just iterate a list).
    Introduction sheets have no .exercises.json by design and are
    silently skipped, not an error.
    """
    sheets: dict[str, Any] = {}

    for meta in metas:
        exercises_path = json_dir / f"{meta['id']}.exercises.json"
        if not exercises_path.exists():
            continue  # e.g. introduction sheets -- no exercises generated

        content_path = json_dir / f"{meta['id']}.content.json"
        title = load_json(content_path)["title"] if content_path.exists() else meta["id"]

        payload = load_json(exercises_path)
        exercises = payload["exercises"]

        sheets[meta["id"]] = {
            "category": meta["category"],
            "subgroup": meta["subgroup"],
            "title": title,
            "counts": dict(Counter(ex["type"] for ex in exercises)),
            "exercises": exercises,
        }

    return {"sheets": sheets}


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Assemble per-sheet JSON into dist/data.json and dist/exercises.json."
    )
    parser.add_argument("--md", type=Path, required=True, help="Directory of .md sheets (for ordering)")
    parser.add_argument("--json", type=Path, required=True, help="Directory of .content.json/.exercises.json files")
    parser.add_argument("-o", "--output-dir", type=Path, required=True, help="Directory to write data.json/exercises.json into")
    args = parser.parse_args()

    try:
        metas = ordered_sheet_ids(args.md)
        data = build_data_json(metas, args.json)
        exercises = build_exercises_json(metas, args.json)
    except (SheetNameError, FileNotFoundError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        sys.exit(1)

    args.output_dir.mkdir(parents=True, exist_ok=True)
    (args.output_dir / "data.json").write_text(
        json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8"
    )
    (args.output_dir / "exercises.json").write_text(
        json.dumps(exercises, ensure_ascii=False, indent=2), encoding="utf-8"
    )

    n_groups = len(data["groups"])
    n_sheets = sum(len(g["sheets"]) for g in data["groups"])
    n_exercise_sheets = len(exercises["sheets"])
    n_exercises = sum(len(s["exercises"]) for s in exercises["sheets"].values())
    print(
        f"Wrote data.json: {n_groups} group(s), {n_sheets} sheet(s).\n"
        f"Wrote exercises.json: {n_exercise_sheets} sheet(s), {n_exercises} exercise(s)."
    )


if __name__ == "__main__":
    main()
