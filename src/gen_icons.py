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

Kids variant: with lang.json's site.icon_style set to "kids", the flag
is shrunk and moved up, and the course's companion emojis (by default
the fox and the rabbit, override with site.icon_companions) are drawn
side by side underneath it. Without it, the icon is the plain flag.

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


ICON_STYLES = ("flag", "kids")
DEFAULT_KIDS_COMPANIONS = ["\U0001F98A", "\U0001F430"]  # fox, rabbit

# Kids layout, as fractions of the safe zone (size * safe_zone_ratio),
# so the maskable variant (smaller safe zone) scales everything with it.
KIDS_FLAG_WIDTH = 0.70       # flag width
KIDS_COMPANION_WIDTH = 0.36  # max width of one companion
KIDS_COMPANION_GAP = 0.085   # horizontal gap between companions
KIDS_ROW_GAP = 0.08          # vertical gap between flag and companions


def companions_from_config(lang_cfg: dict) -> list[str]:
    """The companion emojis to draw under the flag, from lang.json.

    Empty (plain flag icon) unless site.icon_style is "kids". In kids
    mode, site.icon_companions overrides the default fox + rabbit.

    Raises:
        ValueError: unknown icon_style, or icon_companions that isn't a
            list of non-empty strings.
    """
    site = lang_cfg.get("site", {})
    style = site.get("icon_style", "flag")
    if style not in ICON_STYLES:
        raise ValueError(f'site.icon_style must be one of {ICON_STYLES}, got {style!r}')
    if style == "flag":
        return []
    companions = site.get("icon_companions", DEFAULT_KIDS_COMPANIONS)
    if not isinstance(companions, list) or not all(isinstance(c, str) and c for c in companions):
        raise ValueError("site.icon_companions must be a list of emoji strings")
    return companions


def render_glyph(text: str, font_path: str) -> Image.Image:
    """One emoji as a tightly cropped RGBA image.

    NotoColorEmoji is a bitmap font with a small number of fixed
    "strike" sizes -- 109px is the one available in the common Noto
    Color Emoji build. Render at that fixed size and let the caller
    resize the cropped glyph for a crisp result at any target size.
    """
    font = ImageFont.truetype(font_path, 109)
    layer = Image.new("RGBA", (200, 200), (0, 0, 0, 0))
    ImageDraw.Draw(layer).text((10, 10), text, font=font, embedded_color=True)
    bbox = layer.getbbox()
    return layer.crop(bbox) if bbox else layer


def resize_to_width(glyph: Image.Image, width: float) -> Image.Image:
    width = max(1, round(width))
    return glyph.resize((width, max(1, round(glyph.height * width / glyph.width))), Image.LANCZOS)


def draw_flag_icon(
    size: int,
    flag_emoji: str,
    font_path: str,
    safe_zone_ratio: float,
    companions: list[str] | None = None,
) -> Image.Image:
    """Sunset background + centered flag emoji, and optionally the
    companion emojis side by side underneath it (kids icon).

    Args:
        size: Output width/height in pixels (square).
        flag_emoji: The flag to draw (e.g. "🇸🇰").
        font_path: Path to a color emoji font.
        safe_zone_ratio: Fraction of the canvas the flag should occupy
            -- smaller for maskable icons, so the flag survives being
            cropped to a circle/squircle by the OS.
        companions: Emojis drawn under the flag, left to right. None or
            empty = the plain flag icon, unchanged.
    """
    canvas = sunset_gradient(size).convert("RGBA")
    flag = render_glyph(flag_emoji, font_path)

    if not companions:
        flag = resize_to_width(flag, size * safe_zone_ratio)
        canvas.alpha_composite(flag, ((size - flag.width) // 2, (size - flag.height) // 2))
        return canvas

    safe = size * safe_zone_ratio
    count = len(companions)
    companion_glyphs = [render_glyph(c, font_path) for c in companions]

    def layout(k: float):
        """Resized glyphs and gaps for a global scale k (1 = nominal)."""
        gap = round(safe * KIDS_COMPANION_GAP * k)
        # Each companion is at most KIDS_COMPANION_WIDTH wide, narrower
        # if there are so many that the row would overflow the safe zone.
        width_that_fits = (safe * k - (count - 1) * gap) / count
        companion_width = min(safe * KIDS_COMPANION_WIDTH * k, width_that_fits)
        glyphs = [resize_to_width(g, companion_width) for g in companion_glyphs]
        scaled_flag = resize_to_width(flag, safe * KIDS_FLAG_WIDTH * k)
        row_gap = round(safe * KIDS_ROW_GAP * k)
        row_height = max(g.height for g in glyphs)
        return scaled_flag, glyphs, gap, row_gap, row_height

    # The stack (flag over companions) is taller than it is wide: shrink
    # it as a whole until its height also fits the safe zone.
    k = 1.0
    scaled_flag, glyphs, gap, row_gap, row_height = layout(k)
    total_height = scaled_flag.height + row_gap + row_height
    if total_height > safe:
        k = safe / total_height
        scaled_flag, glyphs, gap, row_gap, row_height = layout(k)
        total_height = scaled_flag.height + row_gap + row_height

    # Different glyph heights (fox vs rabbit) are aligned on their
    # vertical centre.
    top = (size - total_height) // 2
    canvas.alpha_composite(scaled_flag, ((size - scaled_flag.width) // 2, top))

    x = (size - (sum(g.width for g in glyphs) + (count - 1) * gap)) // 2
    row_top = top + scaled_flag.height + row_gap
    for glyph in glyphs:
        canvas.alpha_composite(glyph, (x, row_top + (row_height - glyph.height) // 2))
        x += glyph.width + gap
    return canvas


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--lang", type=Path, required=True, help="Path to lang.json")
    parser.add_argument("-o", "--output-dir", type=Path, required=True, help="Directory to write icons into")
    args = parser.parse_args()

    with args.lang.open(encoding="utf-8") as f:
        lang_cfg = json.load(f)
    flag = lang_cfg["target_lang"]["flag"]
    companions = companions_from_config(lang_cfg)

    font_path = find_emoji_font()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    specs = [
        ("icon-192.png", 192, 0.72),
        ("icon-512.png", 512, 0.72),
        ("icon-maskable-512.png", 512, 0.55),  # smaller: maskable safe zone
    ]
    for filename, size, safe_zone_ratio in specs:
        icon = draw_flag_icon(size, flag, font_path, safe_zone_ratio, companions)
        icon.convert("RGB").save(args.output_dir / filename, "PNG")
        print(f"Wrote {args.output_dir / filename} ({size}x{size})")


if __name__ == "__main__":
    main()
