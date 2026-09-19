#!/usr/bin/env python3
"""build.py

Convenience wrapper chaining the full local pipeline for one language:
content generation (smd2data), exercise generation (smd2exercises),
assembly (assemble), static file copy, and PWA asset generation
(manifest, service worker, icons) -- everything except deploying,
which is publish.py's job.

Convention over configuration: a language lives in one self-contained
directory (a "lang dir"), e.g. langs/sk-fr/, laid out as:

    langs/sk-fr/
        md/            source .md sheets (edited by hand)
        json/          intermediates: <id>.content.json (always
                        regenerated), <id>.exercises.json (generated
                        once, persists)
        img/           images referenced by sheets (optional)
        lang.json       this language's config
        dist/           generated -- the servable site

Everything under a lang dir except md/, img/, and lang.json is
generated and safe to delete; md/, img/, and lang.json are the only
things you ever hand-edit (plus lang.json-driven exceptions to
otherwise-generated exercises, kept in json/*.exercises.json).

Usage:
    # One language:
    python3 build.py --lang-dir langs/sk-fr --static-dir static

    # Every language dir under langs/:
    python3 build.py --langs-root langs --static-dir static
"""

from __future__ import annotations

import argparse
import json
import shutil
import sys
from pathlib import Path

import assemble
import smd2exercises
from gen_icons import draw_flag_icon, find_emoji_font
from gen_manifest import build_manifest
from gen_service_worker import render as render_service_worker
from smd2data import SheetNameError, build_sheet_json

# Static front-end files copied as-is into dist/ -- shared across
# every language, not generated from content.
STATIC_FILES = [
    "index.html", "app.js", "exercises.js", "progress.js", "settings.js",
    "style.css", "exercises.css",
]

ICON_SPECS = [
    ("icon-192.png", 192, 0.72),
    ("icon-512.png", 512, 0.72),
    ("icon-maskable-512.png", 512, 0.55),
]


def discover_lang_dirs(langs_root: Path) -> list[Path]:
    """Every immediate subdirectory of `langs_root` that has a
    lang.json -- i.e. looks like a language dir, not clutter."""
    return sorted(
        child for child in langs_root.iterdir()
        if child.is_dir() and (child / "lang.json").exists()
    )


def run_build(
    lang_dir: Path,
    static_dir: Path,
    force_exercises: bool = False,
    force_icons: bool = False,
) -> None:
    """Run the full local pipeline for one language dir.

    Args:
        lang_dir: A language directory (see module docstring for
            layout) -- md/, json/, dist/, lang.json all derived from
            it by convention.
        static_dir: Directory of static front-end files to copy into
            dist/, shared across every language.
        force_exercises: Regenerate exercise files even if already present.
        force_icons: Regenerate icon files even if already present.
    """
    md_dir = lang_dir / "md"
    json_dir = lang_dir / "json"
    dist_dir = lang_dir / "dist"
    img_dir = lang_dir / "img"
    lang_path = lang_dir / "lang.json"

    with lang_path.open(encoding="utf-8") as f:
        lang_cfg = json.load(f)

    print(f"=== {lang_dir.name} ===")

    json_dir.mkdir(parents=True, exist_ok=True)
    md_files = sorted(md_dir.glob("*.md"))

    # 1. Content: always regenerated from the .md source -- the .md is
    #    the only editable source of truth for a sheet's content.
    for path in md_files:
        sheet = build_sheet_json(path, lang_cfg)
        out = json_dir / f"{sheet['id']}.content.json"
        out.write_text(json.dumps(sheet, ensure_ascii=False, indent=2), encoding="utf-8")
    print(f"Content: {len(md_files)} sheet(s) written to {json_dir}")

    # 2. Exercises: persistent -- only sheets without an existing
    #    .exercises.json are generated, unless force_exercises.
    records = smd2exercises.load_corpus(md_dir, lang_cfg)
    all_exercises = smd2exercises.build_exercises_for_corpus(records)
    written, skipped = 0, 0
    for sheet_id, payload in all_exercises.items():
        out = json_dir / f"{sheet_id}.exercises.json"
        if out.exists() and not force_exercises:
            skipped += 1
            continue
        out.write_text(json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8")
        written += 1
    print(f"Exercises: {written} written, {skipped} kept as-is")

    # 3. Assembly.
    metas = assemble.ordered_sheet_ids(md_dir)
    data = assemble.build_data_json(metas, json_dir)
    exercises = assemble.build_exercises_json(metas, json_dir)
    dist_dir.mkdir(parents=True, exist_ok=True)
    (dist_dir / "data.json").write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8")
    (dist_dir / "exercises.json").write_text(json.dumps(exercises, ensure_ascii=False, indent=2), encoding="utf-8")
    print(f"Assembled: {dist_dir}/data.json, {dist_dir}/exercises.json")

    # 4. Static front-end files, copied as-is. lang.json is written
    #    from the in-memory config minus `generator` -- build-only
    #    header-detection lists the browser never needs.
    for filename in STATIC_FILES:
        shutil.copyfile(static_dir / filename, dist_dir / filename)
    runtime_lang_cfg = {k: v for k, v in lang_cfg.items() if k != "generator"}
    (dist_dir / "lang.json").write_text(
        json.dumps(runtime_lang_cfg, ensure_ascii=False, indent=2), encoding="utf-8"
    )
    print(f"Static files: {len(STATIC_FILES)} copied to {dist_dir}, lang.json written without `generator`")

    # Images: copied wholesale if the language has any. Existing files
    # are overwritten with the current source; a file removed from
    # img/ is NOT removed from dist/img/ (same as the static files
    # above) -- publish.py's rsync --delete handles that on deploy.
    if img_dir.exists():
        shutil.copytree(img_dir, dist_dir / "img", dirs_exist_ok=True)
        n_images = sum(1 for p in img_dir.rglob("*") if p.is_file())
        print(f"Images: {n_images} file(s) copied to {dist_dir / 'img'}")

    # 5. PWA: manifest.json and service-worker.js are pure config
    #    transforms of lang.json, always regenerated. Icons are
    #    persistent like exercises -- once you've swapped in real
    #    artwork, a rebuild must not silently overwrite it.
    manifest = build_manifest(lang_cfg)
    (dist_dir / "manifest.json").write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    (dist_dir / "service-worker.js").write_text(render_service_worker(lang_cfg), encoding="utf-8")
    print(f"PWA: manifest.json and service-worker.js written to {dist_dir}")

    icons_dir = dist_dir / "icons"
    icons_dir.mkdir(parents=True, exist_ok=True)
    icon_written, icon_skipped = 0, 0
    font_path = None
    for filename, size, safe_zone_ratio in ICON_SPECS:
        out = icons_dir / filename
        if out.exists() and not force_icons:
            icon_skipped += 1
            continue
        if font_path is None:
            font_path = find_emoji_font()
        icon = draw_flag_icon(size, lang_cfg["target_lang"]["flag"], font_path, safe_zone_ratio)
        icon.convert("RGB").save(out, "PNG")
        icon_written += 1
    print(f"Icons: {icon_written} written, {icon_skipped} kept as-is")


def main() -> None:
    parser = argparse.ArgumentParser(description="Run the full content+exercises+assembly+PWA pipeline.")
    target = parser.add_mutually_exclusive_group(required=True)
    target.add_argument("--lang-dir", type=Path, help="One language directory (e.g. langs/sk-fr)")
    target.add_argument("--langs-root", type=Path, help="Build every language dir under this directory (e.g. langs)")
    parser.add_argument("--static-dir", type=Path, required=True, help="Directory of static front-end files to copy into dist/")
    parser.add_argument(
        "--force-exercises", action="store_true",
        help="Regenerate exercise files even if already present (default: keep existing, hand-edited or not)",
    )
    parser.add_argument(
        "--force-icons", action="store_true",
        help="Regenerate icon files even if already present (default: keep existing -- e.g. real artwork you swapped in)",
    )
    args = parser.parse_args()

    lang_dirs = discover_lang_dirs(args.langs_root) if args.langs_root else [args.lang_dir]
    if not lang_dirs:
        print(f"Error: no language directory (with a lang.json) found under {args.langs_root}", file=sys.stderr)
        sys.exit(1)

    try:
        for lang_dir in lang_dirs:
            run_build(
                lang_dir, args.static_dir,
                force_exercises=args.force_exercises, force_icons=args.force_icons,
            )
            print()
    except (SheetNameError, ValueError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
