#!/usr/bin/env python3
"""Convertit art/ et maps/ en donnees C pour le firmware (src/game/gen/).

    python3 tools/build_assets.py

Reprend l'outil de Zajka Run (memes formats Image8 / Sheet / Font) et y ajoute :
  art/tiles/<theme>.png       planche de tuiles 16 x 16, 16 par ligne, dans l'ordre de TILESET
  art/walk/<nom>_16x24.png    planches de marche (lignes bas, haut, gauche, droite ; 3 images)
  maps/<id>.txt               cartes en texte : une lettre par tuile (TILESET), chiffres = reperes

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


# Tuiles du monde : (lettre dans maps/*.txt, nom, genre). L'ordre fixe la place dans la planche.
# Genres : W on marche dessus, S solide, O passe devant les personnages (le sol dessous = « ground »).
TILESET = [
    (" ", "vide", "S"),
    (".", "paves", "W"),
    (",", "paves2", "W"),
    ("_", "trottoir", "W"),
    ('"', "herbe", "W"),
    ("*", "fleurs", "S"),
    ("^", "toit", "S"),
    ("H", "facade", "S"),
    ("h", "porte", "S"),
    ("=", "rails", "W"),
    ("Z", "abri", "S"),
    ("l", "banc", "S"),
    ("i", "lampadaire", "S"),
    ("T", "tronc", "S"),
    ("O", "feuillage", "O"),
    ("f", "fontaine", "S"),
    ("E", "tram", "S"),
    ("e", "porte_tram", "W"),
    ("-", "plancher", "W"),
    ("s", "siege", "S"),
    ("W", "paroi", "S"),
    ("w", "caisse", "S"),
    ("|", "barre", "S"),
]
TILE_INDEX = {c: i for i, (c, _, _) in enumerate(TILESET)}
TILE_KINDS = {"W": "TK_WALK", "S": "TK_SOLID", "O": "TK_OVER"}
TILES_PER_ROW = 16
THEMES = ["kosice"]
DIRS = {"down": "DIR_DOWN", "up": "DIR_UP", "left": "DIR_LEFT", "right": "DIR_RIGHT"}
NPCS = {"andrea": "SPK_ANDREA", "mato": "SPK_MATO", "katka": "SPK_KATKA", "babka": "SPK_BABKA",
        "jan": "SPK_JAN", "marek": "SPK_MAREK", "human": "SPK_HUMAN"}


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


def c_str(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def parse_map(path):
    meta, marks, rows, in_map = {}, {}, [], False
    for line in open(path, encoding="utf-8").read().split("\n"):
        if not in_map:
            if line.strip() == "---":
                in_map = True
            elif line.startswith("#") or ":" not in line:
                continue
            else:
                k, v = (t.strip() for t in line.split(":", 1))
                if k.isdigit():
                    marks[k] = v.split()   # « 1: warp kosice_tram 9 5 up on e »
                else:
                    meta[k] = v
        else:
            rows.append(line.rstrip("\n"))
    while rows and rows[-1] == "":
        rows.pop()
    w = max(len(r) for r in rows)
    return meta, marks, [r.ljust(w) for r in rows]


def build_map(o, path, map_ids):
    mid = os.path.splitext(os.path.basename(path))[0]
    meta, marks, rows = parse_map(path)
    where = f"maps/{mid}.txt"
    ground = meta.get("ground", ".")
    if ground not in TILE_INDEX:
        raise SystemExit(f"{where} : ground « {ground} » inconnu")
    h, w = len(rows), len(rows[0])
    if w > 255 or h > 255:
        raise SystemExit(f"{where} : carte trop grande ({w} x {h}, 255 au plus)")
    base, over, cmarks = [], [], []
    for y, row in enumerate(rows):
        for x, ch in enumerate(row):
            if ch.isdigit():
                m = marks.get(ch)
                if m is None:
                    raise SystemExit(f"{where} : repere {ch} en ({x}, {y}) sans description « {ch}: ... »")
                on = m[m.index("on") + 1] if "on" in m else ground
                kind, args = m[0], m[1:m.index("on")] if "on" in m else m[1:]
                if kind == "start":
                    cmarks.append(f"{{M_START, {x}, {y}, {DIRS[args[0]]}, -1, 0, 0, SPK_NONE}}")
                elif kind == "warp":
                    tgt, tx, ty, tdir = args
                    if tgt not in map_ids:
                        raise SystemExit(f"{where} : porte {ch} vers une carte inconnue « {tgt} »")
                    cmarks.append(f"{{M_WARP, {x}, {y}, {DIRS[tdir]}, {map_ids.index(tgt)}, {tx}, {ty}, SPK_NONE}}")
                elif kind == "npc":
                    who, d = args
                    cmarks.append(f"{{M_NPC, {x}, {y}, {DIRS[d]}, -1, 0, 0, {NPCS[who]}}}")
                else:
                    raise SystemExit(f"{where} : repere {ch} de type inconnu « {kind} »")
                ch = on
            if ch not in TILE_INDEX:
                raise SystemExit(f"{where} : tuile inconnue « {ch} » en ({x}, {y})")
            if TILESET[TILE_INDEX[ch]][2] == "O":
                base.append(TILE_INDEX[ground])
                over.append(TILE_INDEX[ch])
            else:
                base.append(TILE_INDEX[ch])
                over.append(0)
    name = f"map_{c_ident(mid)}"
    o.bytes_array(f"{name}_base", base)
    over_ref = "nullptr"
    if any(over):
        o.bytes_array(f"{name}_over", over)
        over_ref = f"{name}_over"
    o.data.append(f"static const MapMark {name}_marks[{max(1, len(cmarks))}] = {{ " + ", ".join(cmarks or ["{}"]) + " };\n")
    music = int(meta.get("music", "-1"))
    o.data.append(f"const MapDef {name} = {{ {c_str(mid)}, {c_str(meta.get('name', mid))}, &tileset_{meta.get('theme', 'kosice')}, "
                  f"{w}, {h}, {name}_base, {over_ref}, {name}_marks, {len(cmarks)}, {music} }};\n")
    return name, rows, marks


def check_warps(maps):
    """Chaque porte doit arriver sur une case ou l'on peut marcher."""
    for mid, (meta, marks, rows) in maps.items():
        for ch, m in marks.items():
            if m[0] != "warp":
                continue
            tgt, tx, ty = m[1], int(m[2]), int(m[3])
            trows = maps[tgt][2]
            c = trows[ty][tx] if ty < len(trows) and tx < len(trows[ty]) else " "
            if c.isdigit() or TILESET[TILE_INDEX.get(c, 0)][2] == "S":
                raise SystemExit(f"maps/{mid}.txt : la porte {ch} arrive sur une case pleine ({tx}, {ty}) de {tgt}")


def main():
    os.makedirs(GEN, exist_ok=True)
    o = Out()
    for p in sorted(glob.glob(os.path.join(ART, "sprites", "*_*x*.png"))):
        name = "spr_" + c_ident(os.path.basename(p).split("_")[0])
        o.sheet(name, p)
    for p in sorted(glob.glob(os.path.join(ART, "walk", "*_*x*.png"))):
        name = "walk_" + c_ident(os.path.basename(p).split("_")[0])
        o.sheet(name, p)
    for name, ttf, size, thr in FONTS:
        o.font(name, ttf, size, thr)
    # tuiles et cartes
    o.data.append("static const uint8_t tile_kinds[%d] = { %s };\n" % (
        len(TILESET), ", ".join(TILE_KINDS[k] for _, _, k in TILESET)))
    for th in THEMES:
        o.image(f"tiles_{th}", Image.open(os.path.join(ART, "tiles", f"{th}.png")))
        o.data.append(f"const Tileset tileset_{th} = {{ &tiles_{th}, tile_kinds, {len(TILESET)} }};\n")
        o.decl.append(f"extern const Tileset tileset_{th};")
    paths = sorted(glob.glob(os.path.join(ROOT, "maps", "*.txt")))
    map_ids = [os.path.splitext(os.path.basename(p))[0] for p in paths]
    names, parsed = [], {}
    for p in paths:
        n, rows, marks = build_map(o, p, map_ids)
        names.append(n)
        parsed[os.path.splitext(os.path.basename(p))[0]] = ({}, marks, rows)
    check_warps(parsed)
    o.data.append("const MapDef* const ALL_MAPS[] = { " + ", ".join(f"&{n}" for n in names) + " };\n")
    o.data.append(f"const int NUM_MAPS = {len(names)};\n")
    for i, mid in enumerate(map_ids):
        o.decl.append(f"constexpr int MAP_{c_ident(mid).upper()} = {i};")

    with open(os.path.join(GEN, "assets.h"), "w") as f:
        f.write("// Genere par tools/build_assets.py, ne pas modifier a la main.\n#pragma once\n"
                "#include \"../gfx.h\"\n#include \"../world.h\"\n\n" + "\n".join(o.decl) + "\n")
    with open(os.path.join(GEN, "assets_data.inc"), "w") as f:
        f.write("// Genere par tools/build_assets.py, ne pas modifier a la main.\n\n" + "\n".join(o.data))
    size = os.path.getsize(os.path.join(GEN, "assets_data.inc"))
    print(f"src/game/gen : {len(o.decl)} declarations, {size // 1024} Ko de source")


if __name__ == "__main__":
    main()
