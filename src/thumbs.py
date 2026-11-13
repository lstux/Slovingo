#!/usr/bin/env python3
"""thumbs.py

Square thumbnails of the sheets' illustrations, for the "collection"
index pages (vocabulary, Kronika...): one card per sheet, with the
sheet's first image shown 1:1.

Optional by design: Pillow is only needed here. Without it (or when an
image can't be read) no thumbnail is made, `sheet["thumb"]` is simply
absent, and the front-end falls back to the full image, cropped square
in CSS -- heavier, but the page still works.

Only local images (`img/...` in the course) get a thumbnail. Remote
images (an http(s) URL) are left alone.

Output: <dist>/thumbs/<flattened-image-path>.webp (JPEG if this Pillow
has no WebP support). Source images are never modified. A thumbnail
newer than its source is kept as-is, so a rebuild is cheap.
"""

from __future__ import annotations

import sys
from pathlib import Path

try:
    from PIL import Image, ImageOps
    PIL_AVAILABLE = True
except ImportError:  # Pillow is optional
    PIL_AVAILABLE = False

THUMB_SIZE = 320
THUMB_DIR = "thumbs"
WEBP_QUALITY = 80
JPEG_QUALITY = 82


def _thumb_name(src: str, ext: str) -> str:
    """'img/vocabulary/01_pocasie.jpg' -> 'vocabulary__01_pocasie.webp'."""
    rel = src.split("img/", 1)[1] if src.startswith("img/") else src
    stem = rel.rsplit(".", 1)[0]
    return stem.replace("/", "__") + ext


def _save(im: "Image.Image", dest: Path) -> Path:
    """Save as WebP, or JPEG when WebP is unavailable. Returns the path written."""
    try:
        im.save(dest, "WEBP", quality=WEBP_QUALITY, method=4)
        return dest
    except (OSError, KeyError, ValueError):
        dest.unlink(missing_ok=True)
        fallback = dest.with_suffix(".jpg")
        im.save(fallback, "JPEG", quality=JPEG_QUALITY, optimize=True)
        return fallback


def make_square_thumb(src_path: Path, dest: Path, size: int = THUMB_SIZE) -> Path:
    """Centre-crop `src_path` to a square and shrink it to `size` px.

    Returns the file actually written (`dest`, or its .jpg sibling when
    WebP is not available). Raises OSError if the source is unreadable.
    """
    with Image.open(src_path) as raw:
        im = ImageOps.exif_transpose(raw).convert("RGB")
    side = min(im.width, im.height)
    left = (im.width - side) // 2
    top = (im.height - side) // 2
    im = im.crop((left, top, left + side, top + side))
    if side > size:
        im = im.resize((size, size), Image.LANCZOS)
    return _save(im, dest)


def add_thumbnails(data: dict, img_dir: Path, dist_dir: Path, size: int = THUMB_SIZE) -> int:
    """Make a thumbnail for each sheet's first image and set `sheet["thumb"]`.

    Args:
        data: the assembled data.json content (`data["groups"][*]["sheets"][*]`),
            modified in place.
        img_dir: the course's img/ source directory.
        dist_dir: the build output directory (thumbs/ is created inside).

    Returns:
        The number of sheets that got a thumbnail (0 without Pillow).
    """
    sheets = [s for g in data.get("groups", []) for s in g.get("sheets", [])]
    wanted = [s for s in sheets if isinstance(s.get("image"), dict)
              and str(s["image"].get("src", "")).startswith("img/")]
    if not wanted:
        return 0
    if not PIL_AVAILABLE:
        print("Thumbnails: Pillow not installed, skipped "
              "(collection pages will use the full images)", file=sys.stderr)
        return 0

    out_dir = dist_dir / THUMB_DIR
    made: dict[str, str | None] = {}   # image src -> thumb path relative to dist, or None
    for sheet in wanted:
        src = sheet["image"]["src"]
        if src not in made:
            made[src] = None
            src_path = img_dir / src[len("img/"):]
            # Vector images (logos, maps) scale in CSS by themselves.
            if src_path.suffix.lower() != ".svg" and src_path.is_file():
                out_dir.mkdir(parents=True, exist_ok=True)
                dest = out_dir / _thumb_name(src, ".webp")
                try:
                    existing = [p for p in (dest, dest.with_suffix(".jpg")) if p.is_file()
                                and p.stat().st_mtime >= src_path.stat().st_mtime]
                    written = existing[0] if existing else make_square_thumb(src_path, dest, size)
                    made[src] = f"{THUMB_DIR}/{written.name}"
                except OSError as exc:
                    print(f"Thumbnails: cannot read {src_path} ({exc})", file=sys.stderr)
        if made[src]:
            sheet["thumb"] = made[src]
    return sum(1 for s in wanted if s.get("thumb"))
