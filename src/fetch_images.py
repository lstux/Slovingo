#!/usr/bin/env python3
"""fetch_images.py

Download external images referenced in .md sheets and update references to local paths.

For each .md file in a language's md/ directory, this script:
1. Finds lines matching @ <url> | <caption>
2. Downloads external URLs (http://, https://) to langs/<lang>/img/
3. Optionally resizes/recompresses with ImageMagick if available
4. Rewrites the @ line with the new local path (img/...)
5. Preserves the caption

Local paths (img/...) are left as-is.

Matching the --lang-dir / --langs-root pattern of build.py and publish.py.

Usage:
    # One language (dry-run, preview only):
    python3 fetch_images.py --lang-dir langs/sk-fr

    # One language (execute):
    python3 fetch_images.py --lang-dir langs/sk-fr --execute

    # Every language (dry-run):
    python3 fetch_images.py --langs-root langs

    # Every language (execute):
    python3 fetch_images.py --langs-root langs --execute

    # With resizing (requires ImageMagick 'convert'):
    python3 fetch_images.py --lang-dir langs/sk-fr --resize 800 --execute
"""

from __future__ import annotations

import argparse
import re
import shutil
import subprocess
import sys
from pathlib import Path
from urllib.parse import quote, unquote, urlparse, urlunparse
from urllib.request import Request, urlopen

# Regex for @ img_line | caption
IMAGE_RE = re.compile(r"^@\s+(\S+)\s*\|\s*(.+)$")

# Wikimedia (and several other hosts) reject the default urllib/requests
# User-Agent with a 403. Their policy asks for an identifying UA:
# https://meta.wikimedia.org/wiki/User-Agent_policy
USER_AGENT = "Slovingo-fetch-images/1.0 (https://lslinux.org; contact: ericlecat83@gmail.com)"


def discover_lang_dirs(langs_root: Path) -> list[Path]:
    """Every immediate subdirectory of `langs_root` that has a
    lang.json -- i.e. looks like a language dir, not clutter."""
    return sorted(
        child for child in langs_root.iterdir()
        if child.is_dir() and (child / "lang.json").exists()
    )


def extract_filename_from_url(url: str) -> str:
    """Extract filename from URL, cleaning query params.

    Percent-decoded so e.g. "H%C3%B4tel" becomes "Hôtel" on disk instead
    of the raw URL-encoded form (common with Wikimedia Special:FilePath
    links, which carry accented/punctuated filenames).
    """
    parsed = urlparse(url)
    filename = unquote(Path(parsed.path).name)
    if not filename:
        filename = "image.jpg"
    return filename


def encode_url_for_request(url: str) -> str:
    """Percent-encode any raw non-ASCII characters in the URL's path/query
    before it's sent over HTTP.

    Some sources (Wikimedia Special:FilePath links in particular) end up
    stored in the .md with literal accented/unicode characters instead of
    %-encoded ones (e.g. ".../Holíč_in_Slovakia...jpg" rather than
    ".../Hol%C3%ADč_in_Slovakia...jpg"). http.client encodes the request
    line as ASCII before writing it to the socket, so an un-encoded
    non-ASCII URL raises UnicodeEncodeError deep inside urlopen() rather
    than a clean HTTP error. Re-quoting here (idempotent on URLs that are
    already properly encoded) fixes that without touching the filename we
    derive separately for local storage.
    """
    parsed = urlparse(url)
    safe = "/:@!$&'()*+,;=%"  # keep existing % escapes and URL delimiters intact
    path = quote(parsed.path, safe=safe)
    query = quote(parsed.query, safe=safe + "?")
    return urlunparse(parsed._replace(path=path, query=query))


def has_imagemagick() -> bool:
    """Check if ImageMagick 'convert' is available."""
    return shutil.which("convert") is not None


def resize_image(input_path: Path, output_path: Path, max_width: int) -> bool:
    """Resize/recompress image with ImageMagick.

    Converts to JPEG at quality 85 if original isn't PNG/GIF/WebP.
    Returns True if successful, False otherwise.
    """
    if not has_imagemagick():
        return False

    suffix = output_path.suffix.lower()
    quality = "85" if suffix in [".jpg", ".jpeg"] else "90"

    cmd = [
        "convert",
        str(input_path),
        "-resize", f"{max_width}x{max_width}>",  # only if larger
        "-quality", quality,
        str(output_path),
    ]

    try:
        result = subprocess.run(cmd, capture_output=True, timeout=30)
        return result.returncode == 0
    except Exception as e:
        print(f"  ⚠️  Resize failed: {e}", file=sys.stderr)
        return False


def download_image(url: str, output_path: Path, resize_width: int | None = None) -> bool:
    """Download image from URL. Returns True if successful."""
    try:
        print(f"  ↓ {url} → {output_path.name}", end=" ")

        req = Request(encode_url_for_request(url), headers={"User-Agent": USER_AGENT})
        with urlopen(req, timeout=10) as response:
            data = response.read()

        output_path.write_bytes(data)
        size_kb = len(data) / 1024
        print(f"({size_kb:.1f} KB)", end="")

        if resize_width and has_imagemagick():
            print(f" → resizing...", end=" ")
            if resize_image(output_path, output_path, resize_width):
                new_size_kb = output_path.stat().st_size / 1024
                print(f"✓ ({new_size_kb:.1f} KB)")
            else:
                print("✗ (kept original)")
        else:
            print()

        return True
    except Exception as e:
        print(f"✗ ({e})")
        return False


def process_lang_dir(
    lang_dir: Path,
    execute: bool = False,
    resize_width: int | None = None,
) -> None:
    """Process all .md files in a language directory.

    Args:
        lang_dir: Path to the language directory (langs/sk-fr)
        execute: If False, dry-run only; if True, actually download and modify files
        resize_width: If set, resize images to this max width (requires ImageMagick)
    """
    md_dir = lang_dir / "md"
    img_dir = lang_dir / "img"

    if not md_dir.exists():
        print(f"  ✗ No md/ directory found", file=sys.stderr)
        return

    # Create img dir if it doesn't exist (and execute=True)
    if execute and not img_dir.exists():
        img_dir.mkdir(parents=True, exist_ok=True)
        print(f"  Created {img_dir}")

    md_files = sorted(md_dir.glob("*.md"))
    if not md_files:
        print(f"  No .md files found")
        return

    print(f"\n=== {lang_dir.name} ===")

    if resize_width and has_imagemagick():
        print(f"Resize mode: max width {resize_width}px")
    elif resize_width:
        print(f"⚠️  Resize requested but ImageMagick not found — downloading as-is")

    if not execute:
        print("DRY RUN (use --execute to actually download and modify)\n")

    total_downloads = 0
    total_rewrites = 0

    for md_path in md_files:
        content = md_path.read_text(encoding="utf-8")
        lines = content.split("\n")
        modified = False

        for i, line in enumerate(lines):
            match = IMAGE_RE.match(line.strip())
            if not match:
                continue

            src, caption = match.groups()

            # Skip local paths (already img/)
            if src.startswith("img/"):
                continue

            # Process external URLs
            if src.startswith("http://") or src.startswith("https://"):
                print(f"{md_path.name}:")
                filename = extract_filename_from_url(src)
                local_path = img_dir / filename

                if execute:
                    # Download
                    if download_image(src, local_path, resize_width):
                        total_downloads += 1
                        # Rewrite the line
                        new_src = f"img/{filename}"
                        new_line = f"@ {new_src} | {caption}"
                        lines[i] = new_line
                        modified = True
                        total_rewrites += 1
                        print(f"  ✓ Updated: @ {new_src} | ...")
                    else:
                        print(f"  (skipped line update due to download failure)")
                else:
                    # Dry-run: just show what would happen
                    print(f"  [DRY] Would download: {src}")
                    print(f"  [DRY] Would save as: img/{filename}")
                    print(f"  [DRY] Would rewrite: @ img/{filename} | ...")
                    total_downloads += 1

        # Write back if modified
        if modified and execute:
            new_content = "\n".join(lines)
            md_path.write_text(new_content, encoding="utf-8")
            total_rewrites += 1
            print(f"  {md_path.name} updated")

    print(f"\nSummary: {total_downloads} download(s), {total_rewrites} file(s) updated")

    if not execute and total_downloads > 0:
        print("→ Re-run with --execute to actually download and modify")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Download external images and update .md references to local paths."
    )

    target = parser.add_mutually_exclusive_group(required=True)
    target.add_argument(
        "--lang-dir",
        type=Path,
        help="One language directory (e.g. langs/sk-fr)",
    )
    target.add_argument(
        "--langs-root",
        type=Path,
        help="Process every language dir under this directory (e.g. langs)",
    )

    parser.add_argument(
        "--execute",
        action="store_true",
        default=False,
        help="Execute downloads and modifications (default: dry-run only)",
    )

    parser.add_argument(
        "--resize",
        type=int,
        default=None,
        help="Max image width in pixels (requires ImageMagick, default: no resizing)",
    )

    args = parser.parse_args()

    lang_dirs = (
        discover_lang_dirs(args.langs_root)
        if args.langs_root
        else [args.lang_dir]
    )

    if not lang_dirs:
        print(
            f"Error: no language directory (with a lang.json) found under {args.langs_root}",
            file=sys.stderr,
        )
        sys.exit(1)

    try:
        for lang_dir in lang_dirs:
            process_lang_dir(
                lang_dir,
                execute=args.execute,
                resize_width=args.resize,
            )
            print()
    except KeyboardInterrupt:
        print("\nAborted.", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
