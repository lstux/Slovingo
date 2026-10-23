#!/usr/bin/env python3
"""Graphismes PROVISOIRES du niveau 1-2, Slovenský raj (gorge de Suchá Belá).

Comme make_placeholder_art.py pour Košice : ils fixent les formats, les images Gemini
(art/PROMPTS.md) les remplacent fichier par fichier. Ce script n'ecrit que les fichiers du
Raj et ses nouveaux objets ; un fichier qui a deja une source IA dans art/sources/ n'est
pas regenere (--force pour l'ecraser).

    art/raj/far.png     480x240   ciel, collines boisees, Tatras au loin
    art/raj/mid.png     640x176   falaises calcaires, epiceas, cascade
    art/raj/tiles.png   128x64    tuiles 16x16 (memes numeros que Kosice + 14 echelle, 15-17 eau)
    art/objects/        platforma, flasa, vevericka, orol, rozcestnik, les
"""
import glob
import math
import os
import random
import sys

from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(__file__))
from make_placeholder_art import hexc, lerp, outline  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ART = os.path.join(ROOT, "art")
FORCE = "--force" in sys.argv
rnd = random.Random(42)


def save(img, rel):
    path = os.path.join(ART, rel)
    stem = os.path.basename(rel).split("_")[0].replace(".png", "")
    if not FORCE:
        folder = os.path.dirname(rel)
        key = stem if folder == "objects" else folder + "_" + stem
        if glob.glob(os.path.join(ART, "sources", key + "*")):
            print(f"{rel} : source IA presente, non regenere (--force pour l'ecraser)")
            return
    os.makedirs(os.path.dirname(path), exist_ok=True)
    img.save(path)
    print("ecrit", rel)


# ------------------------------------------------------------------ fond lointain
def far():
    W, H = 480, 240
    im = Image.new("RGBA", (W, H))
    d = ImageDraw.Draw(im)
    for y in range(H):
        d.line([(0, y), (W, y)], fill=lerp(hexc("#4a8fe0"), hexc("#d6ecf7"), min(1, y / 160)))
    for cx, cy, n in [(70, 34, 4), (300, 22, 5), (420, 52, 3)]:
        for i in range(n):
            r = rnd.randint(8, 12)
            d.ellipse([cx + i * 12 - r, cy - r, cx + i * 12 + r, cy + r], fill=hexc("#f7fbff"))

    def ridge(base, amp, col, seed, freq, sharp=False):
        r = random.Random(seed)
        ph = [r.random() * 6.28 for _ in range(3)]
        for x in range(W):
            t = 2 * math.pi * x / W
            v = 0.55 * math.sin(t * freq + ph[0]) + 0.3 * math.sin(t * freq * 2 + ph[1]) + 0.15 * math.sin(t * freq * 5 + ph[2])
            if sharp:
                v = abs(v) * 1.6 - 0.4
            d.line([(x, int(base - amp * v)), (x, H)], fill=col)
    # Vysoke Tatry au loin : sommets pointus bleutes, neige en haut
    ridge(150, 46, hexc("#9db4cc"), 7, 3, sharp=True)
    for x in range(W):
        for y in range(70, 150):
            if im.getpixel((x, y)) == hexc("#9db4cc") and im.getpixel((x, y - 4)) != hexc("#9db4cc"):
                d.point((x, y), fill=hexc("#eef4fa"))
                d.point((x, y + 1), fill=hexc("#dfe8f2"))
    ridge(178, 22, hexc("#6f9c7c"), 1, 2)
    ridge(200, 18, hexc("#4f8058"), 2, 3)
    # epiceas lointains sur la crete
    for x in range(0, W, 5):
        y = 200
        while y > 100 and im.getpixel((x, y - 1)) in (hexc("#4f8058"),):
            y -= 1
        h = rnd.randint(6, 10)
        d.polygon([(x - 2, y + 1), (x, y - h), (x + 2, y + 1)], fill=hexc("#3c6a46"))
    ridge(226, 8, hexc("#3c6a46"), 3, 6)
    return im


# ------------------------------------------------------------------ plan intermediaire
def spruce(d, x, base, h, col="#2f5d3a", dark="#24492d"):
    w = h // 2
    for k in range(4):
        yy = base - h + k * h // 4
        ww = w * (k + 1) // 4
        d.polygon([(x - ww, yy + h // 3), (x, yy), (x + ww, yy + h // 3)], fill=hexc(col))
        d.line([(x, yy), (x + ww, yy + h // 3)], fill=hexc(dark))
    d.rectangle([x - 1, base - 3, x + 1, base], fill=hexc("#5b4030"))


def mid():
    W, H = 640, 176
    im = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    base = H - 1
    stone, stone_d, stone_l = hexc("#c8c3b4"), hexc("#9d9786"), hexc("#e2ddd0")
    # falaises calcaires : colonnes de hauteur variable, fissures verticales
    x = 0
    while x < W:
        w = rnd.choice([30, 40, 52, 64])
        top = rnd.randint(30, 110)
        d.rectangle([x, top, x + w, base], fill=stone)
        for k in range(rnd.randint(2, 4)):
            fx = x + rnd.randint(3, w - 3)
            d.line([(fx, top + rnd.randint(4, 20)), (fx + rnd.randint(-2, 2), base)], fill=stone_d)
        for k in range(3):
            fy = rnd.randint(top + 8, base - 10)
            d.line([(x + 2, fy), (x + w - 3, fy + rnd.randint(-2, 2))], fill=stone_l)
        d.line([(x, top), (x + w, top)], fill=stone_l)
        # mousse et epiceas sur le dessus
        d.rectangle([x, top - 2, x + w, top], fill=hexc("#5e8a46"))
        for k in range(rnd.randint(1, 3)):
            spruce(d, x + rnd.randint(4, w - 4), top, rnd.randint(16, 30))
        x += w
    # cascade (Okienkovy vodopad) : bande blanche et bleue au milieu
    for cxw in (250, 520):
        for y in range(40, base):
            for i in range(10):
                if (y + i * 3 + cxw) % 7 < 4:
                    d.point((cxw + i, y), fill=hexc("#eaf6ff") if i % 3 else hexc("#9fd0f0"))
    # foret au pied des falaises
    for tx in range(0, W, 9):
        spruce(d, tx + rnd.randint(-2, 2), base, rnd.randint(22, 38), "#2c5636", "#20422a")
    return im


# ------------------------------------------------------------------ tuiles
def tiles():
    T = 16
    im = Image.new("RGBA", (8 * T, 4 * T), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    rock, rock_d, rock_l = hexc("#bdb7a6"), hexc("#8f8979"), hexc("#dcd7c9")

    def at(i):
        return (i % 8) * T, (i // 8) * T

    def limestone(x, y, seed):
        r = random.Random(seed)
        d.rectangle([x, y, x + 15, y + 15], fill=rock)
        for _ in range(5):
            px, py = r.randint(0, 13), r.randint(0, 13)
            d.line([(x + px, y + py), (x + px + r.randint(1, 3), y + py)], fill=rock_d)
        for _ in range(3):
            px, py = r.randint(0, 14), r.randint(0, 14)
            d.point((x + px, y + py), fill=rock_l)

    def grass_top(x, y):
        d.rectangle([x, y, x + 15, y + 4], fill=hexc("#4f7f3a"))
        d.line([(x, y), (x + 15, y)], fill=hexc("#7fb055"))
        for i in range(0, 16, 3):
            d.point((x + i, y + 5), fill=hexc("#4f7f3a"))
            d.point((x + i + 1, y - 1 if y > 0 else y), fill=hexc("#7fb055"))
        for i in range(1, 16, 5):
            d.point((x + i, y + 2), fill=hexc("#6d4c32"))  # aiguilles d'epicea

    for idx, top, edge in ((1, True, ""), (2, False, ""), (3, True, "L"), (4, True, "R"), (5, False, "L"), (6, False, "R")):
        x, y = at(idx)
        limestone(x, y, idx * 7)
        if top:
            grass_top(x, y)
        if "L" in edge:
            d.line([(x, y), (x, y + 15)], fill=rock_d)
        if "R" in edge:
            d.line([(x + 15, y), (x + 15, y + 15)], fill=rock_d)
    # bloc calcaire
    x, y = at(7)
    d.rectangle([x, y, x + 15, y + 15], fill=rock_d)
    d.rectangle([x + 1, y + 1, x + 14, y + 14], fill=rock)
    d.line([(x + 1, y + 1), (x + 14, y + 1)], fill=rock_l)
    d.line([(x + 3, y + 6), (x + 9, y + 7)], fill=rock_d)
    d.line([(x + 6, y + 10), (x + 13, y + 11)], fill=rock_d)
    # caisses bonus : reprises de Kosice
    ks = Image.open(os.path.join(ART, "kosice", "tiles.png")).convert("RGBA")
    for idx in (8, 9):
        x, y = at(idx)
        im.paste(ks.crop((x, y, x + 16, y + 16)), (x, y))
    # passerelle metallique (caillebotis) : traversable par dessous
    for idx, l, r in ((10, True, False), (11, False, False), (12, False, True)):
        x, y = at(idx)
        d.rectangle([x, y, x + 15, y + 3], fill=hexc("#7d858e"))
        d.line([(x, y), (x + 15, y)], fill=hexc("#c3cad2"))
        for gx in range(x + 1, x + 16, 3):
            d.line([(gx, y + 1), (gx, y + 2)], fill=hexc("#4c535b"))
        d.line([(x, y + 4), (x + 15, y + 4)], fill=hexc("#4c535b"))
        # garde-corps : chaine et poteaux
        for px in ((x + 1,) if l else ()) + ((x + 14,) if r else ()) + ((x + 7,) if not (l or r) else ()):
            d.line([(px, y - 0), (px, y + 3)], fill=hexc("#4c535b"))
        for gx in range(x, x + 16, 2):
            d.point((gx, y + 7 + (gx // 2) % 2), fill=hexc("#5f666e"))
    # cailloux pointus
    x, y = at(13)
    for i in range(4):
        px = x + i * 4
        h = rnd.randint(6, 10)
        d.polygon([(px, y + 15), (px + 2, y + 15 - h), (px + 3, y + 15)], fill=rock)
        d.line([(px + 2, y + 15 - h), (px + 3, y + 15)], fill=rock_d)
    # echelle (rebrik) : montants metalliques, barreaux
    x, y = at(14)
    for px in (x + 2, x + 13):
        d.rectangle([px, y, px + 1, y + 15], fill=hexc("#8a9199"))
        d.line([(px, y), (px, y + 15)], fill=hexc("#c3cad2"))
    for ry in (y + 2, y + 7, y + 12):
        d.rectangle([x + 2, ry, x + 14, ry + 1], fill=hexc("#6b737b"))
        d.line([(x + 3, ry), (x + 13, ry)], fill=hexc("#b0b8c0"))
    # eau : surface (2 images) et fond
    for idx, ph in ((15, 0), (17, 3)):
        x, y = at(idx)
        d.rectangle([x, y + 3, x + 15, y + 15], fill=hexc("#2f7fb0"))
        for i in range(16):
            yy = y + 2 + int(1.5 * math.sin((i + ph) * 0.8))
            d.point((x + i, yy), fill=hexc("#cfeeff"))
            d.line([(x + i, yy + 1), (x + i, y + 4)], fill=hexc("#5fb0dc"))
        for i in range(2):
            d.line([(x + 3 + i * 7 + ph, y + 8), (x + 6 + i * 7 + ph, y + 8)], fill=hexc("#5fb0dc"))
    x, y = at(16)
    d.rectangle([x, y, x + 15, y + 15], fill=hexc("#21618c"))
    d.line([(x + 2, y + 5), (x + 6, y + 5)], fill=hexc("#2f7fb0"))
    d.line([(x + 9, y + 11), (x + 13, y + 11)], fill=hexc("#2f7fb0"))
    return im


# ------------------------------------------------------------------ objets
def obj_platform():
    """passerelle mobile : caillebotis 48 px, chaines vers le haut (pont suspendu)."""
    im = Image.new("RGBA", (48, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([0, 0, 47, 5], fill=hexc("#7d858e"))
    d.line([(0, 0), (47, 0)], fill=hexc("#d0d6dc"))
    for gx in range(1, 48, 3):
        d.line([(gx, 1), (gx, 4)], fill=hexc("#4c535b"))
    d.rectangle([0, 6, 47, 7], fill=hexc("#4c535b"))
    for px in (2, 45):
        d.rectangle([px - 1, 6, px + 1, 12], fill=hexc("#4c535b"))
    d.line([(0, 0), (0, 7)], fill=hexc("#2a2e33"))
    d.line([(47, 0), (47, 7)], fill=hexc("#2a2e33"))
    return im


def obj_bottle():
    """flasa vody : bouteille bleue, bouchon, etiquette, goutte."""
    im = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([6, 1, 9, 3], fill=hexc("#2c6fbf"))
    d.rectangle([5, 4, 10, 14], fill=hexc("#9fd8f5"))
    d.rectangle([5, 8, 10, 11], fill=hexc("#e8f6ff"))
    d.line([(6, 9), (9, 9)], fill=hexc("#2c6fbf"))
    d.line([(6, 5), (6, 13)], fill=hexc("#d4f0ff"))
    return outline(im)


def obj_squirrel():
    """vevericka : ecureuil roux, queue en panache. assis, bond, aplati."""
    im = Image.new("RGBA", (48, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    fur, light, dark = hexc("#c4581f"), hexc("#f0c79a"), hexc("#8a3a14")
    for f in range(2):
        ox = f * 16
        dy = -2 if f else 0
        d.ellipse([ox + 1, 2 + dy, ox + 8, 13 + dy], fill=fur)          # queue
        d.ellipse([ox + 3, 4 + dy, ox + 7, 10 + dy], fill=dark)
        d.ellipse([ox + 6, 7 + dy, ox + 12, 14 + dy], fill=fur)         # corps
        d.ellipse([ox + 8, 9 + dy, ox + 11, 13 + dy], fill=light)
        d.ellipse([ox + 9, 4 + dy, ox + 14, 9 + dy], fill=fur)          # tete
        d.polygon([(ox + 10, 4 + dy), (ox + 11, 1 + dy), (ox + 12, 4 + dy)], fill=fur)
        d.point((ox + 12, 6 + dy), fill=hexc("#1a1a1a"))
        d.point((ox + 14, 7 + dy), fill=hexc("#3a2a20"))
        if f:
            d.line([(ox + 7, 13 + dy), (ox + 5, 15)], fill=dark)
            d.line([(ox + 11, 13 + dy), (ox + 13, 15)], fill=dark)
    ox = 32
    d.ellipse([ox + 1, 10, ox + 14, 15], fill=fur)
    d.ellipse([ox + 2, 9, ox + 6, 13], fill=dark)
    d.line([(ox + 10, 11), (ox + 12, 13)], fill=hexc("#1a1a1a"))
    d.line([(ox + 12, 11), (ox + 10, 13)], fill=hexc("#1a1a1a"))
    return outline(im)


def obj_eagle():
    """pan Orol, strazca parku : aigle brun au chapeau de garde, sur un rocher. 2 images."""
    im = Image.new("RGBA", (64, 48), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for f in range(2):
        ox = f * 32
        # rocher
        d.polygon([(ox + 1, 47), (ox + 4, 34), (ox + 12, 30), (ox + 24, 31), (ox + 30, 38), (ox + 31, 47)],
                  fill=hexc("#bdb7a6"))
        d.line([(ox + 4, 34), (ox + 12, 30), (ox + 24, 31)], fill=hexc("#dcd7c9"))
        d.line([(ox + 10, 40), (ox + 20, 42)], fill=hexc("#8f8979"))
        # corps
        d.ellipse([ox + 9, 14, ox + 23, 32], fill=hexc("#6b4626"))
        d.ellipse([ox + 12, 18, ox + 20, 30], fill=hexc("#8a5e34"))
        # aile (levee sur l'image 2 : il salue)
        if f:
            d.polygon([(ox + 21, 18), (ox + 30, 8), (ox + 29, 16), (ox + 23, 24)], fill=hexc("#5a3a1e"))
        else:
            d.polygon([(ox + 20, 18), (ox + 25, 22), (ox + 22, 31)], fill=hexc("#5a3a1e"))
        # tete blanche, bec jaune
        d.ellipse([ox + 11, 6, ox + 21, 16], fill=hexc("#f3efe6"))
        d.polygon([(ox + 20, 10), (ox + 25, 12), (ox + 20, 14)], fill=hexc("#f2b631"))
        d.point((ox + 18, 10), fill=hexc("#1a1a1a"))
        # chapeau de garde forestier vert
        d.rectangle([ox + 9, 5, ox + 23, 6], fill=hexc("#3f6b3a"))
        d.rectangle([ox + 12, 1, ox + 20, 5], fill=hexc("#4f7f46"))
        d.line([(ox + 12, 4), (ox + 20, 4)], fill=hexc("#c9a23a"))
        # serres
        d.line([(ox + 13, 32), (ox + 13, 33)], fill=hexc("#f2b631"))
        d.line([(ox + 19, 32), (ox + 19, 33)], fill=hexc("#f2b631"))
    return outline(im)


def obj_signpost():
    """rozcestnik : poteau de balisage slovaque, plaques jaunes, marque rouge-blanc-rouge.
    eteint (plaques ternes) / active (plaques vives, fanion)."""
    im = Image.new("RGBA", (32, 48), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for f in range(2):
        ox = f * 16
        d.rectangle([ox + 7, 6, ox + 8, 47], fill=hexc("#8a6a44"))
        d.line([(ox + 7, 6), (ox + 7, 47)], fill=hexc("#a8865a"))
        sign = hexc("#f2c531") if f else hexc("#b9a466")
        d.polygon([(ox + 1, 8), (ox + 12, 8), (ox + 15, 10), (ox + 12, 12), (ox + 1, 12)], fill=sign)
        d.polygon([(ox + 14, 15), (ox + 3, 15), (ox + 0, 17), (ox + 3, 19), (ox + 14, 19)], fill=sign)
        for yy, c in ((24, "#d63a3a"), (26, "#f4f4f4"), (28, "#d63a3a")):
            d.rectangle([ox + 6, yy, ox + 9, yy + 1], fill=hexc(c) if f else lerp(hexc(c), hexc("#888888"), 0.5))
        if f:
            d.polygon([(ox + 8, 0), (ox + 14, 2), (ox + 8, 4)], fill=hexc("#d63a3a"))
            d.line([(ox + 8, 0), (ox + 8, 6)], fill=hexc("#5a4a30"))
    return outline(im)


def obj_forest():
    """decor de foret : fougere (l), souche (k)."""
    im = Image.new("RGBA", (64, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for i in range(7):
        a = -1.2 + i * 0.4
        x0, y0 = 16, 15
        x1, y1 = x0 + int(14 * math.sin(a)), y0 - int(12 * math.cos(a) * 0.9)
        d.line([(x0, y0), (x1, y1)], fill=hexc("#4f8a3a"))
        for k in range(1, 5):
            px, py = x0 + (x1 - x0) * k // 5, y0 + (y1 - y0) * k // 5
            d.point((px - 1, py), fill=hexc("#6fae4e"))
            d.point((px + 1, py), fill=hexc("#6fae4e"))
    ox = 32
    d.rectangle([ox + 6, 6, ox + 25, 15], fill=hexc("#7a5232"))
    d.ellipse([ox + 6, 3, ox + 25, 9], fill=hexc("#c79a62"))
    d.ellipse([ox + 10, 5, ox + 21, 8], fill=hexc("#a8784a"))
    d.line([(ox + 10, 8), (ox + 10, 15)], fill=hexc("#5a3a20"))
    d.line([(ox + 18, 8), (ox + 18, 15)], fill=hexc("#5a3a20"))
    d.polygon([(ox + 2, 15), (ox + 6, 11), (ox + 6, 15)], fill=hexc("#7a5232"))
    d.ellipse([ox + 22, 11, ox + 27, 15], fill=hexc("#d8452e"))   # champignon
    d.point((ox + 24, 12), fill=hexc("#ffffff"))
    return outline(im)


if __name__ == "__main__":
    save(far(), "raj/far.png")
    save(mid(), "raj/mid.png")
    save(tiles(), "raj/tiles.png")
    save(obj_platform(), "objects/platforma_48x16.png")
    save(obj_bottle(), "objects/flasa_16x16.png")
    save(obj_squirrel(), "objects/vevericka_16x16.png")
    save(obj_eagle(), "objects/orol_32x48.png")
    save(obj_signpost(), "objects/rozcestnik_16x48.png")
    save(obj_forest(), "objects/les_32x16.png")
