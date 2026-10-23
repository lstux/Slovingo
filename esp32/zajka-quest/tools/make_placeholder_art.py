#!/usr/bin/env python3
"""Graphismes PROVISOIRES de Zajka Quest, dessines pixel par pixel.

Ils fixent les formats attendus par tools/build_assets.py ; des images faites a la main ou par
IA les remplacent fichier par fichier, aux memes dimensions.

    art/tiles/kosice.png        tuiles 16 x 16, 16 par ligne, dans l'ordre de TILESET
    art/walk/fox_16x24.png      le renard en vue de dessus : lignes bas, haut, gauche, droite ;
    art/walk/andrea_16x24.png   colonnes debout, pied gauche, pied droit

    python3 tools/make_placeholder_art.py && python3 tools/build_assets.py
"""
import os
import random
import sys

from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ART = os.path.join(ROOT, "art")
sys.path.insert(0, HERE)
from build_assets import TILES_PER_ROW, TILESET  # noqa: E402

rnd = random.Random(7)
T = 16


def hexc(s):
    s = s.lstrip("#")
    return tuple(int(s[i:i + 2], 16) for i in (0, 2, 4)) + (255,)


# ------------------------------------------------------------------ tuiles
STONE = [hexc("#9a8f84"), hexc("#a89d90"), hexc("#8c8177")]
JOINT = hexc("#6e655d")
PAVE = hexc("#c9c0b4")
PAVE_LINE = hexc("#b2a99d")


def cobbles(d, seed):
    r = random.Random(seed)
    d.rectangle([0, 0, 15, 15], fill=JOINT)
    for row in range(4):
        off = 0 if row % 2 == 0 else -2
        for col in range(4):
            x0 = col * 4 + off
            d.rectangle([x0, row * 4, x0 + 2, row * 4 + 2], fill=r.choice(STONE))
            if x0 < 0:
                d.rectangle([x0 + 16, row * 4, x0 + 18, row * 4 + 2], fill=r.choice(STONE))


def pavement(d):
    d.rectangle([0, 0, 15, 15], fill=PAVE)
    d.line([(0, 7), (15, 7)], fill=PAVE_LINE)
    d.line([(0, 15), (15, 15)], fill=PAVE_LINE)
    d.line([(7, 0), (7, 7)], fill=PAVE_LINE)
    d.line([(15, 8), (15, 15)], fill=PAVE_LINE)


def grass(d, seed=3):
    r = random.Random(seed)
    d.rectangle([0, 0, 15, 15], fill=hexc("#5fae4a"))
    for _ in range(14):
        x, y = r.randrange(16), r.randrange(15)
        d.line([(x, y), (x, y + 1)], fill=hexc("#4a9038"))
    for _ in range(4):
        d.point((r.randrange(16), r.randrange(16)), fill=hexc("#8fd46a"))


def draw_tile(name):
    im = Image.new("RGBA", (T, T), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if name == "vide":
        d.rectangle([0, 0, 15, 15], fill=hexc("#000000"))
    elif name == "paves":
        cobbles(d, 1)
    elif name == "paves2":
        cobbles(d, 2)
        d.ellipse([5, 5, 10, 10], fill=hexc("#7d736a"))  # plaque d'egout
        d.ellipse([6, 6, 9, 9], fill=hexc("#5e5650"))
    elif name == "trottoir":
        pavement(d)
    elif name == "herbe":
        grass(d)
    elif name == "fleurs":
        grass(d, 9)
        d.rectangle([0, 0, 15, 15], outline=hexc("#7a6a58"))
        for x, y, c in [(3, 3, "#e8443a"), (10, 4, "#f4d03f"), (6, 9, "#e8443a"), (12, 11, "#f08fbf"),
                        (2, 12, "#f4d03f"), (9, 13, "#ffffff")]:
            d.rectangle([x, y, x + 1, y + 1], fill=hexc(c))
    elif name == "toit":
        d.rectangle([0, 0, 15, 15], fill=hexc("#b4533a"))
        for y in range(1, 16, 4):
            d.line([(0, y), (15, y)], fill=hexc("#8e3e2b"))
        d.rectangle([0, 13, 15, 15], fill=hexc("#e9dcc4"))  # corniche
    elif name in ("facade", "porte"):
        d.rectangle([0, 0, 15, 15], fill=hexc("#f0d9a8"))
        d.line([(0, 15), (15, 15)], fill=hexc("#cdb382"))
        if name == "facade":
            d.rectangle([4, 3, 11, 12], fill=hexc("#e9dcc4"))
            d.rectangle([5, 4, 10, 11], fill=hexc("#5a88b8"))
            d.line([(7, 4), (7, 11)], fill=hexc("#e9dcc4"))
            d.line([(5, 7), (10, 7)], fill=hexc("#e9dcc4"))
        else:
            d.rectangle([3, 2, 12, 15], fill=hexc("#7a4a2a"))
            d.rectangle([4, 3, 11, 15], fill=hexc("#94603a"))
            d.point((10, 9), fill=hexc("#f4d03f"))
    elif name == "rails":
        cobbles(d, 4)
        for y in (4, 11):
            d.line([(0, y), (15, y)], fill=hexc("#5a5a62"))
            d.line([(0, y - 1), (15, y - 1)], fill=hexc("#c8c8d0"))
    elif name == "abri":
        pavement(d)
        d.rectangle([1, 1, 14, 3], fill=hexc("#2e7d4f"))
        d.rectangle([2, 4, 3, 15], fill=hexc("#555a60"))
        d.rectangle([12, 4, 13, 15], fill=hexc("#555a60"))
        d.rectangle([4, 5, 11, 10], fill=hexc("#bcd8e8"))
        d.rectangle([6, 6, 9, 8], fill=hexc("#f4d03f"))  # panneau « Z »
    elif name == "banc":
        cobbles(d, 5)
        d.rectangle([1, 5, 14, 7], fill=hexc("#8e5a32"))
        d.rectangle([1, 9, 14, 10], fill=hexc("#a86c3e"))
        d.rectangle([2, 11, 3, 13], fill=hexc("#3a3a3a"))
        d.rectangle([12, 11, 13, 13], fill=hexc("#3a3a3a"))
    elif name == "lampadaire":
        pavement(d) if False else cobbles(d, 6)
        d.rectangle([7, 3, 8, 14], fill=hexc("#2f3238"))
        d.rectangle([5, 0, 10, 3], fill=hexc("#2f3238"))
        d.rectangle([6, 1, 9, 2], fill=hexc("#fff3b0"))
        d.rectangle([5, 14, 10, 15], fill=hexc("#2f3238"))
    elif name == "tronc":
        cobbles(d, 7)
        d.ellipse([3, 10, 12, 15], fill=hexc("#5c4a3a"))
        d.rectangle([6, 0, 9, 12], fill=hexc("#7a5636"))
        d.line([(7, 0), (7, 11)], fill=hexc("#94683f"))
    elif name == "feuillage":
        for cx, cy, rr in [(8, 9, 7), (4, 10, 4), (12, 10, 4), (8, 5, 5)]:
            d.ellipse([cx - rr, cy - rr, cx + rr, cy + rr], fill=hexc("#3f8f3a"))
        for cx, cy in [(6, 6), (10, 8), (5, 11)]:
            d.ellipse([cx - 2, cy - 2, cx + 1, cy + 1], fill=hexc("#5fb04f"))
    elif name == "fontaine":
        cobbles(d, 8)
        d.rectangle([0, 2, 15, 15], fill=hexc("#b8b0a4"))
        d.rectangle([1, 3, 14, 14], fill=hexc("#4f9fd8"))
        d.rectangle([3, 6, 12, 11], fill=hexc("#7cc3ee"))
        d.rectangle([7, 4, 8, 13], fill=hexc("#e8f6ff"))
    elif name in ("tram", "porte_tram"):
        d.rectangle([0, 0, 15, 15], fill=hexc("#d63a3a"))   # tram de Kosice : rouge et creme
        d.rectangle([0, 0, 15, 2], fill=hexc("#f2e6c8"))
        d.rectangle([0, 13, 15, 15], fill=hexc("#3a3a3a"))
        if name == "tram":
            d.rectangle([2, 4, 13, 9], fill=hexc("#9cc7e4"))
            d.line([(2, 4), (13, 4)], fill=hexc("#d6ecf8"))
        else:
            d.rectangle([3, 3, 12, 15], fill=hexc("#f2e6c8"))
            d.rectangle([4, 4, 7, 12], fill=hexc("#9cc7e4"))
            d.rectangle([8, 4, 11, 12], fill=hexc("#9cc7e4"))
    elif name == "plancher":
        d.rectangle([0, 0, 15, 15], fill=hexc("#6b6f78"))
        for y in (3, 7, 11, 15):
            d.line([(0, y), (15, y)], fill=hexc("#5c6068"))
    elif name == "siege":
        d.rectangle([0, 0, 15, 15], fill=hexc("#6b6f78"))
        d.rectangle([2, 1, 13, 14], fill=hexc("#1f4f8f"))   # tissu bleu
        d.rectangle([2, 1, 13, 4], fill=hexc("#2c66b0"))
        d.rectangle([3, 6, 12, 13], fill=hexc("#28599c"))
    elif name == "paroi":
        d.rectangle([0, 0, 15, 15], fill=hexc("#e9e2d0"))
        d.rectangle([1, 2, 14, 10], fill=hexc("#9cc7e4"))
        d.line([(1, 3), (14, 3)], fill=hexc("#d6ecf8"))
        d.rectangle([0, 12, 15, 15], fill=hexc("#c9c0ac"))
    elif name == "caisse":
        d.rectangle([0, 0, 15, 15], fill=hexc("#d63a3a"))
        d.rectangle([0, 0, 15, 2], fill=hexc("#a82a2a"))
    elif name == "barre":
        d.rectangle([0, 0, 15, 15], fill=hexc("#6b6f78"))
        for y in (3, 7, 11, 15):
            d.line([(0, y), (15, y)], fill=hexc("#5c6068"))
        d.ellipse([5, 9, 10, 14], fill=hexc("#4a4d54"))
        d.rectangle([7, 0, 8, 11], fill=hexc("#f4d03f"))
    else:
        raise SystemExit(f"tuile sans dessin : {name}")
    return im


def tiles():
    rows = (len(TILESET) + TILES_PER_ROW - 1) // TILES_PER_ROW
    sheet = Image.new("RGBA", (TILES_PER_ROW * T, rows * T), (0, 0, 0, 0))
    for i, (_, name, _) in enumerate(TILESET):
        sheet.paste(draw_tile(name), ((i % TILES_PER_ROW) * T, (i // TILES_PER_ROW) * T))
    os.makedirs(os.path.join(ART, "tiles"), exist_ok=True)
    sheet.save(os.path.join(ART, "tiles", "kosice.png"))
    print("ecrit art/tiles/kosice.png", sheet.size)


# ------------------------------------------------------------------ personnages 16 x 24
# Dessins en lettres : une lettre = une couleur de la palette du personnage, « . » = transparent.
# Vues de face et de dos : on ne dessine que la moitie gauche (8 colonnes), le reste est en miroir.
# Les jambes (lignes 16 a 23) sont donnees a part pour chaque image de marche.

FOX_PAL = {"k": "#1a1a2e", "o": "#f07a2a", "O": "#c85a18", "c": "#fff1d6", "g": "#4a4a52",
           "e": "#111111", "b": "#2f5fd0", "B": "#1f3f99", "y": "#fbbf24", "n": "#24305e",
           "w": "#f4f4f4"}
FOX = {
    "down": ["........",
             "..kk....",
             ".kgok...",
             ".kooOkkk",
             ".koooooo",
             "kooeoooo",
             "koooocco",
             "kocccckk",
             ".kcccccc",
             "..kkkkkk",
             ".kbbybbb",
             "kbbbybbb",
             "kbbbybbb",
             "kobbybbb",
             ".kBBBBBB",
             ".knnnnnn"],
    "up": ["........",
           "..kk....",
           ".kgok...",
           ".kooOkkk",
           ".koooooo",
           "kooooooo",
           "kooooooo",
           "koOooooo",
           ".kOOoooo",
           "..kkkkkk",
           ".kbbbyyy",
           "kbbbyyyy",
           "kbbbyyyy",
           "kobbyyyy",
           ".kBBBkkk",
           ".knnnnnk"],
    "left": ["................",
             ".......kk.......",
             "......kgok......",
             "....kkooOk......",
             "...koooooOk.....",
             "..kooeoooook....",
             ".kcccoooooook...",
             "kkcccoooooook...",
             ".kkccooooook....",
             "...kkkkkkkk.....",
             "....kbbbbbk.....",
             "...kbbbybbk.kk..",
             "...kbbbybbkkook.",
             "...kobbybbkooock",
             "....kBBBBBkoock.",
             "....knnnnnkkkk.."],
}
FOX_TAIL_DOWN = {13: (12, "kook"), 14: (12, "kock"), 12: (13, "kk")}  # la queue depasse a droite

ANDREA_PAL = {"k": "#1a1a2e", "r": "#b5c1c6", "R": "#8a969c", "p": "#fba9a9", "W": "#ffffff",
              "e": "#111111", "m": "#7fd6b0", "M": "#5fb894", "j": "#4a6fa5", "s": "#d63a3a",
              "Y": "#fbbf24", "w": "#f4f4f4"}
ANDREA = {
    "down": ["..kk....",
             ".kpk....",
             ".kpk....",
             ".kpk.kkk",
             ".krkkrrr",
             ".krrrrrr",
             "krrerrrr",
             "krrrrrWW",
             "krrrWWkk",
             ".kkWWWWW",
             "..kkkkkk",
             ".kmmmMmm",
             "kmmmmMmm",
             "krmmmmMm",
             ".kYjjjjj",
             ".kjjjjjj"],
    "up": ["..kk....",
           ".krk....",
           ".krk....",
           ".krk.kkk",
           ".krkkrrr",
           ".krrrrrr",
           "krrrrrrr",
           "krrrrrrr",
           "kRrrrrrr",
           ".kRRrrrr",
           "..kkkkkk",
           ".kmmmmmm",
           "kmmmmmmm",
           "krmmmmmm",
           ".kjjjjjj",
           ".kjjjjjj"],
    "left": ["........kk......",
             "......kkpk......",
             ".....kpkpk......",
             ".....kpkrk......",
             "....kkrrrk......",
             "...krrrrrrk.....",
             "..krerrrrrrk....",
             ".kWWWrrrrrrk....",
             "kkWWWrrrrrk.....",
             ".kkWWrrrrk......",
             "...kkkkkkk......",
             "....kmmmmmk.....",
             "...kmmmMmmk.....",
             "...krmmmmMkY....",
             "....kjjjjjkk....",
             "....kjjjjjk....."],
}

# jambes : debout, pied gauche en avant, pied droit en avant (vue de face : 8 colonnes en miroir)
LEGS_FRONT = {
    "fox": {"leg": "n", "shoe": "w"},
    "andrea": {"leg": "r", "shoe": "s"},
}


def legs_front(leg, shoe, lift):
    """8 lignes de 16 colonnes ; lift = -1 aucun, 0 jambe gauche levee, 1 jambe droite levee."""
    left = ["..k{L}{L}{L}k.".format(L=leg),
            "..k{L}{L}{L}k.".format(L=leg),
            "..k{L}{L}{L}k.".format(L=leg),
            "..k{L}{L}{L}k.".format(L=leg),
            "..k{S}{S}{S}k.".format(S=shoe),
            ".k{S}{S}{S}{S}k.".format(S=shoe),
            "..kkkk..",
            "........"]
    up = left[1:] + ["........"]
    rows = []
    for i in range(8):
        a = (up if lift == 0 else left)[i]
        b = (up if lift == 1 else left)[i]
        rows.append(a + b[::-1])
    return rows


def legs_side(leg, shoe, phase):
    """Vue de profil (vers la gauche) : 0 debout, 1 et 2 jambes ecartees."""
    if phase == 0:
        rows = ["....k{L}{L}{L}{L}k......",
                "....k{L}{L}{L}{L}k......",
                ".....k{L}{L}k.......",
                ".....k{L}{L}k.......",
                ".....k{L}{L}k.......",
                "....k{S}{S}{S}k.......",
                "...kkkkkk.......",
                "................"]
    else:
        rows = ["....k{L}{L}{L}{L}k......",
                "...k{L}{L}kk{L}{L}k.....",
                "..k{L}{L}k..k{L}{L}k....",
                "..k{L}k....k{L}k....",
                ".k{S}{S}k....k{S}{S}k...",
                "kk{S}{S}k....kk{S}k...",
                ".kkkk.....kkk...",
                "................"]
        if phase == 2:  # l'autre jambe devant : on decale d'un pixel pour varier
            rows = [r[1:] + "." for r in rows]
    return [r.format(L=leg, S=shoe) for r in rows]


def paint(rows, pal):
    im = Image.new("RGBA", (16, 24), (0, 0, 0, 0))
    for y, row in enumerate(rows):
        for x, ch in enumerate(row[:16]):
            if ch != ".":
                im.putpixel((x, y), hexc(pal[ch]))
    return im


def character(name, art, pal, tail=None):
    legs = LEGS_FRONT[name]
    sheet = Image.new("RGBA", (16 * 3, 24 * 4), (0, 0, 0, 0))
    for row, view in enumerate(["down", "up", "left"]):
        top = art[view]
        if len(top[0]) == 8:
            top = [r + r[::-1] for r in top]
        for col in range(3):
            if view == "left":
                bottom = legs_side(legs["leg"], legs["shoe"], col)
            else:
                bottom = legs_front(legs["leg"], legs["shoe"], [-1, 0, 1][col])
            rows = [list(r) for r in top + bottom]
            if tail and view == "down":
                for y, (x0, seg) in tail.items():
                    for i, ch in enumerate(seg):
                        if x0 + i < 16:
                            rows[y][x0 + i] = ch
            im = paint(["".join(r) for r in rows], pal)
            sheet.paste(im, (col * 16, row * 24))
            if view == "left":  # droite = gauche en miroir
                sheet.paste(im.transpose(Image.FLIP_LEFT_RIGHT), (col * 16, 3 * 24))
    os.makedirs(os.path.join(ART, "walk"), exist_ok=True)
    out = os.path.join(ART, "walk", f"{name}_16x24.png")
    sheet.save(out)
    print(f"ecrit art/walk/{name}_16x24.png", sheet.size)


if __name__ == "__main__":
    tiles()
    character("fox", FOX, FOX_PAL, FOX_TAIL_DOWN)
    character("andrea", ANDREA, ANDREA_PAL)
