#!/usr/bin/env python3
"""Convertit art/ et maps/ en donnees C pour le firmware (src/game/gen/).

    python3 tools/build_assets.py

Reprend l'outil de Zajka Run (memes formats Image8 / Sheet / Font) et y ajoute :
  art/tiles/<theme>.png       planche de tuiles 16 x 16, 16 par ligne, dans l'ordre de TILESET
  art/walk/<nom>_16x24.png    planches de marche (lignes bas, haut, gauche, droite ; 3 images)
  maps/<id>.txt               cartes en texte : une lettre par tuile (TILESET) ; un caractere de
                              MARK_CHARS (chiffre, !, $, %...) = un repere decrit dans l'en-tete :
                                start DIR | warp CARTE X Y DIR | npc QUI DIR [say EP N-M] [id NOM]
                                | sign MOT (panneau, le mot va au Slovnik) | item OBJET | zone NOM
                              suivi de « on T » pour la tuile dessous ; « night: 1 » : de nuit
  story/items.txt             les objets (ITEMS, ITEM_...) ; icones dans art/items/<id>.png

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
           + "àâçèêëîïùûüÿœÀÂÇÈÊËÎÏÙÛÜŒ«»…×’—–·→›„“”")


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
    # maison de la tante d'Andrea (episode 6)
    ("D", "maison", "S"),
    ("d", "porte_maison", "S"),
    ("#", "cloture", "S"),
    ("g", "portail", "W"),
    (":", "allee", "W"),
    ("J", "pommier", "S"),
    ("A", "feuillage_pommier", "O"),
    ("k", "buisson", "S"),
    # chapitre 1, la quete du ticket ; panneaux
    ("K", "trafika", "S"),
    ("V", "composteur", "S"),
    ("P", "panneau", "S"),
    # chapitre 2 : Sucho (parc a sec, fontaine vide, gare)
    ("F", "fontaine_vide", "S"),
    ("Y", "eau", "S"),
    ("X", "boue", "W"),
    ("B", "guichet", "S"),
    ("N", "sol_gare", "W"),
    ("R", "train", "S"),
    ("r", "porte_train", "W"),
    # chapitre 3 : Slovensky raj (foret, ruisseau, gorge aux echelles et aux planches, camp de nuit)
    ("S", "sapin", "S"),
    ("p", "sentier", "W"),
    ("o", "rocher", "S"),
    ("C", "falaise", "S"),
    ("q", "ruisseau", "S"),
    ("v", "eau_gorge", "W"),     # on n'y marche pas : on tombe (chapter3.cpp)
    ("L", "planche", "W"),       # cede quand le renard la quitte
    ("u", "rebrik", "W"),
    ("n", "most", "W"),
    ("a", "dalle", "W"),
    ("Q", "ohnisko", "S"),
]
TILE_INDEX = {c: i for i, (c, _, _) in enumerate(TILESET)}
TILE_KINDS = {"W": "TK_WALK", "S": "TK_SOLID", "O": "TK_OVER"}
TILES_PER_ROW = 16
THEMES = ["kosice"]
DIRS = {"down": "DIR_DOWN", "up": "DIR_UP", "left": "DIR_LEFT", "right": "DIR_RIGHT"}
NPCS = {"andrea": "SPK_ANDREA", "fox": "SPK_FOX", "mouse": "SPK_MOUSE", "chick": "SPK_CHICK", "bird": "SPK_BIRD", "orol": "SPK_OROL", "mato": "SPK_MATO", "katka": "SPK_KATKA", "babka": "SPK_BABKA",
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


# Caracteres possibles pour un repere dans une carte (jamais des lettres de tuile).
MARK_CHARS = "0123456789!$%&+;<>?~'()[]{}/@"


def parse_map(path):
    meta, marks, rows, in_map = {}, {}, [], False
    for line in open(path, encoding="utf-8").read().split("\n"):
        if not in_map:
            if line.strip() == "---":
                in_map = True
            elif line.startswith("#") or ":" not in line:
                continue
            else:
                k, v = line.split(":", 1)
                k, v = k.strip(), v.strip()
                if len(k) == 1 and k in MARK_CHARS:
                    marks[k] = v.split()   # « 1: warp kosice_tram 9 5 up on e »
                else:
                    meta[k] = v
        else:
            rows.append(line.rstrip("\n"))
    while rows and rows[-1] == "":
        rows.pop()
    w = max(len(r) for r in rows)
    return meta, marks, [r.ljust(w) for r in rows]


class Registry:
    """Ce qui est partage entre toutes les cartes : noms (TAG_...), mots des panneaux, objets."""
    def __init__(self, items):
        self.tags = []      # TAG_x = index + 1
        self.words = []     # SIGN_WORDS
        self.items = items  # [(id, sk, fr)]
        self.pickups = 0

    def tag(self, name):
        if name not in self.tags:
            self.tags.append(name)
        return f"TAG_{c_ident(name).upper()}"

    def word(self, w):
        w = w.replace("_", " ")
        if w not in self.words:
            self.words.append(w)
        return self.words.index(w)

    def item(self, iid, where):
        ids = [i for i, _, _ in self.items]
        if iid not in ids:
            raise SystemExit(f"{where} : objet inconnu « {iid} » (voir story/items.txt)")
        return f"ITEM_{c_ident(iid).upper()}"


def mark_c(mtype, x, y, d="DIR_DOWN", target=-1, tx=0, ty=0, npc="SPK_NONE", say=(0, 0, 0), tag="0",
           item="-1", pickup=0, word=-1):
    return (f"{{{mtype}, {x}, {y}, {d}, {target}, {tx}, {ty}, {npc}, {say[0]}, {say[1]}, {say[2]}, "
            f"{tag}, {item}, {pickup}, {word}}}")


def build_map(o, path, map_ids, reg):
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
            if ch in MARK_CHARS:
                m = marks.get(ch)
                if m is None:
                    raise SystemExit(f"{where} : repere « {ch} » en ({x}, {y}) sans description « {ch}: ... »")
                on = m[m.index("on") + 1] if "on" in m else ground
                kind, args = m[0], m[1:m.index("on")] if "on" in m else m[1:]
                try:
                    if kind == "start":
                        cmarks.append(mark_c("M_START", x, y, DIRS[args[0]]))
                    elif kind == "warp":
                        tgt, tx, ty, tdir = args
                        if tgt not in map_ids:
                            raise SystemExit(f"{where} : porte {ch} vers une carte inconnue « {tgt} »")
                        cmarks.append(mark_c("M_WARP", x, y, DIRS[tdir], map_ids.index(tgt), tx, ty))
                    elif kind == "npc":
                        who, d, rest = args[0], args[1], args[2:]
                        say, tag = (0, 0, 0), "0"
                        while rest:
                            if rest[0] == "say":
                                ep = int(rest[1])
                                a_, _, b_ = rest[2].partition("-")
                                say = (ep, int(a_), (int(b_) if b_ else int(a_)) - int(a_) + 1)
                                rest = rest[3:]
                            elif rest[0] == "id":
                                tag = reg.tag(rest[1])
                                rest = rest[2:]
                            else:
                                raise SystemExit(f"{where} : repere {ch} : « {rest[0]} » inattendu")
                        cmarks.append(mark_c("M_NPC", x, y, DIRS[d], npc=NPCS[who], say=say, tag=tag))
                    elif kind == "sign":
                        cmarks.append(mark_c("M_SIGN", x, y, word=reg.word(args[0])))
                    elif kind == "item":
                        cmarks.append(mark_c("M_ITEM", x, y, item=reg.item(args[0], where), pickup=reg.pickups))
                        reg.pickups += 1
                    elif kind == "zone":
                        cmarks.append(mark_c("M_ZONE", x, y, tag=reg.tag(args[0])))
                    else:
                        raise SystemExit(f"{where} : repere {ch} de type inconnu « {kind} »")
                except (IndexError, KeyError, ValueError) as e:
                    raise SystemExit(f"{where} : repere {ch} mal ecrit ({' '.join(m)}) : {e}")
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
    o.data.append(f"static const MapMark {name}_marks[{max(1, len(cmarks))}] = {{\n  "
                  + ",\n  ".join(cmarks or ["{}"]) + "\n};\n")
    music = int(meta.get("music", "-1"))
    night = int(meta.get("night", "0"))
    o.data.append(f"const MapDef {name} = {{ {c_str(mid)}, {c_str(meta.get('name', mid))}, &tileset_{meta.get('theme', 'kosice')}, "
                  f"{w}, {h}, {name}_base, {over_ref}, {name}_marks, {len(cmarks)}, {music}, {night} }};\n")
    return name, rows, marks


def load_items():
    """story/items.txt : « id | slovaque | francais »."""
    items = []
    path = os.path.join(ROOT, "story", "items.txt")
    for n, line in enumerate(open(path, encoding="utf-8"), 1):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = [t.strip() for t in line.split("|")]
        if len(parts) != 3:
            raise SystemExit(f"story/items.txt:{n} : « id | slovaque | francais » attendu")
        items.append(tuple(parts))
    return items


def check_warps(maps):
    """Chaque porte doit arriver sur une case ou l'on peut marcher."""
    for mid, (meta, marks, rows) in maps.items():
        for ch, m in marks.items():
            if m[0] != "warp":
                continue
            tgt, tx, ty = m[1], int(m[2]), int(m[3])
            tmeta, tmarks, trows = maps[tgt]
            c = trows[ty][tx] if ty < len(trows) and tx < len(trows[ty]) else " "
            if c in MARK_CHARS:  # un repere : la tuile dessous (un PNJ qui s'ecarte, par exemple)
                tm = tmarks.get(c, [])
                c = tm[tm.index("on") + 1] if "on" in tm else tmeta.get("ground", ".")
            if TILESET[TILE_INDEX.get(c, 0)][2] == "S":
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
    # portraits de la boite de dialogue (art/portraits/<nom>_48x48.png) : nullptr si absent
    portraits = {}
    for p in sorted(glob.glob(os.path.join(ART, "portraits", "*_48x48.png"))):
        who = os.path.basename(p).split("_")[0]
        spk = NPCS.get(who)
        if spk is None:
            raise SystemExit(f"art/portraits : personnage inconnu « {who} » (voir NPCS)")
        o.image(f"portrait_{c_ident(who)}", Image.open(p))
        portraits[spk] = f"&portrait_{c_ident(who)}"
    order = ["SPK_NONE", "SPK_NARRATOR", "SPK_FOX", "SPK_ANDREA", "SPK_MATO", "SPK_KATKA", "SPK_BABKA",
             "SPK_OROL", "SPK_JEZKOVA", "SPK_HUMAN", "SPK_JAN", "SPK_MAREK", "SPK_MOUSE", "SPK_CHICK", "SPK_BIRD"]
    o.data.append("const Image8* const PORTRAITS[SPK_COUNT] = { "
                  + ", ".join(portraits.get(k, "nullptr") for k in order) + " };\n")
    o.decl.append("extern const Image8* const PORTRAITS[SPK_COUNT];")
    # tuiles et cartes
    o.data.append("static const uint8_t tile_kinds[%d] = { %s };\n" % (
        len(TILESET), ", ".join(TILE_KINDS[k] for _, _, k in TILESET)))
    for i, (_, tname, _) in enumerate(TILESET):
        o.decl.append(f"constexpr int TL_{c_ident(tname).upper()} = {i};")
    for th in THEMES:
        o.image(f"tiles_{th}", Image.open(os.path.join(ART, "tiles", f"{th}.png")))
        o.data.append(f"const Tileset tileset_{th} = {{ &tiles_{th}, tile_kinds, {len(TILESET)} }};\n")
        o.decl.append(f"extern const Tileset tileset_{th};")
    # objets (story/items.txt) et leurs icones 16 x 16 (art/items/<id>.png, une case vide si absente)
    items = load_items()
    icons = Image.new("RGBA", (16 * max(1, len(items)), 16), (0, 0, 0, 0))
    for i, (iid, _, _) in enumerate(items):
        ip = os.path.join(ART, "items", f"{iid}.png")
        if os.path.exists(ip):
            icons.paste(Image.open(ip).convert("RGBA").resize((16, 16)), (16 * i, 0))
    w_, h_ = o.image("img_item_icons", icons)
    o.data.append(f"const Sheet item_icons = {{ &img_item_icons, 16, 16, {w_ // 16} }};\n")
    o.decl.append("extern const Sheet item_icons;")
    o.data.append("const Item ITEMS[] = { " + ", ".join(f"{{{c_str(sk)}, {c_str(fr)}}}" for _, sk, fr in items) + " };\n")
    o.data.append(f"const int NUM_ITEMS = {len(items)};\n")
    o.decl.append("struct Item { const char* sk; const char* fr; };")
    o.decl.append("extern const Item ITEMS[];\nextern const int NUM_ITEMS;")
    for i, (iid, _, _) in enumerate(items):
        o.decl.append(f"constexpr int ITEM_{c_ident(iid).upper()} = {i};")
    reg = Registry(items)
    paths = sorted(glob.glob(os.path.join(ROOT, "maps", "*.txt")))
    map_ids = [os.path.splitext(os.path.basename(p))[0] for p in paths]
    names, parsed = [], {}
    # les noms d'abord (les cartes en ont besoin dans leurs reperes)
    for p in paths:
        _, marks, _ = parse_map(p)
        for m in marks.values():
            if m and m[0] == "npc" and "id" in m:
                reg.tag(m[m.index("id") + 1])
            if m and m[0] == "zone":
                reg.tag(m[1])
    for i, t in enumerate(reg.tags):
        o.decl.append(f"constexpr uint8_t TAG_{c_ident(t).upper()} = {i + 1};")
    for p in paths:
        n, rows, marks = build_map(o, p, map_ids, reg)
        names.append(n)
        parsed[os.path.splitext(os.path.basename(p))[0]] = (parse_map(p)[0], marks, rows)
    check_warps(parsed)
    o.data.append("const MapDef* const ALL_MAPS[] = { " + ", ".join(f"&{n}" for n in names) + " };\n")
    o.data.append(f"const int NUM_MAPS = {len(names)};\n")
    if reg.pickups > 64 or len(reg.words) > 64:
        raise SystemExit(f"{reg.pickups} objets a ramasser et {len(reg.words)} mots de panneaux : 64 au plus (sauvegarde)")
    o.data.append("const char* const SIGN_WORDS[] = { " + ", ".join(c_str(w) for w in reg.words or [""]) + " };\n")
    o.data.append(f"const int NUM_SIGN_WORDS = {len(reg.words)};\nconst int NUM_PICKUPS = {reg.pickups};\n")
    o.decl.append("extern const char* const SIGN_WORDS[];\nextern const int NUM_SIGN_WORDS;\nextern const int NUM_PICKUPS;")
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
