#!/usr/bin/env python3
"""Graphismes PROVISOIRES du niveau Kosice, dessines pixel par pixel avec PIL.

Ils fixent les formats attendus par tools/build_assets.py ; les images generees par IA
(voir art/PROMPTS.md) les remplacent fichier par fichier, aux memes dimensions.

    art/kosice/far.png     fond lointain, 480x240, boucle a l'horizontale (ciel, collines)
    art/kosice/mid.png     plan intermediaire, 640x176, transparent en haut (rue Hlavna, Dom)
    art/kosice/tiles.png   tuiles 16x16, 8 par ligne (voir TILE_NAMES dans build_assets.py)
    art/objects/<nom>_<L>x<H>.png   objets animes, images cote a cote
"""
import math
import os
import random

from PIL import Image, ImageDraw

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ART = os.path.join(ROOT, "art")
rnd = random.Random(1234)


def hexc(s, a=255):
    s = s.lstrip("#")
    return tuple(int(s[i:i + 2], 16) for i in (0, 2, 4)) + (a,)


def lerp(c1, c2, t):
    return tuple(int(round(a + (b - a) * t)) for a, b in zip(c1, c2))


# ------------------------------------------------------------------ fond lointain
def far():
    W, H = 480, 240
    im = Image.new("RGBA", (W, H))
    d = ImageDraw.Draw(im)
    top, bot = hexc("#3d7fd6"), hexc("#bfe3f6")
    for y in range(H):
        t = min(1, y / 170)
        # bandes de couleur (dithering ordonne simple) : rendu « 16 bits »
        c = lerp(top, bot, t)
        d.line([(0, y), (W, y)], fill=c)
    # nuages : amas de cercles, ombre en bas
    for cx, cy, n in [(60, 40, 5), (230, 26, 6), (380, 58, 4), (150, 80, 3)]:
        for i in range(n):
            x = cx + i * 13 + rnd.randint(-3, 3)
            r = rnd.randint(9, 15)
            d.ellipse([x - r, cy - r + 3, x + r, cy + r + 3], fill=hexc("#a9c9e8"))
        for i in range(n):
            x = cx + i * 13
            r = rnd.randint(8, 13)
            d.ellipse([x - r, cy - r, x + r, cy + r], fill=hexc("#f7fbff"))
    # collines lointaines (monts Metaliferes slovaques), deux plans, bouclent sur W

    def ridge(base, amp, col, seed, freq):
        r = random.Random(seed)
        ph = [r.random() * 6.28 for _ in range(3)]
        for x in range(W):
            y = base - amp * (0.55 * math.sin(2 * math.pi * x / W * freq + ph[0])
                              + 0.3 * math.sin(2 * math.pi * x / W * freq * 2 + ph[1])
                              + 0.15 * math.sin(2 * math.pi * x / W * freq * 5 + ph[2]))
            d.line([(x, int(y)), (x, H)], fill=col)
    ridge(178, 26, hexc("#8fb3c9"), 1, 2)
    ridge(196, 18, hexc("#6f9a8a"), 2, 3)
    # sidliska (barres d'immeubles) au loin, gris bleute
    x = 8
    while x < W - 40:
        w = rnd.choice([22, 30, 38])
        h = rnd.choice([26, 34, 44])
        if rnd.random() < 0.7:
            d.rectangle([x, 214 - h, x + w, 240], fill=hexc("#a7b8c6"))
            for wy in range(214 - h + 4, 212, 5):
                for wx in range(x + 3, x + w - 2, 4):
                    d.point((wx, wy), fill=hexc("#8597a8"))
        x += w + rnd.randint(6, 30)
    ridge(232, 6, hexc("#5f8a5a"), 3, 6)
    return im


# ------------------------------------------------------------------ plan intermediaire
ROOF = hexc("#b5532f")
ROOF_D = hexc("#8a3a22")
PASTELS = ["#f2d27a", "#f3b7a8", "#bfe0c4", "#f6ead2", "#c9d7f2", "#f4c48a", "#e8e0a0"]


def house(d, x, base, w, h, col, roof_h=14, gable=False):
    c = hexc(col)
    dark = lerp(c, (60, 40, 30, 255), 0.25)
    d.rectangle([x, base - h, x + w - 1, base], fill=c)
    d.line([(x, base - h), (x, base)], fill=dark)
    # corniche
    d.rectangle([x - 1, base - h - 2, x + w, base - h], fill=lerp(c, (255, 255, 255, 255), 0.4))
    # toit
    if gable:
        d.polygon([(x - 1, base - h - 2), (x + w // 2, base - h - 2 - roof_h - 6),
                   (x + w, base - h - 2)], fill=c)
        d.polygon([(x + 3, base - h - 4), (x + w // 2, base - h - roof_h - 4), (x + w - 3, base - h - 4)],
                  fill=lerp(c, (255, 255, 255, 255), 0.25))
    else:
        d.polygon([(x - 1, base - h - 2), (x + 4, base - h - 2 - roof_h),
                   (x + w - 5, base - h - 2 - roof_h), (x + w, base - h - 2)], fill=ROOF)
        for yy in range(base - h - roof_h, base - h - 2, 3):
            d.line([(x + 2, yy), (x + w - 2, yy)], fill=ROOF_D)
        # lucarne
        if w > 30:
            lx = x + w // 2 - 3
            d.rectangle([lx, base - h - roof_h + 3, lx + 6, base - h - 3], fill=lerp(c, (255, 255, 255, 255), 0.3))
            d.rectangle([lx + 2, base - h - roof_h + 5, lx + 4, base - h - 4], fill=hexc("#3b4a66"))
    # fenetres : 2 etages, encadrements clairs
    floors = max(2, (h - 14) // 14)
    for f in range(floors):
        wy = base - h + 6 + f * 14
        n = max(2, (w - 4) // 9)
        for i in range(n):
            wx = x + 4 + i * ((w - 8) // n) + 1
            d.rectangle([wx - 1, wy - 1, wx + 4, wy + 7], fill=hexc("#fbf6ea"))
            d.rectangle([wx, wy, wx + 3, wy + 6], fill=hexc("#3b4a66"))
            d.point((wx + 1, wy + 1), fill=hexc("#8fb0d8"))
    # porte cochere au rez-de-chaussee
    px = x + w // 2 - 4
    d.rectangle([px, base - 11, px + 8, base], fill=hexc("#5a3a2a"))
    d.pieslice([px, base - 15, px + 8, base - 7], 180, 360, fill=hexc("#5a3a2a"))


def cathedral(d, x, base):
    """Dom svatej Alzbety : gothique, pierre grise, toit en tuiles vernissees a losanges,
    grande tour nord a fleche sombre ornee d'or, rosace."""
    stone = hexc("#9a8f84")
    stone_l = hexc("#b9afa3")
    stone_d = hexc("#6e655d")
    # nef
    d.rectangle([x, base - 70, x + 110, base], fill=stone)
    for bx in range(x + 6, x + 110, 18):  # contreforts
        d.rectangle([bx, base - 74, bx + 4, base], fill=stone_d)
        d.polygon([(bx, base - 74), (bx + 2, base - 82), (bx + 4, base - 74)], fill=stone_d)
    for bx in range(x + 13, x + 105, 18):  # baies gothiques
        d.rectangle([bx, base - 58, bx + 6, base - 20], fill=hexc("#3c4e6e"))
        d.polygon([(bx, base - 58), (bx + 3, base - 64), (bx + 6, base - 58)], fill=hexc("#3c4e6e"))
        d.line([(bx + 3, base - 60), (bx + 3, base - 21)], fill=stone_l)
    # toit a losanges vernisses (vert, jaune, rouge)
    rt = base - 70
    d.polygon([(x - 2, rt), (x + 14, rt - 34), (x + 96, rt - 34), (x + 112, rt)], fill=hexc("#2f6b4f"))
    cols = [hexc("#d9b43b"), hexc("#b33a2c"), hexc("#3f8a5f")]
    for yy in range(rt - 32, rt, 4):
        frac = (yy - (rt - 34)) / 34
        x0 = int(x + 14 - 16 * frac)
        x1 = int(x + 96 + 16 * frac)
        for xx in range(x0 + ((yy // 4) % 2) * 3, x1, 6):
            d.point((xx, yy), fill=cols[(xx // 6 + yy // 4) % 3])
            d.point((xx + 1, yy + 1), fill=cols[(xx // 6 + yy // 4) % 3])
    # pignon ouest avec rosace
    gx = x + 110
    d.rectangle([gx, base - 92, gx + 30, base], fill=stone_l)
    d.polygon([(gx, base - 92), (gx + 15, base - 120), (gx + 30, base - 92)], fill=stone_l)
    d.ellipse([gx + 8, base - 84, gx + 22, base - 70], fill=hexc("#3c4e6e"))
    d.ellipse([gx + 12, base - 80, gx + 18, base - 74], fill=hexc("#c75d4a"))
    d.rectangle([gx + 10, base - 30, gx + 20, base], fill=hexc("#4a3a30"))
    d.polygon([(gx + 10, base - 30), (gx + 15, base - 40), (gx + 20, base - 30)], fill=hexc("#4a3a30"))
    # grande tour nord (a gauche), 4 niveaux puis fleche
    tx = x - 30
    d.rectangle([tx, base - 118, tx + 30, base], fill=stone)
    d.line([(tx, base - 118), (tx, base)], fill=stone_d)
    for lv in range(4):
        ly = base - 112 + lv * 26
        d.rectangle([tx + 10, ly + 4, tx + 20, ly + 18], fill=hexc("#3c4e6e"))
        d.polygon([(tx + 10, ly + 4), (tx + 15, ly - 2), (tx + 20, ly + 4)], fill=hexc("#3c4e6e"))
        d.line([(tx - 1, ly + 22), (tx + 31, ly + 22)], fill=stone_l)
    # galerie + clochetons + fleche baroque sombre
    d.rectangle([tx - 3, base - 124, tx + 33, base - 118], fill=stone_l)
    for px_ in (tx - 3, tx + 30):
        d.polygon([(px_, base - 124), (px_ + 3, base - 136), (px_ + 6, base - 124)], fill=stone_d)
    d.rectangle([tx + 6, base - 140, tx + 24, base - 124], fill=hexc("#3a3f3a"))
    d.ellipse([tx + 4, base - 152, tx + 26, base - 132], fill=hexc("#3a3f3a"))
    d.rectangle([tx + 10, base - 162, tx + 20, base - 146], fill=hexc("#3a3f3a"))
    d.ellipse([tx + 8, base - 168, tx + 22, base - 154], fill=hexc("#3a3f3a"))
    d.polygon([(tx + 12, base - 164), (tx + 15, base - 176), (tx + 18, base - 164)], fill=hexc("#3a3f3a"))
    for yy in (base - 140, base - 150, base - 160):
        d.line([(tx + 8, yy), (tx + 22, yy)], fill=hexc("#d6a83a"))
    d.rectangle([tx + 14, base - 176, tx + 16, base - 172], fill=hexc("#d6a83a"))  # croix doree
    d.line([(tx + 13, base - 175), (tx + 17, base - 175)], fill=hexc("#d6a83a"))
    # tour sud (Matej), plus courte, toit pyramidal
    sx = x + 80
    d.rectangle([sx, base - 96, sx + 20, base - 70], fill=stone)
    d.polygon([(sx - 2, base - 96), (sx + 10, base - 116), (sx + 22, base - 96)], fill=hexc("#2f6b4f"))


def theatre(d, x, base):
    """Theatre d'Etat : neo-baroque, facade claire, coupole verte."""
    c = hexc("#efe4c6")
    d.rectangle([x, base - 56, x + 70, base], fill=c)
    d.rectangle([x - 2, base - 60, x + 72, base - 56], fill=hexc("#d8c9a2"))
    for i in range(5):
        wx = x + 6 + i * 13
        d.rectangle([wx, base - 46, wx + 6, base - 30], fill=hexc("#3b4a66"))
        d.pieslice([wx, base - 50, wx + 6, base - 42], 180, 360, fill=hexc("#3b4a66"))
        d.rectangle([wx + 1, base - 22, wx + 5, base - 8], fill=hexc("#3b4a66"))
    d.ellipse([x + 20, base - 88, x + 50, base - 52], fill=hexc("#4f9a7e"))
    d.rectangle([x + 20, base - 70, x + 50, base - 58], fill=hexc("#4f9a7e"))
    d.line([(x + 35, base - 88), (x + 35, base - 96)], fill=hexc("#d6a83a"))
    d.ellipse([x + 33, base - 99, x + 37, base - 95], fill=hexc("#d6a83a"))
    for i in range(4):
        d.line([(x + 24 + i * 7, base - 84), (x + 24 + i * 7, base - 60)], fill=hexc("#3f8068"))


def mid():
    W, H = 640, 176
    im = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    base = H - 1
    x = 0
    # maisons bourgeoises de la rue Hlavna, toits de tuiles
    plan = [(44, 62, 0), (36, 54, 1), (40, 70, 2), (34, 58, 3), (46, 66, 4)]
    for w, h, c in plan:
        house(d, x, base, w, h, PASTELS[c % len(PASTELS)], gable=(c == 2))
        x += w
    theatre(d, x + 6, base)
    x += 84
    for w, h, c in [(38, 60, 5), (40, 52, 6)]:
        house(d, x, base, w, h, PASTELS[c % len(PASTELS)])
        x += w
    cathedral(d, x + 40, base)
    x += 40 + 152
    # Urbanova veza (tour Urban), puis encore des maisons jusqu'au bord (raccord)
    d.rectangle([x + 4, base - 84, x + 20, base], fill=hexc("#d9cdb6"))
    d.polygon([(x + 2, base - 84), (x + 12, base - 104), (x + 22, base - 84)], fill=hexc("#7a3b2a"))
    for ly in range(base - 76, base - 10, 16):
        d.rectangle([x + 10, ly, x + 14, ly + 7], fill=hexc("#3b4a66"))
    x += 26
    i = 0
    while x < W:
        w = [38, 42, 36, 44][i % 4]
        house(d, x, base, min(w, W - x), [58, 66, 54, 62][i % 4], PASTELS[(i + 1) % len(PASTELS)],
              gable=(i % 3 == 1))
        x += w
        i += 1
    # arbres d'alignement (tilleuls), devant les facades
    for tx in range(18, W, 74):
        d.rectangle([tx + 6, base - 18, tx + 8, base], fill=hexc("#5b4030"))
        for (ox, oy, r) in [(0, -26, 9), (7, -32, 10), (14, -26, 9), (7, -22, 9)]:
            d.ellipse([tx + ox - r + 7, base + oy - r, tx + ox + r + 7 - 7, base + oy + r], fill=hexc("#3f7d3a"))
        for (ox, oy, r) in [(3, -30, 5), (10, -34, 5)]:
            d.ellipse([tx + ox, base + oy - r, tx + ox + r + 3, base + oy + r - 2], fill=hexc("#5e9e45"))
    return im


# ------------------------------------------------------------------ tuiles
def tiles():
    T = 16
    im = Image.new("RGBA", (8 * T, 4 * T), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    def at(i):
        return (i % 8) * T, (i // 8) * T

    stone = [hexc("#b8ada0"), hexc("#a99d90"), hexc("#c7bdb1"), hexc("#9d9184")]
    mortar = hexc("#6f655c")
    earth = hexc("#7b6a5a")
    earth_d = hexc("#5f5044")

    def cobbles(x, y, top):
        d.rectangle([x, y, x + 15, y + 15], fill=mortar)
        r = random.Random(x * 31 + y)
        rows = [(0, 4), (4, 4), (8, 4), (12, 4)]
        for ri, (oy, h) in enumerate(rows):
            off = 0 if ri % 2 == 0 else 3
            xx = -off
            while xx < 16:
                w = r.choice([4, 5, 6])
                x0, x1 = max(0, xx), min(15, xx + w - 2)
                if x1 > x0:
                    d.rectangle([x + x0, y + oy, x + x1, y + oy + h - 2], fill=r.choice(stone))
                    d.point((x + x0, y + oy), fill=hexc("#ddd4c8"))
                xx += w
        if top:
            d.rectangle([x, y, x + 15, y + 2], fill=hexc("#8d8276"))  # bordure (obrubnik)
            d.line([(x, y), (x + 15, y)], fill=hexc("#d8cfc3"))

    def fill(x, y):
        d.rectangle([x, y, x + 15, y + 15], fill=earth)
        r = random.Random(x * 7 + y * 13)
        for _ in range(9):
            px, py = r.randint(0, 14), r.randint(0, 14)
            d.rectangle([x + px, y + py, x + px + 1, y + py], fill=earth_d)
        for _ in range(3):
            px, py = r.randint(0, 13), r.randint(0, 13)
            d.rectangle([x + px, y + py, x + px + 2, y + py + 1], fill=hexc("#948372"))

    # 0 vide ; 1 sol dessus ; 2 sol plein ; 3 dessus bord gauche ; 4 dessus bord droit
    # 5 plein bord gauche ; 6 plein bord droit ; 7 bloc de pierre ; 8 bloc bonus ; 9 bonus vide
    # 10/11/12 corniche (gauche, milieu, droite) traversable par dessous ; 13 pointes
    x, y = at(1)
    cobbles(x, y, True)
    x, y = at(2)
    fill(x, y)
    x, y = at(3)
    cobbles(x, y, True)
    d.line([(x, y), (x, y + 15)], fill=mortar)
    d.line([(x, y + 1), (x + 1, y + 1)], fill=(0, 0, 0, 0))
    x, y = at(4)
    cobbles(x, y, True)
    d.line([(x + 15, y), (x + 15, y + 15)], fill=mortar)
    x, y = at(5)
    fill(x, y)
    d.line([(x, y), (x, y + 15)], fill=earth_d)
    x, y = at(6)
    fill(x, y)
    d.line([(x + 15, y), (x + 15, y + 15)], fill=earth_d)
    # bloc de pierre taillee (comme les soubassements de la rue Hlavna)
    x, y = at(7)
    d.rectangle([x, y, x + 15, y + 15], fill=hexc("#8e8478"))
    d.rectangle([x + 1, y + 1, x + 14, y + 14], fill=hexc("#b3a898"))
    d.line([(x + 1, y + 1), (x + 14, y + 1)], fill=hexc("#d6ccbe"))
    d.line([(x + 1, y + 1), (x + 1, y + 14)], fill=hexc("#d6ccbe"))
    d.line([(x + 1, y + 7), (x + 14, y + 7)], fill=hexc("#8e8478"))
    d.line([(x + 8, y + 1), (x + 8, y + 7)], fill=hexc("#8e8478"))
    d.line([(x + 4, y + 8), (x + 4, y + 14)], fill=hexc("#8e8478"))
    d.line([(x + 12, y + 8), (x + 12, y + 14)], fill=hexc("#8e8478"))
    # bloc bonus : caisse de bois decoree d'un coeur en pain d'epices (medovnik)
    for idx, used in ((8, False), (9, True)):
        x, y = at(idx)
        base_c = hexc("#c98a3c") if not used else hexc("#8a7a66")
        d.rectangle([x, y, x + 15, y + 15], fill=hexc("#5a3a20"))
        d.rectangle([x + 1, y + 1, x + 14, y + 14], fill=base_c)
        d.line([(x + 1, y + 1), (x + 14, y + 1)], fill=lerp(base_c, (255, 255, 255, 255), 0.4))
        for (px, py) in ((2, 2), (13, 2), (2, 13), (13, 13)):
            d.point((x + px, y + py), fill=hexc("#5a3a20"))
        if not used:
            heart(d, x + 4, y + 4, hexc("#b5442e"), hexc("#f6e7c8"), small=True)
    # corniche / balcon (traversable par dessous)
    for idx, (l, r) in zip((10, 11, 12), ((True, False), (False, False), (False, True))):
        x, y = at(idx)
        d.rectangle([x, y, x + 15, y + 4], fill=hexc("#e6dcc8"))
        d.line([(x, y), (x + 15, y)], fill=hexc("#fffaf0"))
        d.rectangle([x, y + 5, x + 15, y + 6], fill=hexc("#a89a84"))
        for bx in range(x + 2, x + 16, 4):
            d.rectangle([bx, y + 7, bx + 1, y + 10], fill=hexc("#a89a84"))
        if l:
            d.line([(x, y), (x, y + 6)], fill=hexc("#a89a84"))
        if r:
            d.line([(x + 15, y), (x + 15, y + 6)], fill=hexc("#a89a84"))
    # pointes : barriere de chantier hérissee
    x, y = at(13)
    for i in range(4):
        px = x + i * 4
        d.polygon([(px, y + 15), (px + 2, y + 6), (px + 3, y + 15)], fill=hexc("#cfd6de"))
        d.line([(px + 2, y + 6), (px + 2, y + 15)], fill=hexc("#8994a0"))
    d.rectangle([x, y + 13, x + 15, y + 15], fill=hexc("#e0a020"))
    for i in range(0, 16, 4):
        d.rectangle([x + i, y + 13, x + i + 1, y + 15], fill=hexc("#2a2a2a"))
    return im


def heart(d, x, y, col, icing, small=False):
    """Coeur en pain d'epices (medovnik) : 8x7, glacage blanc."""
    pts = [".XX.XX..", "XXXXXXX.", "XXXXXXX.", ".XXXXX..", "..XXX...", "...X...."]
    for j, row in enumerate(pts):
        for i, ch in enumerate(row):
            if ch == "X":
                d.point((x + i, y + j), fill=col)
    for (i, j) in ((1, 1), (2, 2), (4, 2), (5, 1), (3, 3)):
        d.point((x + i, y + j), fill=icing)


# ------------------------------------------------------------------ objets
def obj_heart():
    """medovnik qui tourne : 4 images 16x16."""
    im = Image.new("RGBA", (64, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    widths = [14, 10, 4, 10]
    shape = ["..XXX...XXX...", ".XXXXX.XXXXX..", "XXXXXXXXXXXXX.", "XXXXXXXXXXXXX.", "XXXXXXXXXXXXX.",
             ".XXXXXXXXXXX..", "..XXXXXXXXX...", "...XXXXXXX....", "....XXXXX.....", ".....XXX......",
             "......X......."]
    for f, w in enumerate(widths):
        ox = f * 16 + (16 - w) // 2
        for j, row in enumerate(shape):
            for i, ch in enumerate(row[:13]):
                if ch != "X":
                    continue
                sx = ox + int(i * w / 13)
                c = hexc("#b5442e") if f != 2 else hexc("#8a3020")
                d.point((sx, j + 2), fill=c)
        if w >= 10:
            # glacage : festons blancs + points de couleur
            for i in range(1, 12):
                yy = 4 + (i % 2)
                d.point((ox + int(i * w / 13), yy), fill=hexc("#f6e7c8"))
            d.point((ox + int(6 * w / 13), 7), fill=hexc("#3fae5a"))
            d.point((ox + int(4 * w / 13), 8), fill=hexc("#f2c43a"))
            d.point((ox + int(8 * w / 13), 8), fill=hexc("#f2c43a"))
        # contour sombre
    return outline(im)


def outline(im, col=(40, 24, 20, 255)):
    px = im.load()
    W, H = im.size
    out = im.copy()
    po = out.load()
    for y in range(H):
        for x in range(W):
            if px[x, y][3] == 0:
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    xx, yy = x + dx, y + dy
                    if 0 <= xx < W and 0 <= yy < H and px[xx, yy][3] > 0 and (xx // 16 == x // 16 or W % 16):
                        po[x, y] = col
                        break
    return out


def obj_pigeon():
    """holub : 3 images 24x16 (marche x2, ecrase)."""
    im = Image.new("RGBA", (72, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    body, wing, neck = hexc("#8c94a3"), hexc("#6c7484"), hexc("#5d8a7a")
    for f in range(2):
        ox = f * 24
        d.ellipse([ox + 4, 5, ox + 17, 13], fill=body)
        d.polygon([(ox + 3, 8), (ox + 0, 6), (ox + 1, 11)], fill=wing)  # queue
        d.ellipse([ox + 7, 6, ox + 15, 11], fill=wing)
        d.line([(ox + 9, 8), (ox + 14, 8)], fill=hexc("#4c5363"))
        d.ellipse([ox + 14, 2, ox + 20, 8], fill=neck)
        d.ellipse([ox + 15, 1, ox + 20, 6], fill=body)
        d.point((ox + 18, 3), fill=hexc("#f0a020"))
        d.point((ox + 18, 2), fill=hexc("#101010"))
        d.polygon([(ox + 20, 3), (ox + 23, 4), (ox + 20, 5)], fill=hexc("#d9a46a"))
        lx = [(9, 12), (11, 10)][f]
        for l in lx:
            d.line([(ox + l, 13), (ox + l, 15)], fill=hexc("#d0605a"))
            d.point((ox + l + 1, 15), fill=hexc("#d0605a"))
    ox = 48
    d.ellipse([ox + 2, 10, ox + 21, 15], fill=body)
    d.ellipse([ox + 16, 9, ox + 22, 14], fill=neck)
    d.line([(ox + 18, 11), (ox + 20, 13)], fill=hexc("#101010"))
    d.line([(ox + 20, 11), (ox + 18, 13)], fill=hexc("#101010"))
    return outline(im)


def obj_wasp():
    """osa : 2 images 16x16."""
    im = Image.new("RGBA", (32, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for f in range(2):
        ox = f * 16
        wy = [3, 6][f]
        d.ellipse([ox + 4, wy - 3, ox + 10, wy + 2], fill=hexc("#dfefff"))
        d.ellipse([ox + 2, 6, ox + 13, 12], fill=hexc("#f2c22a"))
        for sx in (5, 8, 11):
            d.line([(ox + sx, 6), (ox + sx, 12)], fill=hexc("#2a2420"))
        d.ellipse([ox + 11, 5, ox + 15, 10], fill=hexc("#2a2420"))
        d.point((ox + 13, 7), fill=hexc("#ffffff"))
        d.polygon([(ox + 2, 9), (ox + 0, 10), (ox + 2, 11)], fill=hexc("#2a2420"))
    return outline(im)


def obj_spring():
    """pruzina : 2 images 16x16 (detendue, comprimee)."""
    im = Image.new("RGBA", (32, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for f, top in enumerate((2, 8)):
        ox = f * 16
        d.rectangle([ox + 1, 13, ox + 14, 15], fill=hexc("#5a5f6a"))
        n = 3
        for i in range(n):
            yy = 13 - (i + 1) * (13 - top - 2) // (n + 1)
            d.line([(ox + 3, yy), (ox + 12, yy - 1)], fill=hexc("#c7ccd6"))
        d.rectangle([ox + 1, top, ox + 14, top + 2], fill=hexc("#d84a3a"))
        d.line([(ox + 1, top), (ox + 14, top)], fill=hexc("#f08a7a"))
    return outline(im)


def obj_checkpoint():
    """lampadaire de la rue Hlavna, eteint / allume : 2 images 16x48."""
    im = Image.new("RGBA", (32, 48), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for f in range(2):
        ox = f * 16
        d.rectangle([ox + 7, 10, ox + 8, 45], fill=hexc("#2f3a36"))
        d.rectangle([ox + 5, 44, ox + 10, 47], fill=hexc("#2f3a36"))
        d.rectangle([ox + 6, 30, ox + 9, 32], fill=hexc("#2f3a36"))
        glass = hexc("#ffe58a") if f else hexc("#7d8a8a")
        d.polygon([(ox + 3, 4), (ox + 12, 4), (ox + 11, 11), (ox + 4, 11)], fill=glass)
        d.rectangle([ox + 2, 2, ox + 13, 4], fill=hexc("#2f3a36"))
        d.polygon([(ox + 5, 2), (ox + 7, 0), (ox + 8, 0), (ox + 10, 2)], fill=hexc("#2f3a36"))
        d.rectangle([ox + 3, 11, ox + 12, 12], fill=hexc("#2f3a36"))
        if f:
            for (px, py) in ((1, 6), (14, 6), (7, 14)):
                d.point((ox + px, py), fill=hexc("#fff4c0"))
    return im


def obj_stop():
    """zastavka (arret de tram) : panneau ovale jaune « Z » sur poteau, 16x48."""
    im = Image.new("RGBA", (16, 48), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([7, 12, 8, 47], fill=hexc("#7a8088"))
    d.ellipse([1, 0, 14, 14], fill=hexc("#2c4a8c"))
    d.ellipse([2, 1, 13, 13], fill=hexc("#f4d23c"))
    for (a, b) in (((5, 4), (10, 4)), ((10, 4), (5, 10)), ((5, 10), (10, 10))):
        d.line([a, b], fill=hexc("#1c1c1c"))
    d.rectangle([3, 18, 12, 26], fill=hexc("#ffffff"))
    for yy in (20, 22, 24):
        d.line([(4, yy), (11, yy)], fill=hexc("#5a6070"))
    return im


def obj_tram():
    """elektricka : Tatra T3 creme et rouge (comme a Kosice), 96x48, avec pantographe."""
    im = Image.new("RGBA", (96, 48), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    cream, red, dark = hexc("#f1e3c2"), hexc("#c8352b"), hexc("#26324a")
    # pantographe
    d.line([(40, 10), (48, 2)], fill=hexc("#3a3a3a"))
    d.line([(56, 10), (48, 2)], fill=hexc("#3a3a3a"))
    d.line([(42, 1), (54, 1)], fill=hexc("#3a3a3a"))
    # caisse aux bouts arrondis
    d.rounded_rectangle([1, 10, 94, 42], radius=7, fill=cream)
    d.rectangle([1, 30, 94, 40], fill=red)
    d.rounded_rectangle([1, 30, 94, 42], radius=6, fill=red)
    d.rectangle([4, 10, 91, 12], fill=hexc("#e2d2ad"))
    # fenetres
    for wx in range(10, 86, 13):
        d.rectangle([wx, 15, wx + 9, 26], fill=dark)
        d.line([(wx + 1, 16), (wx + 4, 16)], fill=hexc("#7f9cc8"))
    d.rounded_rectangle([1, 15, 7, 27], radius=3, fill=dark)
    d.rounded_rectangle([88, 15, 94, 27], radius=3, fill=dark)
    # portes
    for px in (22, 62):
        d.rectangle([px, 15, px + 9, 40], fill=hexc("#b02c24"))
        d.rectangle([px + 1, 16, px + 8, 26], fill=dark)
        d.line([(px + 4, 16), (px + 4, 40)], fill=hexc("#7a1c18"))
    # phares, numero, bogies
    d.point((3, 33), fill=hexc("#fff6c0"))
    d.point((92, 33), fill=hexc("#fff6c0"))
    d.rectangle([40, 32, 54, 36], fill=hexc("#f1e3c2"))
    d.rectangle([12, 42, 30, 46], fill=hexc("#2a2a2a"))
    d.rectangle([66, 42, 84, 46], fill=hexc("#2a2a2a"))
    for wx in (15, 25, 69, 79):
        d.ellipse([wx - 2, 43, wx + 2, 47], fill=hexc("#555555"))
    return outline(im)


def obj_bench():
    """lavicka 32x16 + bac a fleurs 32x16."""
    im = Image.new("RGBA", (64, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for yy in (4, 8):
        d.rectangle([1, yy, 30, yy + 2], fill=hexc("#9a6a3a"))
    d.rectangle([3, 11, 4, 15], fill=hexc("#2f3a36"))
    d.rectangle([27, 11, 28, 15], fill=hexc("#2f3a36"))
    d.rectangle([2, 10, 29, 11], fill=hexc("#2f3a36"))
    d.rectangle([33, 8, 62, 15], fill=hexc("#8a5a3a"))
    d.line([(33, 8), (62, 8)], fill=hexc("#b07a4a"))
    r = random.Random(5)
    for i in range(14):
        x = 35 + i * 2
        d.line([(x, 8), (x, 4 + r.randint(0, 3))], fill=hexc("#3f7d3a"))
        if i % 2 == 0:
            d.point((x, 3 + r.randint(0, 2)), fill=r.choice([hexc("#e2453a"), hexc("#f2c43a"), hexc("#e8e8f8")]))
    return im


def obj_fx():
    """effets 16x16 : poussiere x3, etincelle x2."""
    im = Image.new("RGBA", (80, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for f, r in enumerate((3, 5, 6)):
        ox = f * 16
        c = hexc("#efe8dc", 255 if f < 2 else 255)
        d.ellipse([ox + 8 - r, 12 - r, ox + 8 + r, 12 + r // 2], outline=c if f == 2 else None,
                  fill=None if f == 2 else c)
    for f in range(2):
        ox = 48 + f * 16
        s = 3 + f * 2
        d.line([(ox + 8 - s, 8), (ox + 8 + s, 8)], fill=hexc("#fff6b0"))
        d.line([(ox + 8, 8 - s), (ox + 8, 8 + s)], fill=hexc("#fff6b0"))
        d.point((ox + 8, 8), fill=hexc("#ffffff"))
    return im


def obj_hud():
    """icones du HUD 8x8 : coeur plein, coeur vide, medovnik, tete du renard."""
    im = Image.new("RGBA", (32, 8), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    h = [".XX.XX.", "XXXXXXX", "XXXXXXX", ".XXXXX.", "..XXX..", "...X..."]
    for j, row in enumerate(h):
        for i, ch in enumerate(row):
            if ch == "X":
                d.point((i, j + 1), fill=hexc("#ff4d5e"))
                d.point((8 + i, j + 1), fill=hexc("#5a5f6a"))
                d.point((16 + i, j + 1), fill=hexc("#b5442e"))
    d.point((2, 2), fill=hexc("#ffc0c8"))
    d.point((17, 2), fill=hexc("#f6e7c8"))
    d.point((19, 3), fill=hexc("#f6e7c8"))
    d.point((21, 2), fill=hexc("#f6e7c8"))
    f = ["X.....X", "XX...XX", "XXXXXXX", "XWXXXWX", "XXCCCXX", ".XCKCX.", "..CCC.."]
    cols = {"X": hexc("#ff6212"), "W": hexc("#2d2927"), "C": hexc("#fde8cf"), "K": hexc("#2d2927")}
    for j, row in enumerate(f):
        for i, ch in enumerate(row):
            if ch in cols:
                d.point((24 + i, j), fill=cols[ch])
    return im


if __name__ == "__main__":
    os.makedirs(os.path.join(ART, "kosice"), exist_ok=True)
    os.makedirs(os.path.join(ART, "objects"), exist_ok=True)
    far().save(os.path.join(ART, "kosice", "far.png"))
    mid().save(os.path.join(ART, "kosice", "mid.png"))
    tiles().save(os.path.join(ART, "kosice", "tiles.png"))
    objs = {
        "medovnik_16x16": obj_heart(), "holub_24x16": obj_pigeon(), "osa_16x16": obj_wasp(),
        "pruzina_16x16": obj_spring(), "lampa_16x48": obj_checkpoint(), "zastavka_16x48": obj_stop(),
        "elektricka_96x48": obj_tram(), "lavicka_32x16": obj_bench(), "fx_16x16": obj_fx(),
        "hud_8x8": obj_hud(),
    }
    for name, im in objs.items():
        im.save(os.path.join(ART, "objects", name + ".png"))
    print("art provisoire ecrit dans", ART)
