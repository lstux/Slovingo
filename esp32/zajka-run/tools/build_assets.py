#!/usr/bin/env python3
"""Convertit art/ et levels/ en donnees C pour le firmware (src/game/gen/).

    python3 tools/build_assets.py

Images : PNG RGBA -> 8 bits indexes, palette RGB565 (<= 255 couleurs + 0 = transparent).
Tout PNG de la bonne taille peut remplacer un provisoire (image IA, dessin a la main) :
le script le quantifie tout seul. Les fichiers generes sont commites, la compilation
PlatformIO n'a donc pas besoin de Python/PIL.
"""
import glob
import os
import re

import numpy as np
from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ART = os.path.join(ROOT, "art")
GEN = os.path.join(ROOT, "src", "game", "gen")

# theme : point de passage, decor ('l', 'k') et couleurs sous le sol (fonds de trous)
THEMES = {
    "kosice": {"check": "spr_lampa", "decor": "spr_lavicka", "under": ((0x28, 0x2C, 0x28), (0x18, 0x18, 0x18))},
    "raj": {"check": "spr_rozcestnik", "decor": "spr_les", "under": ((0x20, 0x3a, 0x30), (0x10, 0x20, 0x20))},
    "sucho": {"check": "spr_lampa", "decor": "spr_lavicka", "under": ((0x4a, 0x36, 0x24), (0x2a, 0x1e, 0x16))},
    "tatry": {"check": "spr_rozcestnik", "decor": "spr_hory", "under": ((0x30, 0x34, 0x3a), (0x18, 0x1a, 0x20))},
}
FONTS = [
    # nom, fichier, taille, seuil
    ("font_small", "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 10, 110),
    ("font_big", "/usr/share/fonts/truetype/google-fonts/Poppins-Bold.ttf", 16, 120),
    ("font_title", "/usr/share/fonts/truetype/google-fonts/Poppins-Bold.ttf", 28, 120),
]
FONT_FALLBACK = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
CHARSET = ("".join(chr(c) for c in range(32, 127))
           + "áäčďéěíĺľňóôŕšťúýžÁÄČĎÉÍĹĽŇÓÔŔŠŤÚÝŽ"
           + "àâçèêëîïùûüÿœÀÂÇÈÊËÎÏÙÛÜŒ«»…×’—–·→›‹")


def rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def quantize(img, max_colors=255):
    """RGBA -> (indices uint8 avec 0 = transparent, palette [(r,g,b)])."""
    img = img.convert("RGBA")
    a = np.array(img)
    opaque = a[..., 3] >= 128
    rgb = a[..., :3]
    # couleurs reduites a 16 bits avant de compter : evite des doublons inutiles
    r5 = (rgb[..., 0] >> 3).astype(np.uint32)
    g6 = (rgb[..., 1] >> 2).astype(np.uint32)
    b5 = (rgb[..., 2] >> 3).astype(np.uint32)
    key = (r5 << 11) | (g6 << 5) | b5
    uniq = np.unique(key[opaque])
    if len(uniq) <= max_colors:
        lut = {int(k): i + 1 for i, k in enumerate(uniq)}
        idx = np.zeros(key.shape, np.uint8)
        flat = key[opaque]
        idx[opaque] = np.array([lut[int(k)] for k in flat], np.uint8) if flat.size else []
        pal = [(int(k) >> 11 << 3, (int(k) >> 5 & 63) << 2, (int(k) & 31) << 3) for k in uniq]
        return idx, pal
    # trop de couleurs (image IA) : quantification PIL sur les pixels opaques
    rgbimg = Image.fromarray(rgb.astype(np.uint8), "RGB")
    q = rgbimg.quantize(colors=max_colors, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    qi = np.array(q, np.uint8)
    p = q.getpalette()[: max_colors * 3]
    pal = [tuple(p[i:i + 3]) for i in range(0, len(p), 3)]
    idx = np.where(opaque, qi.astype(np.uint16) + 1, 0).astype(np.uint8)
    return idx, pal


def c_ident(s):
    return re.sub(r"[^a-zA-Z0-9_]", "_", s)


class Out:
    def __init__(self):
        self.data = []    # tableaux (fichier .inc)
        self.decl = []    # declarations (fichier .h)

    def bytes_array(self, name, data, typ="uint8_t", per_line=24):
        vals = list(data)
        lines = []
        for i in range(0, len(vals), per_line):
            lines.append("  " + ",".join(str(v) for v in vals[i:i + per_line]) + ",")
        self.data.append(f"static const {typ} {name}[{len(vals)}] PROGMEM = {{\n" + "\n".join(lines) + "\n};\n")

    def image(self, name, img):
        idx, pal = quantize(img)
        h, w = idx.shape
        self.bytes_array(f"{name}_px", idx.flatten())
        self.bytes_array(f"{name}_pal", [0] + [rgb565(*c) for c in pal], "uint16_t", 12)
        self.data.append(f"const Image8 {name} = {{ {w}, {h}, {name}_pal, {name}_px }};\n")
        self.decl.append(f"extern const Image8 {name};")
        return w, h

    def sheet(self, name, path):
        m = re.search(r"_(\d+)x(\d+)\.png$", path)
        fw, fh = int(m.group(1)), int(m.group(2))
        img = Image.open(path)
        w, h = self.image(f"img_{name}", img)
        self.data.append(f"const Sheet {name} = {{ &img_{name}, {fw}, {fh}, {w // fw} }};\n")
        self.decl.append(f"extern const Sheet {name};")

    def font(self, name, ttf, size, thr):
        try:
            ft = ImageFont.truetype(ttf, size)
        except OSError:
            ft = ImageFont.truetype(FONT_FALLBACK, size)
        asc, desc = ft.getmetrics()
        H = asc + desc
        cps, widths, offs, bits = [], [], [], []
        for ch in sorted(set(CHARSET), key=ord):
            adv = int(round(ft.getlength(ch)))
            if ch == " ":
                adv = max(4, int(size * 0.4))
            W = max(1, adv + 2)
            im = Image.new("L", (W + 4, H + 4), 0)
            ImageDraw.Draw(im).text((1, 1), ch, font=ft, fill=255)
            a = (np.array(im) > thr)[1:H + 1, 1:]
            cols = np.where(a.any(0))[0]
            gw = int(cols.max()) + 1 if len(cols) else 0
            adv = max(adv, gw) if ch != " " else adv
            a = a[:, :max(1, adv)]
            if a.shape[1] < adv:
                a = np.pad(a, ((0, 0), (0, adv - a.shape[1])))
            cps.append(ord(ch))
            widths.append(adv)
            offs.append(len(bits))
            rb = (adv + 7) // 8
            for y in range(H):
                for bx in range(rb):
                    v = 0
                    for b in range(8):
                        x = bx * 8 + b
                        if x < adv and a[y, x]:
                            v |= 0x80 >> b
                    bits.append(v)
        self.bytes_array(f"{name}_cp", cps, "uint16_t", 16)
        self.bytes_array(f"{name}_w", widths)
        self.bytes_array(f"{name}_off", offs, "uint16_t", 16)
        self.bytes_array(f"{name}_bits", bits)
        self.data.append(f"const Font {name} = {{ {H}, {len(cps)}, {name}_cp, {name}_w, {name}_off, {name}_bits }};\n")
        self.decl.append(f"extern const Font {name};")


def parse_level(path):
    meta, rows, in_map = {}, [], False
    for line in open(path, encoding="utf-8").read().split("\n"):
        if not in_map:
            if line.strip() == "---":
                in_map = True
            elif ":" in line and not line.startswith("#"):
                k, v = line.split(":", 1)
                meta[k.strip()] = v.strip()
        else:
            if line.startswith(";"):
                continue
            rows.append(line.rstrip("\n"))
    while rows and rows[-1] == "":
        rows.pop()
    w = max(len(r) for r in rows)
    rows = [r.ljust(w) for r in rows]
    return meta, rows


def c_str(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def main():
    os.makedirs(GEN, exist_ok=True)
    o = Out()
    for p in sorted(glob.glob(os.path.join(ART, "sprites", "*_*x*.png"))
                    + glob.glob(os.path.join(ART, "objects", "*_*x*.png"))):
        name = "spr_" + c_ident(os.path.basename(p).split("_")[0])
        o.sheet(name, p)
    for th, cfg in THEMES.items():
        o.image(f"{th}_far", Image.open(os.path.join(ART, th, "far.png")))
        o.image(f"{th}_mid", Image.open(os.path.join(ART, th, "mid.png")))
        o.image(f"{th}_tiles", Image.open(os.path.join(ART, th, "tiles.png")))
        u1, u2 = (rgb565(*c) for c in cfg["under"])
        o.data.append(f"const Theme theme_{th} = {{ &{th}_far, &{th}_mid, &{th}_tiles, &{cfg['check']}, "
                      f"&{cfg['decor']}, {u1}, {u2} }};\n")
        o.decl.append(f"extern const Theme theme_{th};")
    for name, ttf, size, thr in FONTS:
        o.font(name, ttf, size, thr)
    # niveaux
    levels = []
    for p in sorted(glob.glob(os.path.join(ROOT, "levels", "*.txt"))):
        meta, rows = parse_level(p)
        lid = "lv_" + c_ident(os.path.splitext(os.path.basename(p))[0])
        body = ",\n".join("  " + c_str(r) for r in rows)
        o.data.append(f"static const char* const {lid}_rows[{len(rows)}] = {{\n{body}\n}};\n")
        o.data.append(
            f"const LevelDef level_{lid} = {{ {c_str(meta.get('name', lid))}, {c_str(meta.get('subtitle', ''))}, "
            f"{c_str(meta.get('hint', ''))}, &theme_{meta.get('theme', 'kosice')}, {int(meta.get('music', 0))}, "
            f"{('&spr_' + meta['npc']) if meta.get('npc') else 'nullptr'}, {int(meta.get('weather', 0))}, "
            f"{c_str(meta.get('next', ''))}, "
            f"{len(rows[0])}, {len(rows)}, {lid}_rows }};\n")
        levels.append(lid)
    o.data.append("const LevelDef* const ALL_LEVELS[] = { " + ", ".join(f"&level_{l}" for l in levels) + " };\n")
    o.data.append(f"const int NUM_LEVELS = {len(levels)};\n")
    o.decl.append("extern const LevelDef* const ALL_LEVELS[];")
    o.decl.append("extern const int NUM_LEVELS;")

    with open(os.path.join(GEN, "assets.h"), "w") as f:
        f.write("// Genere par tools/build_assets.py, ne pas modifier a la main.\n#pragma once\n"
                "#include \"../gfx.h\"\n#include \"../level.h\"\n\n" + "\n".join(o.decl) + "\n")
    with open(os.path.join(GEN, "assets_data.inc"), "w") as f:
        f.write("// Genere par tools/build_assets.py, ne pas modifier a la main.\n\n" + "\n".join(o.data))
    size = os.path.getsize(os.path.join(GEN, "assets_data.inc"))
    print(f"src/game/gen : {len(o.decl)} declarations, {size // 1024} Ko de source")


if __name__ == "__main__":
    main()
