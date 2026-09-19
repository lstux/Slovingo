#!/usr/bin/env python3
"""gen_manifest.py

Generate manifest.json (PWA manifest) from lang.json. Fields that
depend on the course (name, description, deployment path) come from
the config; everything else (icon list, orientation, display mode) is
a fixed app convention, independent of the language being learned.

Usage:
    python3 gen_manifest.py --lang lang.json -o dist/manifest.json
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any


def build_manifest(lang_cfg: dict[str, Any]) -> dict[str, Any]:
    site = lang_cfg["site"]
    return {
        "name": site["title"],
        "short_name": site.get("short_name", site["title"]),
        "description": site.get("description", ""),
        "start_url": site["url_path"] + "index.html",
        "scope": site["url_path"],
        "display": "standalone",
        "orientation": "portrait-primary",
        "background_color": site.get("theme_color", "#0b4ea2"),
        "theme_color": site.get("theme_color", "#0b4ea2"),
        "lang": lang_cfg["native_lang"]["code"],
        "icons": [
            {"src": "icons/icon-192.png", "sizes": "192x192", "type": "image/png", "purpose": "any"},
            {"src": "icons/icon-512.png", "sizes": "512x512", "type": "image/png", "purpose": "any"},
            {"src": "icons/icon-maskable-512.png", "sizes": "512x512", "type": "image/png", "purpose": "maskable"},
        ],
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--lang", type=Path, required=True, help="Path to lang.json")
    parser.add_argument("-o", "--output", type=Path, required=True, help="Output manifest.json path")
    args = parser.parse_args()

    with args.lang.open(encoding="utf-8") as f:
        lang_cfg = json.load(f)

    manifest = build_manifest(lang_cfg)
    args.output.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {args.output}")


if __name__ == "__main__":
    main()
