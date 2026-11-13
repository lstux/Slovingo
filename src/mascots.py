#!/usr/bin/env python3
"""mascots.py

Course-specific mascot pictures. The engine ships default faces for the
fox and the hare (static/mascots/<fox|rabbit>-<mood>.svg). A course can
replace any of them -- or add pictures the engine has no default for,
such as the duo walking on the map -- by dropping files into its
img/mascots/ folder:

    img/mascots/fox-content.png     fox, moods: content | bravo | think | comfort
    img/mascots/hare-bravo.png      the hare ("rabbit-..." is accepted too)
    img/mascots/duo-walk.png        the fox and the hare together, on the map

Every file found is listed in data.json under "mascot_pictures", key -> path:

    "mascot_pictures": {"fox-content": "mascots-course/fox-content.webp", ...}

and the front-end uses it, mood by mood, falling back to the default
SVG for whatever is missing. A course may therefore provide only some
of them.

With Pillow, each picture is shrunk to fit MAX_SIZE px and saved as
WebP (PNG when this Pillow has no WebP), alpha kept, in
<dist>/mascots-course/. Without Pillow, or for an SVG, the original is
used as-is from dist/img/ (build.py copies the whole img/ folder).
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

try:
    from PIL import Image
    PIL_AVAILABLE = True
except ImportError:  # Pillow is optional
    PIL_AVAILABLE = False

MASCOT_DIR = "mascots"          # inside the course's img/
OUT_DIR = "mascots-course"      # inside dist/
MAX_SIZE = 320
WEBP_QUALITY = 85

# "<who>-<what>.<ext>". "rabbit" is the engine's historical name for the hare.
MASCOT_FILE_RE = re.compile(r"^(fox|hare|rabbit|duo)-([a-z0-9-]+)\.(png|webp|jpe?g|svg)$", re.IGNORECASE)


def mascot_key(filename: str) -> str | None:
    """'Rabbit-Bravo.PNG' -> 'hare-bravo'; None if the name doesn't fit."""
    m = MASCOT_FILE_RE.match(filename)
    if not m:
        return None
    who, what = m.group(1).lower(), m.group(2).lower()
    if who == "rabbit":
        who = "hare"
    return f"{who}-{what}"


def _shrink(src: Path, dest: Path) -> Path:
    """Fit `src` in MAX_SIZE px, keep transparency. Returns the file written."""
    with Image.open(src) as im:
        im = im.convert("RGBA")
        im.thumbnail((MAX_SIZE, MAX_SIZE), Image.LANCZOS)
        try:
            im.save(dest, "WEBP", quality=WEBP_QUALITY, method=4)
            return dest
        except (OSError, KeyError, ValueError):
            dest.unlink(missing_ok=True)
            fallback = dest.with_suffix(".png")
            im.save(fallback, "PNG", optimize=True)
            return fallback


def add_mascots(data: dict, img_dir: Path, dist_dir: Path) -> int:
    """Fill data["mascot_pictures"] from <img_dir>/mascots/. Returns the count.

    The key is left out of data.json when the course has no mascot
    picture, so nothing changes for it.
    """
    src_dir = img_dir / MASCOT_DIR
    if not src_dir.is_dir():
        return 0

    found: dict[str, Path] = {}
    for path in sorted(src_dir.iterdir()):
        key = mascot_key(path.name) if path.is_file() else None
        if key is None:
            if path.is_file():
                print(f"  mascots: {path.name} ignored (expected fox-<mood>, hare-<mood> or duo-<name>)",
                      file=sys.stderr)
            continue
        # Two files for one key (hare-bravo.png and rabbit-bravo.png): the
        # "hare" one, sorted first, wins.
        found.setdefault(key, path)

    mascots: dict[str, str] = {}
    out_dir = dist_dir / OUT_DIR
    for key, path in found.items():
        original = f"img/{MASCOT_DIR}/{path.name}"
        if not PIL_AVAILABLE or path.suffix.lower() == ".svg":
            mascots[key] = original
            continue
        out_dir.mkdir(parents=True, exist_ok=True)
        dest = out_dir / f"{key}.webp"
        existing = next((p for p in (dest, dest.with_suffix(".png")) if p.exists()), None)
        if existing and existing.stat().st_mtime >= path.stat().st_mtime:
            mascots[key] = f"{OUT_DIR}/{existing.name}"
            continue
        try:
            written = _shrink(path, dest)
            mascots[key] = f"{OUT_DIR}/{written.name}"
        except OSError as exc:
            print(f"  mascots: cannot read {path.name} ({exc}), original used", file=sys.stderr)
            mascots[key] = original

    if mascots:
        data["mascot_pictures"] = mascots
    return len(mascots)


if __name__ == "__main__":  # pragma: no cover - manual check
    if len(sys.argv) != 3:
        sys.exit("usage: mascots.py <course img dir> <dist dir>")
    d: dict = {}
    print(add_mascots(d, Path(sys.argv[1]), Path(sys.argv[2])), d.get("mascot_pictures"))
