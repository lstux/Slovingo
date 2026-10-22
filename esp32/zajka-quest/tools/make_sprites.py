#!/usr/bin/env python3
"""Planches de sprites provisoires du renard (et d'Andrea) depuis les SVG des personnages.

Les SVG viennent de MathPulse (assets/story/fox-full.svg, rabbit-full.svg) et sont
copies dans art/characters/. Chaque image d'animation est une pose (rotations des
bras, jambes, queue), rendue a la taille du jeu puis « pixelisee » : alpha tout ou rien,
couleurs ramenees a la palette du personnage, contour sombre d'un pixel.

    python3 tools/make_sprites.py          -> art/sprites/fox.png, art/sprites/rabbit.png

Une planche dessinee a la main ou generee par IA peut remplacer ces PNG : meme grille
(images de FRAME_W x FRAME_H, une ligne par animation, voir ANIMS), fond transparent.
"""
import os
import sys
import xml.etree.ElementTree as ET

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(__file__))
from svgpose import SVGNS, Renderer  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
CHAR = os.path.join(ROOT, "art", "characters")
OUT = os.path.join(ROOT, "art", "sprites")

FRAME_W, FRAME_H = 32, 48   # cellule de la planche
BODY_H = 46                 # hauteur du personnage (pieds -> pointe des oreilles)
OUTLINE = (26, 26, 46)      # #1a1a2e, la couleur d'ombre des SVG

# Une ligne par animation : (nom, liste de poses). Un nom de pose -> dict de transformations.
ANIMS = ["idle", "run", "jump", "fall", "hurt", "win"]


def split_legs(r, legs_id, left_idx, right_idx):
    """Coupe le groupe des jambes en deux sous-groupes animables legL / legR."""
    legs = r.ids[legs_id]
    kids = list(legs)
    gl = ET.Element(SVGNS + "g", {"id": "legL"})
    gr = ET.Element(SVGNS + "g", {"id": "legR"})
    for i, k in enumerate(kids):
        legs.remove(k)
        (gl if i in left_idx else gr).append(k)
    # la jambe de derriere d'abord
    legs.append(gr)
    legs.append(gl)
    r.ids["legL"], r.ids["legR"] = gl, gr


def fox_poses():
    hipL, hipR = (210, 500), (270, 500)
    shL, shR = (182, 356), (298, 358)
    tail = (200, 470)

    def pose(lean=0.0, legL=0.0, legR=0.0, armL=0.0, armR=0.0, tail_a=0.0, bob=0.0,
             liftL=0.0, liftR=0.0):
        body = f"translate(0 {bob}) rotate({lean} 240 740)"
        return {
            "foxf-shadow": None,
            "foxf-tail": f"{body} rotate({tail_a} {tail[0]} {tail[1]})",
            "foxf-legs": f"rotate({lean} 240 740)",
            "legL": f"translate(0 {-liftL}) rotate({legL} {hipL[0]} {hipL[1]})",
            "legR": f"translate(0 {-liftR}) rotate({legR} {hipR[0]} {hipR[1]})",
            "foxf-arm-l": f"{body} rotate({armL} {shL[0]} {shL[1]})",
            "foxf-torso": body,
            "foxf-arm-r": f"{body} rotate({armR} {shR[0]} {shR[1]})",
            "foxf-head": body,
        }

    run = []
    for ph in range(6):
        s = np.sin(ph / 6 * 2 * np.pi)
        c = np.cos(ph / 6 * 2 * np.pi)
        run.append(pose(lean=8, legL=28 * s, legR=-28 * s, armL=-40 * s, armR=40 * s,
                        tail_a=-10 + 8 * c, bob=-abs(c) * 14,
                        liftL=max(0, -s) * 30, liftR=max(0, s) * 30))
    return {
        "idle": [pose(bob=0), pose(bob=4, tail_a=5), pose(bob=6, tail_a=8), pose(bob=4, tail_a=5)],
        "run": run,
        "jump": [pose(lean=4, legL=-20, legR=12, liftL=40, armL=125, armR=-125, tail_a=-25)],
        "fall": [pose(lean=0, legL=14, legR=-14, armL=95, armR=-95, tail_a=20, liftL=10, liftR=10)],
        "hurt": [pose(lean=-6, legL=20, legR=-10, armL=60, armR=-60, tail_a=30)],
        "win": [pose(armR=-150, bob=0), pose(armR=-170, bob=-20, tail_a=10)],
    }


def rabbit_poses():
    hipL, hipR = (212, 520), (268, 520)
    # le bras gauche tient la cle (pivot a l'epaule), le bras droit salue
    shL, shR = (188, 380), (292, 382)

    def pose(lean=0.0, legL=0.0, legR=0.0, armL=0.0, armR=0.0, bob=0.0, liftL=0.0, liftR=0.0):
        body = f"translate(0 {bob}) rotate({lean} 240 740)"
        return {
            "rbf-shadow": None,
            "rbf-legs": f"rotate({lean} 240 740)",
            "legL": f"translate(0 {-liftL}) rotate({legL} {hipL[0]} {hipL[1]})",
            "legR": f"translate(0 {-liftR}) rotate({legR} {hipR[0]} {hipR[1]})",
            "rbf-arm-wave": f"{body} rotate({armR} {shR[0]} {shR[1]})",
            "rbf-torso": body,
            "rbf-arm-wrench": f"{body} rotate({armL} {shL[0]} {shL[1]})",
            "rbf-head": body,
        }

    run = []
    for ph in range(6):
        s = np.sin(ph / 6 * 2 * np.pi)
        c = np.cos(ph / 6 * 2 * np.pi)
        run.append(pose(lean=8, legL=28 * s, legR=-28 * s, armL=-30 * s, bob=-abs(c) * 14,
                        liftL=max(0, -s) * 30, liftR=max(0, s) * 30))
    return {
        "idle": [pose(), pose(bob=4, armR=-10), pose(bob=6, armR=-20), pose(bob=4, armR=-10)],
        "run": run,
        "jump": [pose(lean=4, legL=-20, legR=12, liftL=40, armL=40, armR=-60)],
        "fall": [pose(legL=14, legR=-14, liftL=10, liftR=10, armL=40, armR=-30)],
        "hurt": [pose(lean=-6, legL=20, legR=-10)],
        "win": [pose(armR=-20), pose(armR=-40, bob=-20)],
    }


def svg_palette(svg_file):
    import re
    cols = set(re.findall(r"#[0-9a-fA-F]{6}\b|#[0-9a-fA-F]{3}\b", open(svg_file).read()))
    out = []
    for c in cols:
        c = c[1:]
        if len(c) == 3:
            c = "".join(ch * 2 for ch in c)
        out.append(tuple(int(c[i:i + 2], 16) for i in (0, 2, 4)))
    return np.array(out, dtype=np.float32)


def pixelize(rgba, palette, outline=True):
    """alpha tout ou rien, couleurs de la palette, contour d'un pixel."""
    a = rgba[..., 3] >= 110
    rgb = rgba[..., :3].astype(np.float32)
    d = ((rgb[:, :, None, :] - palette[None, None, :, :]) ** 2).sum(-1)
    q = palette[d.argmin(-1)].astype(np.uint8)
    out = np.zeros_like(rgba)
    out[a, :3] = q[a]
    out[a, 3] = 255
    if outline:
        p = np.pad(a, 1)
        nb = p[:-2, 1:-1] | p[2:, 1:-1] | p[1:-1, :-2] | p[1:-1, 2:]
        edge = nb & ~a
        out[edge, :3] = OUTLINE
        out[edge, 3] = 255
    return out


def build(svg_name, poses_fn, legs_id, left_idx, right_idx, crop, out_name):
    svg = os.path.join(CHAR, svg_name)
    r = Renderer(svg)
    split_legs(r, legs_id, left_idx, right_idx)
    pal = svg_palette(svg)
    pal = np.vstack([pal, np.array([OUTLINE], dtype=np.float32)])
    poses = poses_fn()
    ncols = max(len(poses[a]) for a in ANIMS)
    sheet = Image.new("RGBA", (ncols * FRAME_W, len(ANIMS) * FRAME_H), (0, 0, 0, 0))
    x0, y0, w, h = crop
    for row, anim in enumerate(ANIMS):
        for col, p in enumerate(poses[anim]):
            # rendu dans une cellule FRAME_W x FRAME_H, pieds en bas, centre du corps au milieu
            scale = BODY_H / h
            cw, ch = FRAME_W / scale, FRAME_H / scale
            cx = 240 - cw / 2 + 2 / scale      # corps un peu a droite : la queue depasse a gauche
            cy = (y0 + h) - ch + 1 / scale
            img = r.render(FRAME_H, pose=p, width_px=FRAME_W, crop=(cx, cy, cw, ch))
            px = pixelize(img, pal)
            sheet.paste(Image.fromarray(px), (col * FRAME_W, row * FRAME_H))
    os.makedirs(OUT, exist_ok=True)
    sheet.save(os.path.join(OUT, out_name))
    print("ecrit", out_name, sheet.size)


def build_fw_tile():
    """Icone 86x48 de la liste des firmwares de l'ODROID-GO : tete du renard + titre."""
    from PIL import ImageDraw, ImageFont
    tile = Image.new("RGBA", (86, 48), (0, 0, 0, 255))
    d = ImageDraw.Draw(tile)
    for y in range(48):
        t = y / 47
        d.line([(0, y), (86, y)], fill=(int(61 + 130 * t), int(127 + 100 * t), int(214 + 32 * t), 255))
    d.rectangle([0, 40, 86, 48], fill=(184, 173, 160, 255))
    head = Renderer(os.path.join(CHAR, "fox-head.svg")).render(38, width_px=38)
    tile.alpha_composite(Image.fromarray(head), (2, 3))
    try:
        ft = ImageFont.truetype("/usr/share/fonts/truetype/google-fonts/Poppins-Bold.ttf", 13)
    except OSError:
        ft = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 12)
    for txt, y in (("ZAJKA", 6), ("RUN", 21)):
        d.text((43, y + 1), txt, font=ft, fill=(26, 26, 46, 255))
        d.text((42, y), txt, font=ft, fill=(255, 98, 18, 255) if txt == "RUN" else (255, 255, 255, 255))
    tile = tile.convert("RGB")
    tile.save(os.path.join(ROOT, "art", "fw_tile.png"))
    # version brute RGB565 petit-boutiste pour tools/mkfw.py (PlatformIO n'a pas Pillow)
    import struct
    raw = bytearray()
    for y in range(48):
        for x in range(86):
            r, g, b = tile.getpixel((x, y))
            raw += struct.pack("<H", ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
    open(os.path.join(ROOT, "art", "fw_tile.raw"), "wb").write(bytes(raw))
    print("ecrit fw_tile.png / fw_tile.raw")


if __name__ == "__main__":
    build_fw_tile()
    # fox-full : jambes = 2 paths, 2 rects, 2 uses ; indices pairs = jambe gauche
    build("fox-full.svg", fox_poses, "foxf-legs", {0, 2, 4}, {1, 3, 5},
          (30, 50, 360, 700), "fox_32x48.png")
    # rabbit-full : 2 paths, 1 g (rayures, les deux ensemble), 2 uses
    build("rabbit-full.svg", rabbit_poses, "rbf-legs", {0, 2, 3}, {1, 4},
          (60, 50, 340, 700), "rabbit_32x48.png")
