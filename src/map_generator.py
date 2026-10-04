#!/usr/bin/env python3
"""map_generator.py

Generate map metadata for the adventure map homepage.

Detects series from .md filenames (Serie_NN_...) and generates step data
(intro, survival kit, series, castle) with coordinates for positioning on
the adventure map.

The generated map.json is injected into the browser as window.SLOVINGO_MAP
and consumed by map.js to render the interactive carte.

Usage:
    python3 map_generator.py --md md_dir --lang lang.json -o dist/map.json
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any

from smd2data import parse_sheet_filename


def generate_series_coordinates(
    num_series: int, mobile: bool = False
) -> list[tuple[float, float]]:
    """Generate (x, y) coordinates for series positioned along a sinuous path.

    The path starts at the bottom (intro/survival) and winds upward toward
    the castle at the top. Coordinates are in percentages (0-100).

    Args:
        num_series: Number of series to position.
        mobile: If True, use a more vertical layout.

    Returns:
        List of (x, y) tuples, one per series.
    """
    coords = []

    if mobile:
        # Mobile: narrow vertical path
        for i in range(num_series):
            progress = i / max(1, num_series - 1)
            x = 50 + 10 * (1 if i % 2 == 0 else -1)  # Slight horizontal wiggle
            y = 70 - (progress * 50)  # 70% down to 20% up
            coords.append((x, y))
    else:
        # Desktop: wider sinuous path
        for i in range(num_series):
            progress = i / max(1, num_series - 1)
            # Sine wave wiggle left-right
            x_offset = 30 * (1 - 2 * abs(0.5 - (progress % 1)))
            x = 50 + x_offset
            y = 70 - (progress * 60)  # 70% down to 10% up
            coords.append((x, y))

    return coords


def build_map_json(
    md_dir: Path, lang_cfg: dict[str, Any], force: bool = False
) -> dict[str, Any]:
    """Build the map.json structure from .md files and lang.json.

    Args:
        md_dir: Directory containing .md source files.
        lang_cfg: Parsed lang.json configuration.
        force: If True, regenerate coordinates even if present.

    Returns:
        A dict with 'steps' (array of step objects) and 'resources'.
    """
    # Detect all .md files and parse their metadata
    md_files = sorted(md_dir.glob("*.md"))
    metas = [parse_sheet_filename(p) for p in md_files]

    # Separate by category
    intro_sheets = [m for m in metas if m.get("category") == "introduction"]
    survival_sheets = [m for m in metas if m.get("category") == "survival"]
    series_sheets = [m for m in metas if m.get("category") == "series"]

    # Hardcoded for sk-fr: intro and survival are special cases
    intro_visited = bool(intro_sheets)
    survival_visited = bool(survival_sheets)

    steps = []

    # 1. Introduction
    if intro_sheets:
        intro_sheet = intro_sheets[0]
        steps.append(
            {
                "id": "intro",
                "type": "intro",
                "title": "Introduction",
                "label": "",
                "href": f"#/sheet/{intro_sheet['id']}",
                "icon": "👋",
                "x": 12,
                "y": 88,
            }
        )

    # 2. Kit de survie (survival category)
    if survival_sheets:
        survival_sheet = survival_sheets[0]
        steps.append(
            {
                "id": "survival",
                "type": "survival",
                "title": "Kit de survie",
                "label": "",
                "href": f"#/sheet/{survival_sheet['id']}",
                "icon": "🧰",
                "x": 23,
                "y": 78,
            }
        )

    # 3. Series (the bulk of the content)
    series_coords = generate_series_coordinates(len(series_sheets))

    # Map of series number to emoji icon (from spec)
    SERIES_ICONS = {
        1: "👨‍👩‍👧",  # Rodina (famille) -- family
        2: "🏠",  # Doma (maison) -- home
        3: "🍲",  # Jedlo (nourriture) -- food
        4: "🕐",  # Cas (heure) -- time
        5: "🛒",  # Nakupy (achats) -- shopping
        6: "🏙️",  # Mesto (ville) -- city
        7: "🌦️",  # Pocasie (météo) -- weather
        8: "🏔️",  # Tatry (Tatras) -- mountains
        9: "🥚",  # VelkaNoc (Pâques) -- Easter
    }

    for meta, (x, y) in zip(series_sheets, series_coords):
        series_id = meta["id"]

        # Extract series number from ID or filename
        # E.g. "Serie_01_Rodina" -> 1
        match = re.search(r"Serie_(\d+)", series_id)
        series_num = int(match.group(1)) if match else 0

        icon = SERIES_ICONS.get(series_num, "📚")
        title = meta["subgroup"] or f"Serie {series_num}"

        steps.append(
            {
                "id": series_id,
                "type": "series",
                "number": series_num,
                "title": title,
                "label": "",
                "href": f"#/sheet/{series_id}",
                "icon": icon,
                "x": round(x, 1),
                "y": round(y, 1),
            }
        )

    # 4. Castle (final destination)
    steps.append(
        {
            "id": "final-exam",
            "type": "final",
            "title": "Château",
            "label": "Méga-examen",
            "href": None,
            "icon": "🏰",
            "x": 84,
            "y": 12,
        }
    )

    # 5. Direct resources (outside the main path)
    resources = [
        {
            "id": "vocabulaire",
            "title": "Vocabulaire",
            "icon": "📚",
            "href": "#/category/vocabulary",
        },
        {
            "id": "revisions",
            "title": "Révisions",
            "icon": "🔄",
            "href": "#/category/annex",  # or another category TBD
        },
        {
            "id": "annexes",
            "title": "Annexes",
            "icon": "📎",
            "href": "#/category/annex",
        },
    ]

    return {
        "steps": steps,
        "resources": resources,
        "config": {
            "validationThreshold": 85,
            "foxAnimationDuration": 1200,
            "discoveryAnimationDuration": 1500,
        },
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate map.json for the adventure map.")
    parser.add_argument("--md", type=Path, required=True, help="Directory of .md source files")
    parser.add_argument("--lang", type=Path, required=True, help="Path to lang.json")
    parser.add_argument("-o", "--output", type=Path, required=True, help="Output path for map.json")
    parser.add_argument("--force", action="store_true", help="Force regeneration")
    args = parser.parse_args()

    with args.lang.open(encoding="utf-8") as f:
        lang_cfg = json.load(f)

    map_data = build_map_json(args.md, lang_cfg, force=args.force)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(map_data, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )

    print(f"Map: {args.output}")


if __name__ == "__main__":
    main()
