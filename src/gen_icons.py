#!/usr/bin/env python3
"""gen_icons.py

Generate the three PWA icon files referenced by manifest.json:
icon-192.png, icon-512.png (purpose "any"), and icon-maskable-512.png
(purpose "maskable", extra padding so the flag survives being cropped
to a circle/squircle by the OS).

Design: a sunset gradient background (a placeholder aesthetic, easy to
replace later with real design work) with the target language's flag
emoji centered on top -- rendered through a color emoji font, so this
works for any target language without hardcoding flag colors per
language (see LANG.target_lang.flag in lang.json).

Usage:
    python3 gen_icons.py --lang lang.json -o icons/
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

# Common locations for a color emoji font (Noto Color Emoji ships on
# most Linux distros / CI images). If none is found, the script fails
# with a clear message rather than silently drawing a blank icon.
EMOJI_FONT_CANDIDATES = [
    "/usr/share/fonts/truetype/noto/NotoColorEmoji.ttf",
    "/usr/share/fonts/noto/NotoColorEmoji.ttf",
    "/usr/share/fonts/noto-emoji/NotoColorEmoji.ttf",
    "/System/Library/Fonts/Apple Color Emoji.ttc",
    "C:\\Windows\\Fonts\\seguiemj.ttf",
]

# Sunset gradient stops (top -> bottom), RGB.
GRADIENT_STOPS = [
    (255, 178, 89),    # warm orange (sky top)
    (255, 94, 110),     # coral / pink (horizon)
    (86, 39, 107),       # deep purple (sky bottom)
]


def find_emoji_font() -> str:
    for candidate in EMOJI_FONT_CANDIDATES:
        if Path(candidate).exists():
            return candidate
    raise FileNotFoundError(
        "No color emoji font found (tried: "
        + ", ".join(EMOJI_FONT_CANDIDATES)
        + "). Install one (e.g. fonts-noto-color-emoji) or point "
        "EMOJI_FONT_CANDIDATES at yours."
    )


def sunset_gradient(size: int) -> Image.Image:
    """A smooth vertical gradient through GRADIENT_STOPS."""
    img = Image.new("RGB", (1, size))
    stops = GRADIENT_STOPS
    segments = len(stops) - 1
    for y in range(size):
        position = y / max(size - 1, 1) * segments
        segment = min(int(position), segments - 1)
        local_t = position - segment
        r0, g0, b0 = stops[segment]
        r1, g1, b1 = stops[segment + 1]
        pixel = (
            round(r0 + (r1 - r0) * local_t),
            round(g0 + (g1 - g0) * local_t),
            round(b0 + (b1 - b0) * local_t),
        )
        img.putpixel((0, y), pixel)
    return img.resize((size, size))


def draw_flag_icon(size: int, flag_emoji: str, font_path: str, safe_zone_ratio: float) -> Image.Image:
    """Sunset background + centered flag emoji.

    Args:
        size: Output width/height in pixels (square).
        flag_emoji: The flag to draw (e.g. "🇸🇰").
        font_path: Path to a color emoji font.
        safe_zone_ratio: Fraction of the canvas the flag should occupy
            -- smaller for maskable icons, so the flag survives being
            cropped to a circle/squircle by the OS.
    """
    canvas = sunset_gradient(size).convert("RGBA")

    # NotoColorEmoji is a bitmap font with a small number of fixed
    # "strike" sizes -- 109px is the one available in the common Noto
    # Color Emoji build. Render at that fixed size, then resize the
    # cropped glyph down/up for a crisp result at any target size.
    render_size = 109
    font = ImageFont.truetype(font_path, render_size)
    glyph_layer = Image.new("RGBA", (200, 200), (0, 0, 0, 0))
    glyph_draw = ImageDraw.Draw(glyph_layer)
    glyph_draw.text((10, 10), flag_emoji, font=font, embedded_color=True)
    glyph_bbox = glyph_layer.getbbox()
    if glyph_bbox:
        glyph_layer = glyph_layer.crop(glyph_bbox)

    target_width = round(size * safe_zone_ratio)
    scale = target_width / glyph_layer.width
    target_height = round(glyph_layer.height * scale)
    glyph_layer = glyph_layer.resize((target_width, target_height), Image.LANCZOS)

    position = ((size - target_width) // 2, (size - target_height) // 2)
    canvas.alpha_composite(glyph_layer, dest=position)
    return canvas


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--lang", type=Path, required=True, help="Path to lang.json")
    parser.add_argument("-o", "--output-dir", type=Path, required=True, help="Directory to write icons into")
    args = parser.parse_args()

    with args.lang.open(encoding="utf-8") as f:
        lang_cfg = json.load(f)
    flag = lang_cfg["target_lang"]["flag"]

    font_path = find_emoji_font()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    specs = [
        ("icon-192.png", 192, 0.72),
        ("icon-512.png", 512, 0.72),
        ("icon-maskable-512.png", 512, 0.55),  # smaller: maskable safe zone
    ]
    for filename, size, safe_zone_ratio in specs:
        icon = draw_flag_icon(size, flag, font_path, safe_zone_ratio)
        icon.convert("RGB").save(args.output_dir / filename, "PNG")
        print(f"Wrote {args.output_dir / filename} ({size}x{size})")


if __name__ == "__main__":
    main()
