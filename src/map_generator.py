#!/usr/bin/env python3
"""map_generator.py

Generate map.json, the data behind the adventure-map homepage.

The map shows one step per *unit of the course*, not per sheet:

    intro      -- the whole "introduction" category (one step)
    survival   -- the series numbered 00 (Kit de survie)
    series     -- every other series (01, 02, ...)
    final      -- the castle (placeholder for the future mega-exam)

Steps are derived from the .md filenames only, following the v2 naming
convention (see smd2data.parse_sheet_filename):

    XX_Series_NN_Subgroup_XX_Title.md
                 ^^ the series number (the second order prefix)

parse_sheet_filename() discards the order prefixes (sheet ids must
survive renumbering), so the series number is re-read from the filename
here. A step's id is built from the subgroup name, not the number, so it
stays stable if series are renumbered.

Coordinates are presentation data, in percent of the map (0-100).
They are generated along a zigzag path (top to bottom), and can be overridden per step
in lang.json:

    "map": {
        "positions": {"series-rodina": [31, 70]},
        "icons":     {"rodina": "🏠"}
    }

Usage:
    python3 map_generator.py --md md_dir --lang lang.json -o dist/map.json
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
from typing import Any

from smd2data import parse_sheet_filename

VALIDATION_THRESHOLD = 85
"""Average score (percent) at which a series counts as validated."""

CASTLE_POSITION = (50.0, 88.0)
"""Where a lone castle goes; also the bottom end of the path."""

DEFAULT_ICONS = {
    "intro": "👋",
    "survival": "🧰",
    "final": "🏰",
    "fallback": "📚",
    # Known subgroup slugs (adult and kids courses); anything else
    # falls back to "fallback" unless lang.json -> map.icons says so.
    "kitsurvie": "🧰",
    "rodina": "👨‍👩‍👧",
    "doma": "🏠",
    "jedlo": "🍲",
    "cas": "🕐",
    "nakupy": "🛒",
    "mesto": "🏙️",
    "dedina": "🏘️",
    "pocasie": "🌦️",
    "tatry": "🏔️",
    "velkanoc": "🥚",
    "zvierata": "🦌",
    "hry": "🎲",
}

# Categories that make up the path itself. Every other category present
# in the course (dialog, vocabulary, annex...) is a free-access resource.
PATH_CATEGORIES = {"introduction", "series"}


def series_number(path: Path) -> int | None:
    """The series number (second order prefix) of a series sheet.

    Args:
        path: A sheet filename, XX_Series_NN_Subgroup_XX_Title.md form.

    Returns:
        The number, or None if the filename has no such segment.
    """
    parts = path.stem.split("_")
    if len(parts) >= 6 and parts[2].isdigit():
        return int(parts[2])
    return None


def generate_positions(count: int) -> tuple[list[tuple[float, float]], int]:
    """Positions (x, y in percent) for `count` steps along a path that
    runs from the top of the map (start) down to the castle at the
    bottom (always the last position, centred).

    The path zigzags left and right: two consecutive steps are always
    far apart horizontally, so their labels never collide even when the
    map is only ~400 px tall. Deterministic, so a rebuild never moves
    the steps around.

    For courses with many series, the vertical span is enlarged to
    maintain clear spacing between steps.
    """
    if count <= 0:
        return [], 1000
    if count == 1:
        return [CASTLE_POSITION], 1000

    # Scale the vertical span based on number of steps: ensure ~10 pixels
    # per step in the coordinate system (viewBox 0-1000).
    # Minimum span: 67 (original 9..76). For count steps, need at least count*10.
    # For N steps, need spacing of ~10 pixels to keep labels clear.
    # Original design: top=9, last=76 (67 pixels), fits ~9 steps.
    # For more steps, extend downward to maintain spacing.
    top = 0.0
    min_span = (count - 1) * 10.0  # Total vertical span needed
    last = max(76.0, min_span)  # Expand if necessary, but stay >= 76

    # Compute viewBox height: castle will be 12 units below last step
    viewBox_height = int((last + 20) * 10) if last > 76 else 1000

    positions: list[tuple[float, float]] = []
    for i in range(count - 1):
        y = top + (last - top) * i / max(count - 2, 1)
        side = -1 if i % 2 == 0 else 1
        x = 50 + side * (24 + 5 * math.sin(i * 2.1))
        positions.append((round(x, 1), round(y, 1)))
    positions.append(CASTLE_POSITION)
    return positions, viewBox_height


def build_map_json(md_dir: Path, lang_cfg: dict[str, Any]) -> dict[str, Any]:
    """Build the map.json structure from the .md filenames.

    Args:
        md_dir: Directory of the course's .md sheets.
        lang_cfg: Parsed lang.json (subgroup labels, optional `map`
            overrides).

    Returns:
        {"steps": [...], "resources": [...], "config": {...}}. Each
        step has id, type, title, icon, x, y, sheets (ids, in order)
        and href (the first sheet), except the castle (no sheets).
    """
    map_cfg = lang_cfg.get("map", {})
    icons = {**DEFAULT_ICONS, **map_cfg.get("icons", {})}
    subgroup_labels = lang_cfg.get("subgroups", {})
    category_labels = lang_cfg.get("categories", {})

    intro_sheets: list[str] = []
    series_units: dict[tuple[int, str], dict[str, Any]] = {}
    other_categories: list[str] = []

    for path in sorted(md_dir.glob("*.md")):
        meta = parse_sheet_filename(path)
        category = meta["category"]

        if category == "introduction":
            intro_sheets.append(meta["id"])
        elif category == "series" and meta["subgroup"]:
            number = series_number(path)
            if number is None:
                continue
            key = (number, meta["subgroup"])
            unit = series_units.setdefault(
                key,
                {
                    "number": number,
                    "subgroup": meta["subgroup"],
                    "label": meta["subgroup_label"],
                    "sheets": [],
                },
            )
            unit["sheets"].append(meta["id"])
        elif category not in PATH_CATEGORIES and category not in other_categories:
            other_categories.append(category)

    steps: list[dict[str, Any]] = []

    if intro_sheets:
        steps.append(
            {
                "id": "intro",
                "type": "intro",
                "title": category_labels.get("introduction", "Introduction"),
                "icon": icons["intro"],
                "sheets": intro_sheets,
            }
        )

    for (number, subgroup), unit in sorted(series_units.items()):
        step_type = "survival" if number == 0 else "series"
        steps.append(
            {
                "id": f"series-{subgroup}",
                "type": step_type,
                "number": number,
                "title": subgroup_labels.get(subgroup, unit["label"]),
                "icon": icons.get(subgroup, icons[step_type] if step_type in icons else icons["fallback"]),
                "sheets": unit["sheets"],
            }
        )

    # The castle is the last step. It has no sheets: the mega-exam is
    # specified separately and is not implemented yet.
    steps.append(
        {
            "id": "final-exam",
            "type": "final",
            "title": "Château",
            "icon": icons["final"],
            "sheets": [],
        }
    )

    positions, viewBox_height = generate_positions(len(steps))
    overrides = map_cfg.get("positions", {})
    for step, (x, y) in zip(steps, positions):
        step["x"], step["y"] = overrides.get(step["id"], [x, y])
        step["href"] = f"#/sheet/{step['sheets'][0]}" if step["sheets"] else None

    resources = [
        {"id": category, "title": category_labels.get(category, category.capitalize())}
        for category in other_categories
    ]

    return {
        "steps": steps,
        "resources": resources,
        "config": {
            "validationThreshold": VALIDATION_THRESHOLD,
            "foxAnimationDuration": 1200,
            "discoveryAnimationDuration": 1500,
            "viewBoxHeight": viewBox_height,
        },
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate map.json for the adventure map.")
    parser.add_argument("--md", type=Path, required=True, help="Directory of .md source files")
    parser.add_argument("--lang", type=Path, required=True, help="Path to lang.json")
    parser.add_argument("-o", "--output", type=Path, required=True, help="Output path for map.json")
    args = parser.parse_args()

    with args.lang.open(encoding="utf-8") as f:
        lang_cfg = json.load(f)

    map_data = build_map_json(args.md, lang_cfg)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(map_data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Map: {args.output}")


if __name__ == "__main__":
    main()
