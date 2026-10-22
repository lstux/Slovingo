#!/usr/bin/env python3
"""Convertit art/ en donnees C pour le firmware (src/game/gen/).

    python3 tools/build_assets.py

Reprend l'outil de Zajka Run (memes formats Image8 / Sheet / Font). Pour l'instant :
polices et planches de sprites ; les cartes et le contenu du cours viendront ensuite.

Images : PNG RGBA -> 8 bits indexes, palette RGB565 (<= 255 couleurs + 0 = transparent).
Les fichiers generes sont commites : la compilation PlatformIO n'a pas besoin de Python/PIL.
"""
import glob
import os
import re

import numpy as np
from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ART = os.path.join(ROOT, "art")
GEN = os.path.join(ROOT, "src", "game", "gen")

FONTS = [
    # nom, fichier, taille, seuil
    ("font_small", "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 10, 110),
    ("font_big", "/usr/share/fonts/truetype/google-fonts/Poppins-Bold.ttf", 16, 120),
    ("font_title", "/usr/share/fonts/truetype/google-fonts/Poppins-Bold.ttf", 28, 120),
]
FONT_FALLBACK = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
CHARSET = ("".join(chr(c) for c in range(32, 127))
           + "áäčďéěíĺľňóôŕšťúýžÁÄČĎÉÍĹĽŇÓÔŔŠŤÚÝŽ"
           + "àâçèêëîïùûüÿœÀÂÇÈÊËÎÏÙÛÜŒ«»…×’—–·→›")


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


def main():
    os.makedirs(GEN, exist_ok=True)
    o = Out()
    for p in sorted(glob.glob(os.path.join(ART, "sprites", "*_*x*.png"))):
        name = "spr_" + c_ident(os.path.basename(p).split("_")[0])
        o.sheet(name, p)
    for name, ttf, size, thr in FONTS:
        o.font(name, ttf, size, thr)

    with open(os.path.join(GEN, "assets.h"), "w") as f:
        f.write("// Genere par tools/build_assets.py, ne pas modifier a la main.\n#pragma once\n"
                "#include \"../gfx.h\"\n\n" + "\n".join(o.decl) + "\n")
    with open(os.path.join(GEN, "assets_data.inc"), "w") as f:
        f.write("// Genere par tools/build_assets.py, ne pas modifier a la main.\n\n" + "\n".join(o.data))
    size = os.path.getsize(os.path.join(GEN, "assets_data.inc"))
    print(f"src/game/gen : {len(o.decl)} declarations, {size // 1024} Ko de source")


if __name__ == "__main__":
    main()
