#!/usr/bin/env python3
"""Graphismes PROVISOIRES de Zajka Quest, dessines pixel par pixel.

Ils fixent les formats attendus par tools/build_assets.py ; des images faites a la main ou par
IA les remplacent fichier par fichier, aux memes dimensions.

    art/tiles/kosice.png        tuiles 16 x 16, 16 par ligne, dans l'ordre de TILESET
    art/walk/fox_16x24.png      le renard en vue de dessus : lignes bas, haut, gauche, droite ;
    art/walk/andrea_16x24.png   colonnes debout, pied gauche, pied droit
    art/walk/human_16x24.png    un adulte (passants et voyageurs, en attendant de vrais personnages)
    art/portraits/<nom>_48x48   portraits de la boite de dialogue : les tetes SVG de Slovingo

    python3 tools/make_placeholder_art.py && python3 tools/build_assets.py
    python3 tools/make_placeholder_art.py --new     # seulement ce qui manque (ne touche pas aux images IA)
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
    elif name in ("maison", "porte_maison"):
        d.rectangle([0, 0, 15, 15], fill=hexc("#e6e0d0"))       # crepi clair
        d.line([(0, 15), (15, 15)], fill=hexc("#b8b0a0"))
        if name == "maison":
            d.rectangle([4, 3, 11, 11], fill=hexc("#8a6a4a"))   # volets
            d.rectangle([5, 4, 10, 10], fill=hexc("#ffd970"))   # fenetre allumee (c'est le soir)
            d.line([(7, 4), (7, 10)], fill=hexc("#8a6a4a"))
            d.line([(5, 7), (10, 7)], fill=hexc("#8a6a4a"))
        else:
            d.rectangle([4, 2, 11, 15], fill=hexc("#5a3a22"))
            d.rectangle([5, 3, 10, 15], fill=hexc("#704a2c"))
            d.point((9, 9), fill=hexc("#f4d03f"))
            d.rectangle([3, 14, 12, 15], fill=hexc("#9a8f84"))
    elif name == "cloture":
        grass(d, 11)
        d.rectangle([0, 6, 15, 7], fill=hexc("#8e6a44"))
        d.rectangle([0, 11, 15, 12], fill=hexc("#8e6a44"))
        for x in (1, 6, 11):
            d.rectangle([x, 2, x + 2, 15], fill=hexc("#b08a5a"))
            d.point((x + 1, 2), fill=hexc("#d8b884"))
    elif name == "portail":
        d.rectangle([0, 0, 15, 15], fill=hexc("#c9b99c"))       # gravier de l'allee
        for x, y in [(3, 4), (10, 2), (6, 9), (12, 12), (2, 13)]:
            d.point((x, y), fill=hexc("#a8987c"))
        d.rectangle([0, 0, 1, 15], fill=hexc("#8e6a44"))        # montants du portail ouvert
        d.rectangle([14, 0, 15, 15], fill=hexc("#8e6a44"))
    elif name == "allee":
        grass(d, 12)
        d.rectangle([3, 0, 12, 15], fill=hexc("#c9b99c"))
        for x, y in [(5, 4), (10, 2), (7, 9), (11, 12), (4, 13)]:
            d.point((x, y), fill=hexc("#a8987c"))
    elif name == "pommier":
        grass(d, 13)
        d.ellipse([3, 11, 12, 15], fill=hexc("#4a6a32"))
        d.rectangle([6, 0, 9, 13], fill=hexc("#6a4a2e"))
        d.line([(7, 0), (7, 12)], fill=hexc("#84603a"))
    elif name == "feuillage_pommier":
        for cx, cy, rr in [(8, 8, 7), (4, 9, 4), (12, 9, 4), (8, 4, 5)]:
            d.ellipse([cx - rr, cy - rr, cx + rr, cy + rr], fill=hexc("#3f8a3a"))
        for cx, cy in [(5, 6), (11, 7), (8, 11), (3, 10), (12, 4)]:
            d.rectangle([cx, cy, cx + 1, cy + 1], fill=hexc("#e8443a"))  # pommes
    elif name == "buisson":
        grass(d, 14)
        d.ellipse([1, 3, 14, 15], fill=hexc("#2f7a34"))
        d.ellipse([4, 4, 10, 9], fill=hexc("#4f9a44"))
    elif name == "trafika":
        cobbles(d, 15)
        d.rectangle([1, 3, 14, 15], fill=hexc("#2e7d4f"))        # kiosque vert
        d.rectangle([0, 1, 15, 4], fill=hexc("#e8443a"))         # auvent rouge
        d.rectangle([3, 6, 12, 11], fill=hexc("#f2e6c8"))        # comptoir, journaux
        d.rectangle([4, 7, 6, 10], fill=hexc("#5a88b8"))
        d.rectangle([8, 7, 11, 9], fill=hexc("#f4d03f"))
    elif name == "composteur":
        d.rectangle([0, 0, 15, 15], fill=hexc("#6b6f78"))
        for y in (3, 7, 11, 15):
            d.line([(0, y), (15, y)], fill=hexc("#5c6068"))
        d.rectangle([7, 8, 8, 15], fill=hexc("#9a9ea8"))         # pied
        d.rectangle([4, 1, 11, 9], fill=hexc("#f4d03f"), outline=hexc("#1a1a2e"))  # boitier jaune
        d.rectangle([6, 3, 9, 4], fill=hexc("#1a1a2e"))          # fente
        d.point((9, 7), fill=hexc("#e8443a"))
    elif name == "panneau":
        cobbles(d, 16)
        d.rectangle([7, 6, 8, 15], fill=hexc("#5a5a62"))
        d.rectangle([1, 1, 14, 7], fill=hexc("#2a5aa8"), outline=hexc("#1a1a2e"))
        d.line([(3, 4), (12, 4)], fill=hexc("#f4f4f4"))
    elif name == "fontaine_vide":
        cobbles(d, 8)
        d.rectangle([0, 2, 15, 15], fill=hexc("#b8b0a4"))
        d.rectangle([1, 3, 14, 14], fill=hexc("#9a8e7e"))       # fond sec
        d.line([(3, 6), (6, 8), (5, 11)], fill=hexc("#7d7264"))  # fissures
        d.line([(10, 5), (11, 9), (13, 10)], fill=hexc("#7d7264"))
        d.rectangle([7, 4, 8, 13], fill=hexc("#8a8a8a"))        # jet a l'arret
    elif name == "eau":
        d.rectangle([0, 0, 15, 15], fill=hexc("#3f7fc0"))
        d.line([(2, 4), (6, 4)], fill=hexc("#7cb8ec"))
        d.line([(9, 10), (13, 10)], fill=hexc("#7cb8ec"))
    elif name == "boue":
        d.rectangle([0, 0, 15, 15], fill=hexc("#a88a5e"))       # vase seche, craquelee
        for pts in ([(0, 5), (5, 6), (8, 3)], [(5, 6), (7, 11), (3, 15)], [(8, 3), (12, 7), (15, 6)],
                    [(12, 7), (11, 12), (15, 14)]):
            d.line(pts, fill=hexc("#7e6440"))
    elif name == "guichet":
        d.rectangle([0, 0, 15, 15], fill=hexc("#c9b99c"))
        d.rectangle([0, 9, 15, 15], fill=hexc("#8e6a44"))       # comptoir en bois
        d.rectangle([2, 1, 13, 8], fill=hexc("#bcd8e8"))        # vitre
        d.rectangle([5, 6, 10, 8], fill=hexc("#1a1a2e"))        # hygiaphone
    elif name == "sol_gare":
        d.rectangle([0, 0, 15, 15], fill=hexc("#d8d0c0"))
        d.line([(0, 7), (15, 7)], fill=hexc("#b8b0a0"))
        d.line([(7, 0), (7, 7)], fill=hexc("#b8b0a0"))
        d.line([(0, 15), (15, 15)], fill=hexc("#b8b0a0"))
        d.line([(15, 8), (15, 15)], fill=hexc("#b8b0a0"))
    elif name in ("train", "porte_train"):
        d.rectangle([0, 0, 15, 15], fill=hexc("#2e5aa8"))       # wagon bleu des ZSSK
        d.rectangle([0, 0, 15, 2], fill=hexc("#c8c8d0"))
        d.rectangle([0, 12, 15, 15], fill=hexc("#3a3a3a"))
        if name == "train":
            d.rectangle([2, 4, 13, 9], fill=hexc("#9cc7e4"))
            d.line([(0, 11), (15, 11)], fill=hexc("#f4d03f"))
        else:
            d.rectangle([3, 3, 12, 15], fill=hexc("#c8c8d0"))
            d.rectangle([4, 4, 7, 11], fill=hexc("#9cc7e4"))
            d.rectangle([8, 4, 11, 11], fill=hexc("#9cc7e4"))
    elif name == "sapin":
        grass(d, 21)
        d.rectangle([7, 12, 8, 15], fill=hexc("#5a3e26"))
        for y0, hw, c in ((9, 6, "#1f5a30"), (5, 5, "#26703a"), (1, 4, "#2f8a46")):
            for k in range(0, 5):
                w = max(1, hw - (4 - k) * 1)
            d.polygon([(8, y0 - 1), (8 - hw, y0 + 5), (8 + hw, y0 + 5)], fill=hexc(c), outline=hexc("#17421f"))
        d.point((7, 4), fill=hexc("#58b36c"))
    elif name == "sentier":
        d.rectangle([0, 0, 15, 15], fill=hexc("#a8885c"))
        for x, y in [(2, 2), (9, 1), (5, 6), (12, 8), (3, 11), (10, 13), (14, 4)]:
            d.rectangle([x, y, x + 1, y], fill=hexc("#8a6c44"))
        for x, y in [(6, 3), (13, 11), (1, 8)]:
            d.point((x, y), fill=hexc("#c8aa7c"))
    elif name == "rocher":
        grass(d, 22)
        d.ellipse([1, 4, 14, 15], fill=hexc("#8a8a90"), outline=hexc("#4a4a52"))
        d.ellipse([3, 5, 9, 9], fill=hexc("#aeaeb4"))
        d.line([(9, 9), (12, 12)], fill=hexc("#6a6a72"))
    elif name == "falaise":
        d.rectangle([0, 0, 15, 15], fill=hexc("#6e6258"))
        for y in (3, 8, 12):
            d.line([(0, y), (15, y)], fill=hexc("#544a42"))
        for x, y in [(3, 1), (10, 5), (6, 10), (13, 14), (1, 6)]:
            d.rectangle([x, y, x + 2, y], fill=hexc("#80736a"))
    elif name == "ruisseau":
        d.rectangle([0, 0, 15, 15], fill=hexc("#4b93cf"))
        for x, y in [(2, 3), (9, 7), (4, 12), (12, 2)]:
            d.line([(x, y), (x + 3, y)], fill=hexc("#a6d4f4"))
        d.line([(0, 15), (15, 15)], fill=hexc("#3a78b0"))
    elif name == "eau_gorge":
        d.rectangle([0, 0, 15, 15], fill=hexc("#2f6f9a"))
        for x, y in [(1, 2), (8, 5), (3, 10), (11, 12)]:
            d.line([(x, y), (x + 4, y)], fill=hexc("#8cc4e4"))
        for x, y in [(5, 7), (13, 1)]:
            d.rectangle([x, y, x + 1, y], fill=hexc("#e8f6ff"))
    elif name == "planche":
        d.rectangle([0, 0, 15, 15], fill=hexc("#2f6f9a"))               # l'eau voit entre les planches
        for y0 in (0, 5, 10):
            d.rectangle([0, y0, 15, y0 + 3], fill=hexc("#b07a3c"), outline=hexc("#6a4222"))
            d.line([(1, y0 + 1), (14, y0 + 1)], fill=hexc("#cc9658"))
            d.point((2, y0 + 2), fill=hexc("#2e2e34"))
            d.point((13, y0 + 2), fill=hexc("#2e2e34"))
        d.line([(7, 0), (6, 2), (8, 3)], fill=hexc("#4a2e16"))           # fendue : elle ne tiendra qu'un pas
        d.line([(9, 10), (10, 12), (8, 13)], fill=hexc("#4a2e16"))
    elif name == "rebrik":
        d.rectangle([0, 0, 15, 15], fill=hexc("#544a42"))
        d.rectangle([3, 0, 4, 15], fill=hexc("#b07a3c"))
        d.rectangle([11, 0, 12, 15], fill=hexc("#b07a3c"))
        for y in (1, 5, 9, 13):
            d.rectangle([3, y, 12, y + 1], fill=hexc("#d09a54"))
    elif name == "most":
        d.rectangle([0, 0, 15, 15], fill=hexc("#a07040"))
        for x in (3, 8, 13):
            d.line([(x, 2), (x, 13)], fill=hexc("#6a4222"))
        d.rectangle([0, 0, 15, 2], fill=hexc("#7a5230"))
        d.rectangle([0, 13, 15, 15], fill=hexc("#7a5230"))
        for x in (1, 8, 14):
            d.rectangle([x, 0, x + 1, 3], fill=hexc("#3e2a18"))
            d.rectangle([x, 12, x + 1, 15], fill=hexc("#3e2a18"))
    elif name == "dalle":
        d.rectangle([0, 0, 15, 15], fill=hexc("#9a9288"))
        d.line([(0, 7), (15, 7)], fill=hexc("#6e665e"))
        d.line([(0, 15), (15, 15)], fill=hexc("#6e665e"))
        d.line([(5, 0), (5, 7)], fill=hexc("#6e665e"))
        d.line([(11, 8), (11, 15)], fill=hexc("#6e665e"))
        for x, y in [(2, 2), (9, 4), (7, 11), (13, 13)]:
            d.point((x, y), fill=hexc("#b4aca0"))
    elif name == "ohnisko":
        grass(d, 23)
        for x, y in [(3, 10), (6, 12), (9, 12), (12, 10), (4, 13), (11, 13)]:
            d.rectangle([x, y, x + 2, y + 2], fill=hexc("#7a7a82"), outline=hexc("#4a4a52"))
        d.polygon([(8, 2), (12, 9), (10, 11), (6, 11), (4, 9)], fill=hexc("#f07a2a"))
        d.polygon([(8, 5), (10, 9), (9, 10), (7, 10), (6, 9)], fill=hexc("#f8d850"))
        d.rectangle([5, 11, 11, 12], fill=hexc("#5a3e26"))
    else:
        raise SystemExit(f"tuile sans dessin : {name}")
    return im


def tiles(only_new=False):
    """Toute la planche, ou (--new) seulement les tuiles pas encore dessinees : une tuile qui a deja
    des pixels (dessinee a la main ou par IA) n'est jamais ecrasee."""
    rows = (len(TILESET) + TILES_PER_ROW - 1) // TILES_PER_ROW
    sheet = Image.new("RGBA", (TILES_PER_ROW * T, rows * T), (0, 0, 0, 0))
    path = os.path.join(ART, "tiles", "kosice.png")
    if only_new and os.path.exists(path):
        old = Image.open(path).convert("RGBA")
        sheet.paste(old, (0, 0))
    drawn = []
    for i, (_, name, _) in enumerate(TILESET):
        box = ((i % TILES_PER_ROW) * T, (i // TILES_PER_ROW) * T, (i % TILES_PER_ROW) * T + T, (i // TILES_PER_ROW) * T + T)
        if only_new and i > 0 and sheet.crop(box).getbbox():
            continue
        sheet.paste(draw_tile(name), box[:2])
        drawn.append(name)
    os.makedirs(os.path.join(ART, "tiles"), exist_ok=True)
    sheet.save(path)
    print("ecrit art/tiles/kosice.png", sheet.size, ("(nouvelles : " + " ".join(drawn) + ")") if only_new else "")


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

# Un adulte humain (passants, voyageurs) : cheveux bruns, veste grise, pantalon sombre.
HUMAN_PAL = {"k": "#1a1a2e", "h": "#5a3a22", "s": "#f0c8a0", "S": "#d8a880", "e": "#111111",
             "c": "#7a8290", "C": "#5a6270", "w": "#f4f4f4", "n": "#2e3440", "b": "#3a2a1e"}
HUMAN = {
    "down": ["........",
             "........",
             "....kkkk",
             "...khhhh",
             "..khhhhh",
             "..khssss",
             "..kssess",
             "..kssssS",
             "...kssss",
             "....kkSS",
             "..kkcccc",
             ".kcccwcc",
             ".kcccwcc",
             ".ksccwcc",
             "..kCCCCC",
             "..knnnnn"],
    "up": ["........",
           "........",
           "....kkkk",
           "...khhhh",
           "..khhhhh",
           "..khhhhh",
           "..khhhhh",
           "..khhhhh",
           "...khhhh",
           "....kkSS",
           "..kkcccc",
           ".kcccccc",
           ".kcccccc",
           ".ksccccc",
           "..kCCCCC",
           "..knnnnn"],
    "left": ["................",
             "................",
             ".....kkkk.......",
             "....khhhhk......",
             "...khhhhhhk.....",
             "...kssshhhk.....",
             "..ksesshhhk.....",
             "..kssssshk......",
             "...kSsssk.......",
             "....kkSk........",
             "....kccccck.....",
             "...kcccccck.....",
             "...kcwcccck.....",
             "...kscccccck....",
             "....kCCCCCk.....",
             "....knnnnnk....."],
}

# jambes : debout, pied gauche en avant, pied droit en avant (vue de face : 8 colonnes en miroir)
LEGS_FRONT = {
    "fox": {"leg": "n", "shoe": "w"},
    "andrea": {"leg": "r", "shoe": "s"},
    "human": {"leg": "n", "shoe": "b"},
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


def creature(name, rows, pal):
    """Petit animal (souris, poussin) : un dessin de 16 x 12 en bas de la case, le meme dans les
    quatre sens, qui sautille un peu pour marcher."""
    sheet = Image.new("RGBA", (16 * 3, 24 * 4), (0, 0, 0, 0))
    base = paint(["." * 16] * (24 - len(rows)) + rows, pal)
    for r in range(4):
        for c in range(3):
            img = base if c == 0 else base.transform(base.size, Image.AFFINE, (1, 0, 0, 0, 1, 1))
            if r == 2:
                img = img.transpose(Image.FLIP_LEFT_RIGHT)
            sheet.paste(img, (c * 16, r * 24))
    sheet.save(os.path.join(ART, "walk", f"{name}_16x24.png"))
    print(f"ecrit art/walk/{name}_16x24.png", sheet.size)


MOUSE = ["....kk....kk....",
         "...kppk..kppk...",
         "...kpgkkkkgpk...",
         "....kggggggk....",
         "...kggeggeggk...",
         "...kgggggggggk..",
         "....kggpgggggk..",
         ".....kkggggggk..",
         "......kggggggk..",
         ".....kgggkggk.kk",
         ".....kkk.kkk.kk.",
         "................"]
MOUSE_PAL = {"k": "#1a1a2e", "g": "#a8a8b0", "p": "#f4a8b8", "e": "#111111"}
CHICK = ["................",
         "......kkkk......",
         ".....kyyyyk.....",
         "....kyeyyeyk....",
         "....kyyooyyk....",
         "....kyyyyyyk....",
         "...kyyyyyyyyk...",
         "...kyyyyyyyyk...",
         "....kyyyyyyk....",
         ".....kkkkkk.....",
         "......o..o......",
         ".....oo..oo....."]
CHICK_PAL = {"k": "#1a1a2e", "y": "#f8d850", "o": "#f08a2a", "e": "#111111"}


OROL = ["................",
        "......kkkk......",
        ".....kwwwwk.....",
        "....kwwewwwk....",
        "....kwwwwyykk...",
        ".....kwwwyykk...",
        "....kkbbbkkk....",
        "...kbbbbbbbbk...",
        "..kbbBBbbBBbbk..",
        "..kbbBbbbbBbbk..",
        "..kbbbbbbbbbbk..",
        "...kbbbbbbbbk...",
        "....kbbbbbbk....",
        ".....kyyykyyk...",
        ".....kkk.kkk....",
        "................"]
OROL_PAL = {"k": "#1a1a2e", "w": "#f4f0e4", "b": "#8a5a30", "B": "#5a3a1c", "y": "#f4c030", "e": "#111111"}
BIRD = [".....kkkk.......",
        "....kbbbbk......",
        "...kbebbbbk.....",
        "..kyybbbbbbk....",
        "...kkbbbbbbbk...",
        "....kbrrbbbbk.k.",
        "....kbrrbbbbkkk.",
        ".....kbbbbbk....",
        "......kkkkk.....",
        ".......o.o......",
        "................",
        "................"]
BIRD_PAL = {"k": "#1a1a2e", "b": "#5a88c8", "r": "#e8604a", "y": "#f4c030", "e": "#111111", "o": "#f08a2a"}


def item_icons():
    """Icones 16 x 16 des objets (story/items.txt), fond transparent."""
    os.makedirs(os.path.join(ART, "items"), exist_ok=True)

    def new():
        im = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
        return im, ImageDraw.Draw(im)
    K = hexc("#1a1a2e")
    drawn = {}
    im, d = new()                                   # ticket de tram : carton jaune
    d.rectangle([2, 4, 13, 11], fill=hexc("#f4d03f"), outline=K)
    d.line([(5, 6), (10, 6)], fill=hexc("#8a6a1a"))
    d.line([(5, 8), (8, 8)], fill=hexc("#8a6a1a"))
    d.rectangle([11, 5, 12, 10], fill=hexc("#e8443a"))
    drawn["listok"] = im
    im, d = new()                                   # eau : petite bouteille
    d.rectangle([6, 1, 9, 3], fill=hexc("#3a6fd0"), outline=K)
    d.rectangle([4, 4, 11, 14], fill=hexc("#9cd4f4"), outline=K)
    d.rectangle([5, 8, 10, 13], fill=hexc("#4f9fd8"))
    drawn["voda"] = im
    im, d = new()                                   # goutte d'eau
    d.polygon([(8, 1), (13, 9), (11, 13), (8, 14), (5, 13), (3, 9)], fill=hexc("#4f9fd8"), outline=K)
    d.rectangle([6, 8, 7, 10], fill=hexc("#d6ecf8"))
    drawn["kvapka"] = im
    im, d = new()                                   # sac a dos
    d.rounded_rectangle([3, 3, 12, 14], 3, fill=hexc("#e0a020"), outline=K)
    d.rectangle([5, 8, 10, 12], fill=hexc("#c88a10"), outline=K)
    d.line([(6, 3), (6, 1), (9, 1), (9, 3)], fill=K)
    drawn["batoh"] = im
    im, d = new()                                   # carte pliee
    d.polygon([(1, 3), (5, 2), (10, 3), (14, 2), (14, 13), (10, 14), (5, 13), (1, 14)], fill=hexc("#f2e6c8"), outline=K)
    d.line([(5, 2), (5, 13)], fill=hexc("#b8a880"))
    d.line([(10, 3), (10, 14)], fill=hexc("#b8a880"))
    d.line([(2, 9), (7, 6), (12, 10)], fill=hexc("#e8443a"))
    drawn["mapa"] = im
    im, d = new()                                   # billets de train : deux cartons verts
    d.rectangle([1, 3, 11, 10], fill=hexc("#7fd6b0"), outline=K)
    d.rectangle([4, 6, 14, 13], fill=hexc("#5fb894"), outline=K)
    d.line([(6, 9), (12, 9)], fill=hexc("#2e6a4f"))
    drawn["listky"] = im
    im, d = new()                                   # bouteille d'eau (la gourde remplie au ruisseau)
    d.rectangle([6, 1, 9, 3], fill=hexc("#e8443a"), outline=K)
    d.rounded_rectangle([4, 4, 11, 14], 2, fill=hexc("#cfe8f4"), outline=K)
    d.rectangle([5, 7, 10, 13], fill=hexc("#4f9fd8"))
    d.line([(6, 5), (6, 6)], fill=hexc("#ffffff"))
    drawn["flasa"] = im
    im, d = new()                                   # veste
    d.polygon([(5, 2), (11, 2), (14, 5), (14, 13), (11, 13), (11, 6), (10, 14), (6, 14), (5, 6), (5, 13), (2, 13), (2, 5)],
              fill=hexc("#e8603a"), outline=K)
    d.line([(8, 3), (8, 14)], fill=K)
    d.rectangle([7, 2, 9, 3], fill=hexc("#f4d03f"))
    drawn["bunda"] = im
    for name, img in drawn.items():
        path = os.path.join(ART, "items", f"{name}.png")
        if os.path.exists(path):  # ne jamais ecraser un dessin deja fait (IA)
            continue
        img.save(path)
    print("ecrit art/items/ :", " ".join(n for n in drawn))


def portraits():
    """Portraits 48 x 48 depuis les tetes SVG (art/characters/fox-head.svg, rabbit-head.svg)."""
    from import_ai import crop_alpha, outline, quantize_rgba, shrink
    from svgpose import Renderer
    os.makedirs(os.path.join(ART, "portraits"), exist_ok=True)
    for name, svg in (("fox", "fox-head.svg"), ("andrea", "rabbit-head.svg")):
        head = crop_alpha(Image.fromarray(Renderer(os.path.join(ART, "characters", svg)).render(400)))
        side = max(head.width, head.height)
        sq = Image.new("RGBA", (side, side), (0, 0, 0, 0))
        sq.alpha_composite(head, ((side - head.width) // 2, side - head.height))
        img = outline(quantize_rgba(shrink(sq, 46, 46), 32))
        out = Image.new("RGBA", (48, 48), (0, 0, 0, 0))
        out.alpha_composite(img, (1, 1))
        out.save(os.path.join(ART, "portraits", f"{name}_48x48.png"))
        print(f"ecrit art/portraits/{name}_48x48.png")


if __name__ == "__main__":
    if "--new" in sys.argv:
        # ce qui manque seulement : les tuiles pas encore dessinees, les personnages et objets absents
        tiles(only_new=True)
        for name, rows, pal in (("orol", OROL, OROL_PAL), ("bird", BIRD, BIRD_PAL)):
            if not os.path.exists(os.path.join(ART, "walk", f"{name}_16x24.png")):
                creature(name, rows, pal)
        sys.exit(0)
    tiles()
    item_icons()
    portraits()
    character("fox", FOX, FOX_PAL, FOX_TAIL_DOWN)
    character("andrea", ANDREA, ANDREA_PAL)
    character("human", HUMAN, HUMAN_PAL)
    creature("mouse", MOUSE, MOUSE_PAL)
    creature("chick", CHICK, CHICK_PAL)
    creature("orol", OROL, OROL_PAL)
    creature("bird", BIRD, BIRD_PAL)
