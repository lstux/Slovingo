#!/usr/bin/env python3
"""Graphismes PROVISOIRES des niveaux 1-2 (Sucho, Košice dans la secheresse) et 1-4
(Búrka, l'orage dans les Tatras). Memes regles que make_placeholder_raj.py : un fichier
qui a deja une source IA dans art/sources/ n'est pas regenere (--force pour l'ecraser).

    art/sucho/{far,mid,tiles}.png   parc de Kosice, etang a sec, fontaine chantante
    art/tatry/{far,mid,tiles}.png   pierriers de granit, pins nains, lac, nuages d'orage
    art/objects/  tryska (jet de fontaine), zahradnik (jardinier), chatar (gardien),
                  kamen (pierre), chata (refuge), hory (decor de montagne)
"""
import math
import os
import random
import sys

from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(__file__))
from make_placeholder_art import hexc, lerp, outline  # noqa: E402
from make_placeholder_raj import save, spruce  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ART = os.path.join(ROOT, "art")
rnd = random.Random(7)


def sky(W, H, top, bot, until):
    im = Image.new("RGBA", (W, H))
    d = ImageDraw.Draw(im)
    for y in range(H):
        d.line([(0, y), (W, y)], fill=lerp(hexc(top), hexc(bot), min(1, y / until)))
    return im, d


def ridge(d, W, H, base, amp, col, seed, freq, sharp=False):
    r = random.Random(seed)
    ph = [r.random() * 6.28 for _ in range(3)]
    for x in range(W):
        t = 2 * math.pi * x / W
        v = 0.55 * math.sin(t * freq + ph[0]) + 0.3 * math.sin(t * freq * 2 + ph[1]) + 0.15 * math.sin(t * freq * 5 + ph[2])
        if sharp:
            v = abs(v) * 1.7 - 0.4
        d.line([(x, int(base - amp * v)), (x, H)], fill=col)


def tile_at(i):
    return (i % 8) * 16, (i // 8) * 16


def copy_tiles(im, src_theme, ids):
    src = Image.open(os.path.join(ART, src_theme, "tiles.png")).convert("RGBA")
    for i in ids:
        x, y = tile_at(i)
        im.paste(src.crop((x, y, x + 16, y + 16)), (x, y))


# ================================================================== SUCHO
def sucho_far():
    W, H = 480, 240
    im, d = sky(W, H, "#7fb3dc", "#f4e6b8", 190)   # ciel pale, brume de chaleur jaune
    d.ellipse([380, 18, 420, 58], fill=hexc("#fff3c0"))   # soleil
    d.ellipse([386, 24, 414, 52], fill=hexc("#fffbe6"))
    ridge(d, W, H, 182, 20, hexc("#c9b98e"), 4, 2)     # collines brulees
    x = 10
    while x < W - 40:   # sidliska dans la brume
        w, h = rnd.choice([24, 32, 40]), rnd.choice([28, 36, 46])
        if rnd.random() < 0.7:
            d.rectangle([x, 212 - h, x + w, 240], fill=hexc("#d8ccb0"))
            for wy in range(212 - h + 4, 210, 5):
                for wx in range(x + 3, x + w - 2, 4):
                    d.point((wx, wy), fill=hexc("#bfb196"))
        x += w + rnd.randint(8, 30)
    ridge(d, W, H, 230, 6, hexc("#a89a6a"), 5, 6)
    return im


def tree(d, x, base, h, leaves="#8a9a3a", dark="#6b7a2a"):
    d.rectangle([x - 2, base - h // 2, x + 1, base], fill=hexc("#6b4a30"))
    for (ox, oy, r) in [(0, -h + 10, h // 3), (-h // 4, -h + 18, h // 4), (h // 4, -h + 18, h // 4)]:
        d.ellipse([x + ox - r, base + oy - r, x + ox + r, base + oy + r], fill=hexc(leaves))
    d.ellipse([x - h // 5, base - h + 4, x + h // 8, base - h + 12], fill=hexc(dark))


def sucho_mid():
    W, H = 640, 148
    im = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    base = H - 1
    # parc : arbres aux feuilles jaunies par la secheresse
    for tx in range(10, 260, 26):
        tree(d, tx + rnd.randint(-4, 4), base, rnd.randint(50, 80), rnd.choice(["#9aa23e", "#b3a23a", "#8a9a3a"]))
    # theatre et fontaine chantante (Spievajuca fontana) au centre
    tx = 300
    c = hexc("#efe4c6")
    d.rectangle([tx, base - 70, tx + 110, base], fill=c)
    d.rectangle([tx - 2, base - 74, tx + 112, base - 70], fill=hexc("#d8c9a2"))
    for i in range(7):
        wx = tx + 8 + i * 15
        d.rectangle([wx, base - 58, wx + 7, base - 40], fill=hexc("#3b4a66"))
        d.pieslice([wx, base - 62, wx + 7, base - 54], 180, 360, fill=hexc("#3b4a66"))
        d.rectangle([wx + 1, base - 30, wx + 6, base - 12], fill=hexc("#3b4a66"))
    d.ellipse([tx + 35, base - 108, tx + 75, base - 66], fill=hexc("#4f9a7e"))
    d.rectangle([tx + 35, base - 88, tx + 75, base - 74], fill=hexc("#4f9a7e"))
    d.line([(tx + 55, base - 108), (tx + 55, base - 118)], fill=hexc("#d6a83a"))
    # bassin vide devant le theatre
    d.rectangle([tx - 10, base - 10, tx + 120, base], fill=hexc("#bdb3a2"))
    d.rectangle([tx - 6, base - 8, tx + 116, base - 2], fill=hexc("#8f8676"))
    for i in range(6):
        d.rectangle([tx + 6 + i * 20, base - 14, tx + 8 + i * 20, base - 8], fill=hexc("#6b737b"))
    # gare (zeleznicna stanica) a droite
    gx = 470
    d.rectangle([gx, base - 60, gx + 150, base], fill=hexc("#e8d9b0"))
    d.polygon([(gx - 4, base - 60), (gx + 20, base - 78), (gx + 130, base - 78), (gx + 154, base - 60)], fill=hexc("#8a3a22"))
    d.rectangle([gx + 60, base - 92, gx + 90, base - 60], fill=hexc("#e8d9b0"))
    d.ellipse([gx + 67, base - 88, gx + 83, base - 72], fill=hexc("#ffffff"))
    d.line([(gx + 75, base - 80), (gx + 75, base - 86)], fill=hexc("#222222"))
    d.line([(gx + 75, base - 80), (gx + 79, base - 80)], fill=hexc("#222222"))
    for i in range(8):
        wx = gx + 8 + i * 18
        d.rectangle([wx, base - 46, wx + 9, base - 20], fill=hexc("#3b4a66"))
    for tx2 in range(270, 290, 12):
        tree(d, tx2, base, 60, "#a6a03c")
    for tx2 in range(440, 470, 14):
        tree(d, tx2, base, 56, "#b3a23a")
    return im


def sucho_tiles():
    im = Image.new("RGBA", (128, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    earth, earth_d, crack = hexc("#c9a46a"), hexc("#a8824e"), hexc("#7a5a34")

    def dry(x, y, seed, top):
        r = random.Random(seed)
        d.rectangle([x, y, x + 15, y + 15], fill=earth)
        for _ in range(6):
            px, py = r.randint(0, 14), r.randint(0, 14)
            d.point((x + px, y + py), fill=earth_d)
        # craquelures de secheresse
        px = r.randint(3, 12)
        d.line([(x + px, y + 4), (x + px + 2, y + 9), (x + px - 1, y + 14)], fill=crack)
        if top:
            d.rectangle([x, y, x + 15, y + 2], fill=hexc("#b8a34a"))   # herbe jaunie
            for i in range(0, 16, 3):
                d.point((x + i, y), fill=hexc("#d8c46a"))
                d.point((x + i + 1, y + 3), fill=hexc("#9a8a3a"))

    for idx, top, edge in ((1, True, ""), (2, False, ""), (3, True, "L"), (4, True, "R"), (5, False, "L"), (6, False, "R")):
        x, y = tile_at(idx)
        dry(x, y, idx * 11, top)
        if "L" in edge:
            d.line([(x, y), (x, y + 15)], fill=crack)
        if "R" in edge:
            d.line([(x + 15, y), (x + 15, y + 15)], fill=crack)
    copy_tiles(im, "kosice", (7, 8, 9, 10, 11, 12, 13))   # bloc, caisses, corniche, pointes de Kosice
    # 14 echelle (pas utilisee) ; 15-17 eau boueuse de l'etang
    for idx, ph in ((15, 0), (17, 3)):
        x, y = tile_at(idx)
        d.rectangle([x, y + 3, x + 15, y + 15], fill=hexc("#6f8a6a"))
        for i in range(16):
            d.point((x + i, y + 2 + int(math.sin((i + ph) * 0.8))), fill=hexc("#b8c8a8"))
    x, y = tile_at(16)
    d.rectangle([x, y, x + 15, y + 15], fill=hexc("#4f6a4a"))
    # 18 sol fendu qui s'effondre, 19 le meme qui se fissure
    for idx, k in ((18, 0), (19, 1)):
        x, y = tile_at(idx)
        dry(x, y, 99, True)
        for (a, b) in (((2, 3), (7, 9)), ((7, 9), (5, 15)), ((7, 9), (13, 12)), ((10, 2), (13, 12))):
            d.line([(x + a[0], y + a[1]), (x + b[0], y + b[1])], fill=crack)
        if k:
            for (a, b) in (((1, 8), (6, 6)), ((12, 5), (15, 7))):
                d.line([(x + a[0], y + a[1]), (x + b[0], y + b[1])], fill=hexc("#3a2a1a"))
            d.line([(x, y + 15), (x + 15, y + 15)], fill=hexc("#3a2a1a"))
    return im


# ================================================================== TATRY
def tatry_far():
    W, H = 480, 240
    im, d = sky(W, H, "#3f4f68", "#9aa7b8", 170)   # ciel lourd, nuages d'orage
    for cx, cy, n in [(60, 40, 6), (240, 26, 8), (400, 50, 5)]:
        for i in range(n):
            r = rnd.randint(12, 20)
            d.ellipse([cx + i * 15 - r, cy - r + 6, cx + i * 15 + r, cy + r + 6], fill=hexc("#2e3a4c"))
            d.ellipse([cx + i * 15 - r, cy - r, cx + i * 15 + r, cy + r - 4], fill=hexc("#4a5870"))
    ridge(d, W, H, 150, 60, hexc("#5e6a78"), 9, 3, sharp=True)   # sommets de granit
    for x in range(W):
        for y in range(70, 150):
            if im.getpixel((x, y)) == hexc("#5e6a78") and im.getpixel((x, y - 3)) != hexc("#5e6a78"):
                d.point((x, y), fill=hexc("#c8d0d8"))
    ridge(d, W, H, 190, 26, hexc("#47525e"), 3, 4, sharp=True)
    ridge(d, W, H, 222, 10, hexc("#3a4a3e"), 6, 7)
    return im


def tatry_mid():
    W, H = 640, 160
    im = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    base = H - 1
    granite, gd, gl = hexc("#8c8f96"), hexc("#6a6d74"), hexc("#b0b3ba")
    x = 0
    while x < W:   # pentes et pierriers de granit
        w = rnd.choice([60, 80, 100])
        top = rnd.randint(20, 90)
        d.polygon([(x - 10, base), (x + w // 3, top), (x + w // 2, top + 6), (x + w + 10, base)], fill=granite)
        d.line([(x + w // 3, top), (x + w // 2, top + 6)], fill=gl)
        for _ in range(12):   # blocs eboules
            bx, by = rnd.randint(x, x + w), rnd.randint(top + 20, base)
            d.rectangle([bx, by, bx + rnd.randint(3, 7), by + rnd.randint(2, 5)], fill=gd)
        x += w
    # pins nains (kosodrevina) en bas
    for tx in range(0, W, 7):
        r = rnd.randint(5, 9)
        d.ellipse([tx - r, base - r - rnd.randint(2, 10), tx + r, base + 2], fill=hexc("#3c5a3a"))
    # lac (pleso) a droite
    d.rectangle([420, base - 8, 600, base], fill=hexc("#3f5f78"))
    d.line([(420, base - 8), (600, base - 8)], fill=hexc("#7f9fb8"))
    return im


def tatry_tiles():
    im = Image.new("RGBA", (128, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    g, gd, gl = hexc("#9a9da4"), hexc("#6f727a"), hexc("#c4c7cc")

    def granite(x, y, seed, top):
        r = random.Random(seed)
        d.rectangle([x, y, x + 15, y + 15], fill=g)
        for _ in range(8):
            px, py = r.randint(0, 14), r.randint(0, 14)
            d.point((x + px, y + py), fill=r.choice([gd, gl, hexc("#d8c8c8")]))
        d.line([(x + r.randint(2, 6), y + 15), (x + r.randint(8, 13), y + r.randint(6, 10))], fill=gd)
        if top:
            d.rectangle([x, y, x + 15, y + 2], fill=hexc("#5f7a4a"))   # herbe rase d'altitude
            for i in range(0, 16, 4):
                d.point((x + i + 1, y + 3), fill=hexc("#4a6a3a"))

    for idx, top, edge in ((1, True, ""), (2, False, ""), (3, True, "L"), (4, True, "R"), (5, False, "L"), (6, False, "R")):
        x, y = tile_at(idx)
        granite(x, y, idx * 5, top)
        if "L" in edge:
            d.line([(x, y), (x, y + 15)], fill=gd)
        if "R" in edge:
            d.line([(x + 15, y), (x + 15, y + 15)], fill=gd)
    x, y = tile_at(7)   # gros bloc de granit
    d.rectangle([x, y, x + 15, y + 15], fill=gd)
    d.rectangle([x + 1, y + 1, x + 14, y + 14], fill=g)
    d.line([(x + 1, y + 1), (x + 14, y + 1)], fill=gl)
    copy_tiles(im, "kosice", (8, 9))   # caisses bonus
    # 10-12 marches de bois (chemin balise), traversables par dessous
    for idx in (10, 11, 12):
        x, y = tile_at(idx)
        d.rectangle([x, y, x + 15, y + 4], fill=hexc("#8a6440"))
        d.line([(x, y), (x + 15, y)], fill=hexc("#b08a5a"))
        d.line([(x, y + 4), (x + 15, y + 4)], fill=hexc("#5a3e26"))
        if idx != 11:
            px = x + 1 if idx == 10 else x + 13
            d.rectangle([px, y + 4, px + 1, y + 12], fill=hexc("#5a3e26"))
    x, y = tile_at(13)   # pierres tranchantes
    for i in range(4):
        px = x + i * 4
        hh = rnd.randint(6, 11)
        d.polygon([(px, y + 15), (px + 2, y + 15 - hh), (px + 3, y + 15)], fill=gl)
        d.line([(px + 2, y + 15 - hh), (px + 3, y + 15)], fill=gd)
    x, y = tile_at(14)   # chaine (retaz) scellee dans le rocher
    for cy in range(y, y + 16, 4):
        d.ellipse([x + 5, cy, x + 10, cy + 4], outline=hexc("#c8ccd2"))
        d.point((x + 7, cy + 2), fill=hexc("#5a5e66"))
    d.point((x + 7, y), fill=hexc("#ffffff"))
    for idx, ph in ((15, 0), (17, 3)):   # lac de montagne (pleso), eau froide
        x, y = tile_at(idx)
        d.rectangle([x, y + 3, x + 15, y + 15], fill=hexc("#2f5f80"))
        for i in range(16):
            d.point((x + i, y + 2 + int(1.4 * math.sin((i + ph) * 0.8))), fill=hexc("#cfe6f5"))
    x, y = tile_at(16)
    d.rectangle([x, y, x + 15, y + 15], fill=hexc("#1f4560"))
    for idx, k in ((18, 0), (19, 1)):   # eboulis instable
        x, y = tile_at(idx)
        d.rectangle([x, y, x + 15, y + 15], fill=gd)
        r = random.Random(3)
        for _ in range(9):
            bx, by = r.randint(0, 12), r.randint(0, 12)
            d.rectangle([x + bx, y + by, x + bx + 3, y + by + 2], fill=gl if k == 0 else hexc("#a0a4aa"))
        d.line([(x, y), (x + 15, y)], fill=gl)
        if k:
            d.line([(x + 3, y + 2), (x + 8, y + 9), (x + 12, y + 4)], fill=hexc("#2a2c30"))
    return im


# ================================================================== objets
def obj_jet():
    """tryska : buse de la fontaine chantante. 0 eteint, 1 crachote, 2-3 jet plein (64 px)."""
    im = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for f in range(4):
        ox = f * 16
        d.rectangle([ox + 4, 58, ox + 11, 63], fill=hexc("#6b737b"))
        d.rectangle([ox + 5, 55, ox + 10, 58], fill=hexc("#8a9199"))
        d.line([(ox + 5, 55), (ox + 10, 55)], fill=hexc("#c3cad2"))
        if f == 1:
            for (px, py) in ((7, 52), (8, 50), (6, 49), (9, 53)):
                d.point((ox + px, py), fill=hexc("#9fd8f5"))
        if f >= 2:
            top = 0 if f == 2 else 2
            d.rectangle([ox + 5, top + 4, ox + 10, 55], fill=hexc("#9fd8f5"))
            d.rectangle([ox + 6, top + 4, ox + 8, 55], fill=hexc("#e8f6ff"))
            d.ellipse([ox + 2, top, ox + 13, top + 8], fill=hexc("#cfeeff"))
            for py in range(top + 10, 54, 6):
                d.point((ox + 4 + (py // 6 + f) % 2 * 7, py), fill=hexc("#ffffff"))
    return im


def person(d, ox, skin, fur, coat, hat=None, arm_up=False, beard=False):
    """petit personnage animal debout (32x48) : corps, tete, bras."""
    d.rectangle([ox + 11, 36, ox + 14, 46], fill=hexc("#3a3a46"))   # jambes
    d.rectangle([ox + 17, 36, ox + 20, 46], fill=hexc("#3a3a46"))
    d.rectangle([ox + 9, 45, ox + 15, 47], fill=hexc("#4a3020"))
    d.rectangle([ox + 16, 45, ox + 22, 47], fill=hexc("#4a3020"))
    d.rounded_rectangle([ox + 9, 20, ox + 22, 38], radius=3, fill=hexc(coat))
    d.ellipse([ox + 8, 6, ox + 23, 21], fill=hexc(fur))
    d.ellipse([ox + 12, 12, ox + 21, 21], fill=hexc(skin))
    d.point((ox + 13, 12), fill=hexc("#1a1a1a"))
    d.point((ox + 19, 12), fill=hexc("#1a1a1a"))
    d.ellipse([ox + 15, 15, ox + 17, 17], fill=hexc("#2a2020"))
    if beard:
        d.ellipse([ox + 11, 17, ox + 22, 24], fill=hexc("#d8d8d8"))
    if hat:
        d.rectangle([ox + 6, 6, ox + 25, 7], fill=hexc(hat))
        d.rectangle([ox + 10, 1, ox + 21, 6], fill=hexc(hat))
    if arm_up:
        d.line([(ox + 22, 22), (ox + 27, 12)], fill=hexc(coat), width=3)
        d.ellipse([ox + 25, 8, ox + 29, 12], fill=hexc(fur))
    else:
        d.line([(ox + 22, 22), (ox + 24, 33)], fill=hexc(coat), width=3)
    d.line([(ox + 9, 22), (ox + 7, 33)], fill=hexc(coat), width=3)


def obj_gardener():
    """zahradnik : vieux blaireau jardinier, salopette verte, chapeau de paille, arrosoir vide."""
    im = Image.new("RGBA", (64, 48), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for f in range(2):
        ox = f * 32
        person(d, ox, "#f0f0f0", "#6f6f78", "#4f7f46", hat="#e0c060", arm_up=f == 1)
        d.line([(ox + 15, 9), (ox + 15, 16)], fill=hexc("#f0f0f0"))   # raie blanche du blaireau
        # arrosoir
        d.rectangle([ox + 1, 34, ox + 7, 41], fill=hexc("#7d858e"))
        d.line([(ox + 7, 35), (ox + 11, 31)], fill=hexc("#7d858e"))
    return outline(im)


def obj_warden():
    """chatar : vieil ours, gardien du refuge, pull rouge a motifs, bonnet, barbe blanche."""
    im = Image.new("RGBA", (64, 48), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for f in range(2):
        ox = f * 32
        person(d, ox, "#c8a078", "#6a4a2e", "#b03a2e", hat="#2f4f7f", arm_up=f == 1, beard=True)
        for i in range(3):
            d.point((ox + 12 + i * 4, 27), fill=hexc("#f4f4f4"))
        d.ellipse([ox + 7, 5, ox + 11, 9], fill=hexc("#6a4a2e"))   # oreilles d'ours
        d.ellipse([ox + 20, 5, ox + 24, 9], fill=hexc("#6a4a2e"))
    return outline(im)


def obj_rock():
    """kamen : pierre qui tombe (2 images, elle tourne)."""
    im = Image.new("RGBA", (32, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for f in range(2):
        ox = f * 16
        pts = [(2, 6), (6, 2), (12, 3), (14, 9), (10, 14), (4, 13)]
        if f:
            pts = [(16 - x, 16 - y) for x, y in pts]
        d.polygon([(ox + x, y) for x, y in pts], fill=hexc("#8c8f96"))
        d.line([(ox + pts[0][0], pts[0][1]), (ox + pts[1][0], pts[1][1]), (ox + pts[2][0], pts[2][1])], fill=hexc("#c4c7cc"))
    return outline(im)


def obj_hut():
    """chata : refuge de montagne en pierre et bois, toit pentu, fenetre allumee."""
    im = Image.new("RGBA", (96, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([8, 30, 88, 63], fill=hexc("#8c8f96"))   # soubassement de pierre
    for yy in range(34, 63, 7):
        for xx in range(10 + (yy // 7 % 2) * 6, 86, 12):
            d.rectangle([xx, yy, xx + 9, yy + 5], outline=hexc("#6a6d74"))
    d.rectangle([14, 20, 82, 36], fill=hexc("#8a5a34"))   # etage en bois
    for xx in range(14, 82, 4):
        d.line([(xx, 20), (xx, 36)], fill=hexc("#6a4426"))
    d.polygon([(2, 24), (48, 0), (94, 24)], fill=hexc("#5a3a26"))
    d.polygon([(8, 22), (48, 3), (88, 22)], fill=hexc("#7a5236"))
    d.rectangle([66, 2, 72, 12], fill=hexc("#6a6d74"))   # cheminee
    d.rectangle([24, 42, 36, 52], fill=hexc("#f2c860"))   # fenetres allumees
    d.rectangle([60, 42, 72, 52], fill=hexc("#f2c860"))
    d.line([(30, 42), (30, 52)], fill=hexc("#5a3a26"))
    d.line([(66, 42), (66, 52)], fill=hexc("#5a3a26"))
    d.rectangle([42, 44, 54, 63], fill=hexc("#5a3a26"))   # porte
    d.rectangle([30, 23, 66, 31], fill=hexc("#e8dcc0"))   # enseigne (sans texte)
    return outline(im)


def obj_mountain_decor():
    """hory : pin nain (kosodrevina) et cairn de pierres (muzik)."""
    im = Image.new("RGBA", (64, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for (x, y, r) in [(8, 10, 7), (16, 8, 8), (24, 10, 7), (12, 5, 5), (20, 5, 5)]:
        d.ellipse([x - r, y - r, x + r, y + r + 2], fill=hexc("#3c5a3a"))
        d.ellipse([x - r + 2, y - r + 1, x + r - 3, y], fill=hexc("#557a4e"))
    ox = 32
    for (x, y, w, h) in [(8, 11, 16, 5), (10, 7, 12, 4), (12, 4, 8, 3), (14, 1, 4, 3)]:
        d.rounded_rectangle([ox + x, y, ox + x + w, y + h], radius=1, fill=hexc("#9a9da4"))
        d.line([(ox + x + 1, y), (ox + x + w - 1, y)], fill=hexc("#c4c7cc"))
    return outline(im)


if __name__ == "__main__":
    save(sucho_far(), "sucho/far.png")
    save(sucho_mid(), "sucho/mid.png")
    save(sucho_tiles(), "sucho/tiles.png")
    save(tatry_far(), "tatry/far.png")
    save(tatry_mid(), "tatry/mid.png")
    save(tatry_tiles(), "tatry/tiles.png")
    save(obj_jet(), "objects/tryska_16x64.png")
    save(obj_gardener(), "objects/zahradnik_32x48.png")
    save(obj_warden(), "objects/chatar_32x48.png")
    save(obj_rock(), "objects/kamen_16x16.png")
    save(obj_hut(), "objects/chata_96x64.png")
    save(obj_mountain_decor(), "objects/hory_32x16.png")
