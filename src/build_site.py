#!/usr/bin/env python3
"""build_site.py

Assemble ONE site made of a launcher page plus several courses, to publish
on a single GitHub Pages origin:

    <out>/index.html, manifest.json, service-worker.js, courses.json, icons/
    <out>/<course>/...        (one full build per course)

Why: a PWA installed from the launcher has a scope that contains every
course (e.g. /Slovingo/ contains /Slovingo/sk-fr-kids/), so a single install
gives access to all of them. The launcher remembers the last course chosen
and sends the user straight back to it.

Each course is built exactly like a standalone one (src/build.py), with only
site.url_path rewritten to <base-path><course>/. storage_prefix is left
untouched, so progress saved by a course is the same wherever it is served
from on the same origin.

courses.json doubles as a tiny public API: the list of available courses with, for
each, its path and the URL of its own light API (<course>/api/index.json). Paths are
relative to courses.json, so any client (T-Watch, widget...) can follow them.

Inputs:
    --content-root DIR   a directory holding one checkout per course, named
                         after the course code (DIR/sk-fr-kids/, ...)
    --launcher-dir DIR   launcher/ (index.html, manifest.json, ...)
    --static-dir DIR     the engine's static/ directory
    --base-path PATH     where the site is served, e.g. /Slovingo/
    --courses CODE...    courses to include (default: launcher/course-ids.json)

Usage:
    python3 src/build_site.py --content-root content --launcher-dir launcher \\
        --static-dir static --base-path /Slovingo/ --out _site
"""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

from build import ICON_SPECS
from gen_icons import PIL_AVAILABLE, companions_from_config, draw_flag_icon, find_emoji_font

SRC_DIR = Path(__file__).resolve().parent
CODE_RE = re.compile(r"^[a-z0-9][a-z0-9-]*$")

# Files of a course checkout that are not part of the course content.
COPY_IGNORE = shutil.ignore_patterns(".git", ".github", "LICENSE", "README*", "dist")

LAUNCHER_FILES = ["index.html", "manifest.json", "service-worker.js"]

# The launcher icon: the fox, alone on the usual sunset background.
LAUNCHER_GLYPH = "🦊"


def normalize_base_path(path: str) -> str:
    return "/" + path.strip("/") + "/" if path.strip("/") else "/"


def read_course_ids(launcher_dir: Path, requested: list[str] | None) -> list[str]:
    if requested:
        ids = requested
    else:
        data = json.loads((launcher_dir / "course-ids.json").read_text(encoding="utf-8"))
        ids = data["courses"]
    for code in ids:
        if not CODE_RE.match(code):
            raise SystemExit(f"Invalid course code: {code!r}")
    if len(set(ids)) != len(ids):
        raise SystemExit("Duplicate course code in the list")
    return ids


def build_course(code: str, content_root: Path, static_dir: Path, base_path: str, work_root: Path, out: Path) -> dict:
    """Build one course under out/<code>/ and return its launcher entry."""
    src = content_root / code
    if not (src / "lang.json").is_file():
        raise SystemExit(f"{src}/lang.json not found: is the course checked out?")

    lang_dir = work_root / code
    shutil.copytree(src, lang_dir, ignore=COPY_IGNORE)

    lang_path = lang_dir / "lang.json"
    cfg = json.loads(lang_path.read_text(encoding="utf-8"))
    cfg["site"]["url_path"] = f"{base_path}{code}/"
    lang_path.write_text(json.dumps(cfg, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    subprocess.run(
        [sys.executable, str(SRC_DIR / "build.py"), "--lang-dir", str(lang_dir), "--static-dir", str(static_dir)],
        check=True,
    )

    # Optional light JSON API (older engines may not have the script).
    export_api = SRC_DIR / "export_api.py"
    if export_api.is_file():
        subprocess.run(
            [sys.executable, str(export_api), "--lang-dir", str(lang_dir), "-o", str(lang_dir / "dist" / "api")],
            check=True,
        )

    dest = out / code
    shutil.copytree(lang_dir / "dist", dest)

    site = cfg["site"]
    kids = site.get("icon_style") == "kids"
    entry = {
        "id": code,
        "path": f"{code}/",
        "title": site["title"],
        "short_name": site.get("short_name", site["title"]),
        "description": site.get("description", ""),
        "flag": cfg["target_lang"].get("flag", ""),
        "companions": companions_from_config(cfg) if kids else [],
        "theme_color": site.get("theme_color", ""),
    }
    if (dest / "api" / "index.json").is_file():
        entry["api"] = f"{code}/api/index.json"  # light JSON API of the course (export_api.py)
    return entry


def write_launcher(launcher_dir: Path, out: Path, entries: list[dict]) -> None:
    for name in LAUNCHER_FILES:
        shutil.copy2(launcher_dir / name, out / name)
    (out / "courses.json").write_text(
        json.dumps({"version": 1, "courses": entries}, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )

    icons_dir = out / "icons"
    icons_dir.mkdir(parents=True, exist_ok=True)
    if not PIL_AVAILABLE:
        raise SystemExit("Pillow is required to generate the launcher icons: pip install -r requirements.txt")
    font_path = find_emoji_font()
    for filename, size, safe_zone_ratio in ICON_SPECS:
        icon = draw_flag_icon(size, LAUNCHER_GLYPH, font_path, safe_zone_ratio, None)
        icon.convert("RGB").save(icons_dir / filename, "PNG")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--content-root", type=Path, required=True)
    parser.add_argument("--launcher-dir", type=Path, required=True)
    parser.add_argument("--static-dir", type=Path, required=True)
    parser.add_argument("--base-path", required=True, help="Where the site is served, e.g. /Slovingo/")
    parser.add_argument("--out", type=Path, required=True, help="Output directory (must not exist or be empty)")
    parser.add_argument("--courses", nargs="+", help="Course codes (default: launcher/course-ids.json)")
    args = parser.parse_args()

    base_path = normalize_base_path(args.base_path)
    ids = read_course_ids(args.launcher_dir, args.courses)

    if args.out.exists() and any(args.out.iterdir()):
        raise SystemExit(f"{args.out} is not empty")
    args.out.mkdir(parents=True, exist_ok=True)

    entries = []
    with tempfile.TemporaryDirectory() as tmp:
        for code in ids:
            print(f"== {code} -> {base_path}{code}/")
            entries.append(build_course(code, args.content_root, args.static_dir, base_path, Path(tmp), args.out))

    write_launcher(args.launcher_dir, args.out, entries)
    (args.out / ".nojekyll").touch()
    print(f"Site written to {args.out}: launcher + {len(entries)} course(s)")


if __name__ == "__main__":
    main()
