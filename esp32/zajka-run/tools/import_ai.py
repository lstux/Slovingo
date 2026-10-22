#!/usr/bin/env python3
"""Importe une image generee par IA (Gemini, ChatGPT...) dans art/, au bon format.

Les IA rendent de grandes images (1024 px et plus), sans vraie transparence et sans grille
exacte. Ce script : detoure le fond (magenta #FF00FF demande dans les prompts, ou couleur
unie des coins), recadre, reduit a la taille du jeu, aligne les images d'animation et
reduit la palette. Ensuite : python3 tools/build_assets.py, puis compiler.

    # fonds du niveau (theme kosice par defaut)
    python3 tools/import_ai.py far   ~/Telechargements/kosice_far.png
    python3 tools/import_ai.py mid   ~/Telechargements/kosice_mid.png
    python3 tools/import_ai.py tiles ~/Telechargements/kosice_tiles.png
    # une ligne d'animation d'un personnage (voir ANIMS : idle run jump fall hurt win)
    python3 tools/import_ai.py sprite fox run ~/Telechargements/fox_run.png --frames 6
    # un objet anime (nom de fichier existant dans art/objects/)
    python3 tools/import_ai.py object holub ~/Telechargements/pigeon.png --frames 3

Options utiles : --theme, --colors (palette, 48 par defaut pour les sprites), --no-key
(l'image a deja un fond transparent), --key RRGGBB (couleur de fond a retirer).
"""
import argparse
import glob
import os
import re

import numpy as np
from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ART = os.path.join(ROOT, "art")
ANIMS = ["idle", "run", "jump", "fall", "hurt", "win"]
SIZES = {"far": (480, 240), "mid": (640, 176), "tiles": (128, 64)}
OUTLINE = (26, 26, 46, 255)


def key_out(img, key=None, tol=150):
    """Rend transparent le fond uni (magenta par defaut, sinon la couleur des coins),
    puis retire le liseré de couleur de fond laissé par l'anticrénelage."""
    a = np.array(img.convert("RGBA")).astype(np.int32)
    if a[..., 3].min() < 250:      # deja transparente
        return Image.fromarray(a.astype(np.uint8))
    corners = np.array([a[0, 0, :3], a[0, -1, :3], a[-1, 0, :3], a[-1, -1, :3]])
    if key is None:
        key = np.median(corners, axis=0)
        if np.abs(corners - np.array([255, 0, 255])).sum(1).min() < 150:
            key = np.array([255, 0, 255])
    key = np.array(key)
    rgb = a[..., :3]
    d = np.abs(rgb - key[None, None, :]).sum(-1)
    # « magentitude » : de combien r et b depassent g (fond magenta), ou distance a la cle
    is_mag = key[1] < 80 and key[0] > 180 and key[2] > 180
    mag = np.minimum(rgb[..., 0], rgb[..., 2]) - rgb[..., 1] if is_mag else 255 - d
    alpha = np.where((d < tol) | (mag > 90), 0, 255)
    a[..., 3] = alpha
    # despill : un pixel opaque encore teinte par le fond prend la couleur moyenne de ses voisins sains
    opaque = alpha > 0
    tinted = opaque & (mag > 25)
    clean = opaque & ~tinted
    if tinted.any():
        acc = np.zeros(rgb.shape, np.float64)
        cnt = np.zeros(rgb.shape[:2], np.float64)
        for dy in (-2, -1, 0, 1, 2):
            for dx in (-2, -1, 0, 1, 2):
                sh_c = np.roll(np.roll(clean, dy, 0), dx, 1)
                sh_r = np.roll(np.roll(rgb, dy, 0), dx, 1)
                acc += sh_r * sh_c[..., None]
                cnt += sh_c
        fix = tinted & (cnt > 0)
        rgb[fix] = (acc[fix] / cnt[fix][:, None]).astype(np.int32)
        a[..., 3] = np.where(tinted & (cnt == 0), 0, a[..., 3])
        a[..., :3] = rgb
    return Image.fromarray(np.clip(a, 0, 255).astype(np.uint8))


def crop_alpha(img):
    bb = img.getbbox()
    return img.crop(bb) if bb else img


def shrink(img, w, h):
    """Reduction douce (BOX) puis alpha tout ou rien : rendu pixel art propre."""
    img = img.resize((w, h), Image.BOX)
    a = np.array(img)
    a[..., 3] = np.where(a[..., 3] >= 128, 255, 0)
    return Image.fromarray(a)


def quantize_rgba(img, colors):
    a = np.array(img)
    rgb = Image.fromarray(a[..., :3])
    q = rgb.quantize(colors=colors, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE).convert("RGB")
    out = np.dstack([np.array(q), a[..., 3]])
    return Image.fromarray(out.astype(np.uint8))


def outline(img):
    a = np.array(img)
    m = a[..., 3] > 0
    p = np.pad(m, 1)
    nb = p[:-2, 1:-1] | p[2:, 1:-1] | p[1:-1, :-2] | p[1:-1, 2:]
    edge = nb & ~m
    a[edge] = OUTLINE
    return Image.fromarray(a)


def cover(img, w, h):
    """Recadre au centre pour remplir w x h sans deformer."""
    sw, sh = img.size
    s = max(w / sw, h / sh)
    nw, nh = int(round(sw * s)), int(round(sh * s))
    img = img.resize((nw, nh), Image.LANCZOS)
    x, y = (nw - w) // 2, nh - h  # on garde le bas (le sol)
    return img.crop((x, y, x + w, y + h))


def frames_from(img, n):
    """Coupe une bande horizontale en n images, aux n-1 plus larges colonnes vides
    (a parts egales s'il n'y en a pas assez)."""
    if n == 1:
        return [img]
    cols = (np.array(img)[..., 3] > 0).any(0)
    gaps, x = [], 0
    while x < len(cols):
        if not cols[x]:
            s0 = x
            while x < len(cols) and not cols[x]:
                x += 1
            if s0 > 0 and x < len(cols):
                gaps.append((x - s0, (s0 + x) // 2))
        else:
            x += 1
    if len(gaps) >= n - 1:
        cuts = sorted(c for _, c in sorted(gaps, reverse=True)[:n - 1])
    else:
        cuts = [int(img.width * (i + 1) / n) for i in range(n - 1)]
    edges = [0] + cuts + [img.width]
    return [img.crop((edges[i], 0, edges[i + 1], img.height)) for i in range(n)]


def fit_frame(fr, fw, fh, colors, pad_bottom=1):
    """Place une image dans une cellule fw x fh : hauteur ajustee, pieds en bas, centree."""
    fr = crop_alpha(fr)
    s = min((fh - pad_bottom - 1) / fr.height, (fw - 2) / fr.width)
    nw, nh = max(1, int(round(fr.width * s))), max(1, int(round(fr.height * s)))
    fr = shrink(fr, nw, nh)
    fr = quantize_rgba(fr, colors)
    cell = Image.new("RGBA", (fw, fh), (0, 0, 0, 0))
    cell.alpha_composite(fr, ((fw - nw) // 2, fh - pad_bottom - nh))
    return outline(cell)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("kind", choices=["far", "mid", "tiles", "sprite", "object"])
    ap.add_argument("args", nargs="+")
    ap.add_argument("--theme", default="kosice")
    ap.add_argument("--frames", type=int, default=1)
    ap.add_argument("--colors", type=int, default=0)
    ap.add_argument("--no-key", action="store_true")
    ap.add_argument("--key", default=None)
    a = ap.parse_args()
    key = tuple(int(a.key[i:i + 2], 16) for i in (0, 2, 4)) if a.key else None

    if a.kind in SIZES:
        src = Image.open(a.args[0]).convert("RGBA")
        w, h = SIZES[a.kind]
        if a.kind == "far":
            img = cover(src, w, h)
            img = quantize_rgba(img, a.colors or 200)
        elif a.kind == "mid":
            img = src if a.no_key else key_out(src, key)
            img = cover(img, w, h)
            al = np.array(img)
            al[..., 3] = np.where(al[..., 3] >= 128, 255, 0)
            img = quantize_rgba(Image.fromarray(al), a.colors or 200)
        else:  # tiles : grille 8 x 4 de tuiles
            img = src if a.no_key else key_out(src, key)
            img = shrink(img, w, h)
            img = quantize_rgba(img, a.colors or 96)
        out = os.path.join(ART, a.theme, a.kind + ".png")
        os.makedirs(os.path.dirname(out), exist_ok=True)
        img.save(out)
        print("ecrit", os.path.relpath(out, ROOT))
        return

    if a.kind == "sprite":
        name, anim, path = a.args
        sheet_path = glob.glob(os.path.join(ART, "sprites", f"{name}_*x*.png"))[0]
        row = ANIMS.index(anim)
    else:
        name, path = a.args
        sheet_path = glob.glob(os.path.join(ART, "objects", f"{name}_*x*.png"))[0]
        row = 0
    fw, fh = map(int, re.search(r"_(\d+)x(\d+)\.png$", sheet_path).groups())
    sheet = Image.open(sheet_path).convert("RGBA")
    src = Image.open(path).convert("RGBA")
    src = src if a.no_key else key_out(src, key)
    frames = frames_from(crop_alpha(src), a.frames)
    need_w = fw * len(frames)
    if sheet.width < need_w or sheet.height < (row + 1) * fh:
        big = Image.new("RGBA", (max(sheet.width, need_w), max(sheet.height, (row + 1) * fh)), (0, 0, 0, 0))
        big.paste(sheet, (0, 0))
        sheet = big
    # efface la ligne puis place les nouvelles images
    sheet.paste(Image.new("RGBA", (sheet.width, fh), (0, 0, 0, 0)), (0, row * fh))
    for i, fr in enumerate(frames):
        sheet.paste(fit_frame(fr, fw, fh, a.colors or 48), (i * fw, row * fh))
    sheet.save(sheet_path)
    print(f"ecrit {os.path.relpath(sheet_path, ROOT)} : ligne {row}, {len(frames)} image(s) de {fw}x{fh}")
    if a.kind == "sprite":
        print("Rappel : le nombre d'images par animation est fixe dans src/game/game.cpp (ANIM_LEN).")


if __name__ == "__main__":
    main()
