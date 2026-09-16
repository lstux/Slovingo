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
import sys
from collections import Counter, OrderedDict
from pathlib import Path
from typing import Any

from smd2data import SheetNameError, parse_sheet_filename


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


def build_data_json(metas: list[dict[str, Any]], json_dir: Path) -> dict[str, Any]:
    """Assemble dist/data.json: sheets grouped by (category, subgroup),
    in first-seen order, each group holding its sheets in filesystem
    order. `subgroup` can be None -- sheets without one still share a
    single group per category (see smd2data.py's optional-subgroup
    filename form), which the front-end renders as a flat list with
    no subgroup heading.
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

    return {"groups": list(groups.values())}


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
