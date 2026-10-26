#!/usr/bin/env python3
"""Importe une image generee par IA (Gemini, ChatGPT...) dans art/, au bon format.

Les IA rendent de grandes images (1024 px et plus), sans vraie transparence et sans grille
exacte. Ce script : detoure le fond (magenta #FF00FF demande dans les prompts, ou couleur
unie des coins), recadre, reduit a la taille du jeu, aligne les images d'animation et
reduit la palette. Ensuite : python3 tools/build_assets.py, puis compiler.

    # fonds du niveau (theme kosice par defaut)
    python3 tools/import_ai.py far   ~/Telechargements/kosice_far.png --mirror
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
ANIMS = ["idle", "run", "jump", "fall", "hurt", "win", "climb", "glide"]
SIZES = {"far": (480, 240), "mid": (640, 148), "tiles": (128, 64)}  # far/mid : hauteur fixe, largeur libre
SCREEN_W_MIN = 320
OUTLINE = (26, 26, 46, 255)


def key_out(img, key=None, tol=150, mag_cut=90, tint_cut=25):
    """Rend transparent le fond uni (magenta par defaut, sinon la couleur des coins),
    puis retire le liseré de couleur de fond laissé par l'anticrénelage."""
    a = np.array(img.convert("RGBA")).astype(np.int32)
    if a[..., 3].min() < 250:      # deja transparente
        return Image.fromarray(a.astype(np.uint8))
    corners = np.array([a[0, 0, :3], a[0, -1, :3], a[-1, 0, :3], a[-1, -1, :3]])
    if key is None:
        # magenta des qu'il couvre une part notable de l'image (les coins peuvent etre un cadre noir)
        near_mag = (np.abs(a[..., :3] - np.array([255, 0, 255])).sum(-1) < 150).mean()
        key = np.array([255, 0, 255]) if near_mag > 0.02 else np.median(corners, axis=0)
    key = np.array(key)
    rgb = a[..., :3]
    d = np.abs(rgb - key[None, None, :]).sum(-1)
    # « magentitude » : de combien r et b depassent g (fond magenta), ou distance a la cle
    is_mag = key[1] < 80 and key[0] > 180 and key[2] > 180
    mag = np.minimum(rgb[..., 0], rgb[..., 2]) - rgb[..., 1] if is_mag else 255 - d
    alpha = np.where((d < tol) | (mag > mag_cut), 0, 255)
    a[..., 3] = alpha
    # despill : un pixel opaque encore teinte par le fond prend la couleur moyenne de ses voisins sains
    opaque = alpha > 0
    tinted = opaque & (mag > tint_cut)
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


def fit_strip(frames, fw, fh, colors, pad_bottom=1):
    """Comme fit_frame, mais une seule echelle pour toutes les images : les tailles et les
    hauteurs relatives sont gardees (ressort comprime, petit nuage, etc.)."""
    strip_h = max(f.height for f in frames)
    boxes = []
    for f in frames:
        bb = f.getbbox() or (0, 0, 1, 1)
        boxes.append(bb)
    max_w = max(bb[2] - bb[0] for bb in boxes)
    top = min(bb[1] for bb in boxes)
    bot = max(bb[3] for bb in boxes)
    s = min((fh - pad_bottom - 1) / (bot - top), (fw - 2) / max_w)
    cells = []
    for f, bb in zip(frames, boxes):
        part = f.crop((bb[0], bb[1], bb[2], bb[3]))
        nw, nh = max(1, int(round(part.width * s))), max(1, int(round(part.height * s)))
        part = quantize_rgba(shrink(part, nw, nh), colors)
        cell = Image.new("RGBA", (fw, fh), (0, 0, 0, 0))
        cell.alpha_composite(part, ((fw - nw) // 2, fh - pad_bottom - nh))
        cells.append(outline(cell))
    return cells


def remove_rules(img):
    """Efface les traits de separation qu'une IA ajoute parfois (lignes ou colonnes sombres
    qui traversent presque toute l'image)."""
    a = np.array(img)
    op = a[..., 3] > 0
    lum = a[..., :3].mean(-1)
    dark = op & (lum < 70)
    rows = dark.mean(1) > 0.6
    cols = dark.mean(0) > 0.6
    for y in np.where(rows)[0]:
        a[max(0, y - 1):y + 2, :, 3] = 0
    for x in np.where(cols)[0]:
        a[:, max(0, x - 1):x + 2, 3] = 0
    return Image.fromarray(a)


def grid_frames(img, rows, cols):
    """Decoupe une planche en grille (rows x cols), images lues ligne par ligne."""
    W, H = img.size
    out = []
    for r in range(rows):
        for c in range(cols):
            out.append(img.crop((int(c * W / cols), int(r * H / rows), int((c + 1) * W / cols), int((r + 1) * H / rows))))
    return out


def fit_character(frames, fw, fh, colors, head_px, anchor_x, pad_bottom=1, height_px=0):
    """Personnage : meme taille de tete dans toutes les poses (les IA changent les proportions
    d'une image a l'autre), pieds en bas de la case, centre de la tete toujours a anchor_x
    (pas de sautillement d'une image a l'autre). La tete va du haut des oreilles au haut du
    sweat (premiere ligne nettement bleue) ; sans sweat bleu, toute la hauteur compte."""
    # une seule echelle par planche (les images d'une meme generation sont a la meme echelle) :
    # mediane des tailles de tete, puis plafond pour que chaque pose tienne dans la case
    frames = [crop_alpha(fr) for fr in frames]
    heads = []
    for fr in frames:
        a = np.array(fr)
        op = a[..., 3] > 0
        r, g, b = (a[..., i].astype(int) for i in range(3))
        blue = op & (b > 140) & (r < 110) & (b - r > 60)
        # ligne de sweat assez large : une manche levee ne compte pas
        rows = np.where(blue.mean(1) > 0.12)[0]
        heads.append(int(rows[0]) if len(rows) and rows[0] > 5 else fr.height // 3)
    if height_px:   # personnage sans sweat bleu, poses debout : on cale sur la hauteur totale
        s0 = height_px / float(np.median([fr.height for fr in frames]))
        heads = [fr.height // 3 for fr in frames]
    else:
        s0 = head_px / float(np.median(heads))
    s0 = min([s0] + [min((fh - pad_bottom - 1) / fr.height, (fw - 2) / fr.width) for fr in frames])
    cells = []
    for fr, head_h in zip(frames, heads):
        a = np.array(fr)
        op = a[..., 3] > 0
        s = s0
        nw, nh = max(1, int(round(fr.width * s))), max(1, int(round(fr.height * s)))
        hx = np.where(op[:max(1, head_h)].any(0))[0]
        head_cx = (hx.min() + hx.max()) / 2 * s if len(hx) else nw / 2
        part = quantize_rgba(shrink(fr, nw, nh), colors)
        x = int(round(anchor_x - head_cx))
        x = max(1, min(fw - 1 - nw, x))
        cell = Image.new("RGBA", (fw, fh), (0, 0, 0, 0))
        cell.alpha_composite(part, (x, fh - pad_bottom - nh))
        cells.append(outline(cell))
    return cells


def mirror_loop(img):
    """Image + son reflet : raccord parfait quand le fond boucle a l'horizontale."""
    out = Image.new("RGBA", (img.width * 2, img.height))
    out.paste(img, (0, 0))
    out.paste(img.transpose(Image.FLIP_LEFT_RIGHT), (img.width, 0))
    return out


def strip_ground(fr):
    """Retire un trait de sol sous l'objet : lignes du bas faites seulement de pixels sombres."""
    a = np.array(fr)
    for y in range(a.shape[0] - 1, -1, -1):
        op = a[y, :, 3] > 0
        if not op.any():
            continue
        lum = a[y, :, :3].mean(-1)
        if (lum[op] < 80).all():
            a[y, :, 3] = 0
        else:
            break
    return Image.fromarray(a)


def grid_cells(src, cols=8, rows=4, margin=0.03):
    """Coupe une planche de tuiles en cases (sans les traits de grille)."""
    W, H = src.size
    cw, ch = W / cols, H / rows
    cells = {}
    for r in range(rows):
        for c in range(cols):
            mx, my = cw * margin, ch * margin
            cells[(r, c)] = src.crop((int(c * cw + mx), int(r * ch + my), int((c + 1) * cw - mx), int((r + 1) * ch - my)))
    return cells


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("kind", choices=["far", "mid", "tiles", "sprite", "object"])
    ap.add_argument("args", nargs="+")
    ap.add_argument("--theme", default="kosice")
    ap.add_argument("--frames", type=int, default=1)
    ap.add_argument("--colors", type=int, default=0)
    ap.add_argument("--no-key", action="store_true")
    ap.add_argument("--mag-cut", type=int, default=90, help="seuil du fond magenta (monter pour garder du rose vif)")
    ap.add_argument("--tint-cut", type=int, default=25, help="seuil de correction des bords teintes de magenta")
    ap.add_argument("--key", default=None)
    ap.add_argument("--mirror", action="store_true", help="far/mid : ajoute le reflet pour boucler sans raccord")
    ap.add_argument("--strip-ground", action="store_true", help="sprite/object : retire un trait de sol")
    ap.add_argument("--grid", default=None, help="sprite/object : planche en grille LIGNESxCOLONNES (ex. 3x2)")
    ap.add_argument("--pick", default=None, help="images a garder, numerotees de 1 (ex. 1,2)")
    ap.add_argument("--head-px", type=float, default=0,
                    help="sprite : taille de la tete en pixels (meme taille dans toutes les poses)")
    ap.add_argument("--height-px", type=float, default=0,
                    help="sprite : hauteur du personnage debout (mediane des images), a la place de --head-px")
    ap.add_argument("--anchor-x", type=float, default=0, help="sprite : position x du centre de la tete dans la case")
    ap.add_argument("--uniform", action="store_true",
                    help="sprite/object : meme echelle pour toutes les images (garde les tailles relatives)")
    ap.add_argument("--cells", default=None,
                    help="tiles : tuile=ligne,colonne[,f] ... ex. '1=0,1 3=0,1,L 4=0,1,R 6=0,5 5=0,5,f' "
                         "(f : retournee, L/R : ajoute un bord gauche/droit) quand l'IA n'a pas respecte l'ordre")
    a = ap.parse_args()
    key = tuple(int(a.key[i:i + 2], 16) for i in (0, 2, 4)) if a.key else None

    if a.kind in SIZES:
        src = Image.open(a.args[0]).convert("RGBA")
        w, h = SIZES[a.kind]
        if a.kind in ("far", "mid"):
            # hauteur fixe, largeur libre (le fond boucle) : rien n'est rogne
            img = src if (a.kind == "far" or a.no_key) else key_out(src, key, mag_cut=a.mag_cut, tint_cut=a.tint_cut)
            nw = max(SCREEN_W_MIN, int(round(img.width * h / img.height)))
            img = img.resize((nw, h), Image.LANCZOS)
            al = np.array(img)
            al[..., 3] = 255 if a.kind == "far" else np.where(al[..., 3] >= 128, 255, 0)
            img = Image.fromarray(al)
            if a.mirror:
                img = mirror_loop(img)
            img = quantize_rgba(img, a.colors or 200)
        elif a.cells:
            img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
            src = src if a.no_key else key_out(src, key, mag_cut=a.mag_cut, tint_cut=a.tint_cut)
            cells = grid_cells(src)
            for spec in a.cells.split():
                idx, rc = spec.split("=")
                parts = rc.split(",")
                cell = cells[(int(parts[0]), int(parts[1]))]
                t = shrink(cell, 16, 16)
                flags = parts[2] if len(parts) > 2 else ""
                if "f" in flags:
                    t = t.transpose(Image.FLIP_LEFT_RIGHT)
                ta = np.array(t)
                if "L" in flags:
                    ta[:, 0, :] = (70, 62, 56, 255)
                if "R" in flags:
                    ta[:, 15, :] = (70, 62, 56, 255)
                n = int(idx)
                img.paste(Image.fromarray(ta), ((n % 8) * 16, (n // 8) * 16))
            img = quantize_rgba(img, a.colors or 96)
        else:  # tiles : grille 8 x 4 de tuiles dans l'ordre attendu
            img = src if a.no_key else key_out(src, key, mag_cut=a.mag_cut, tint_cut=a.tint_cut)
            img = shrink(img, w, h)
            img = quantize_rgba(img, a.colors or 96)
        out = os.path.join(ART, a.theme, a.kind + ".png")
        os.makedirs(os.path.dirname(out), exist_ok=True)
        img.save(out)
        print("ecrit", os.path.relpath(out, ROOT), img.size)
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
    src = src if a.no_key else key_out(src, key, mag_cut=a.mag_cut, tint_cut=a.tint_cut)
    src = remove_rules(src)
    if a.strip_ground:
        src = strip_ground(src)
    if a.grid:
        gr, gc = (int(v) for v in a.grid.lower().split("x"))
        frames = grid_frames(src, gr, gc)
    else:
        frames = frames_from(crop_alpha(src), a.frames)
    if a.pick:
        frames = [frames[int(i) - 1] for i in a.pick.split(",")]
    need_w = fw * len(frames)
    if sheet.width < need_w or sheet.height < (row + 1) * fh:
        big = Image.new("RGBA", (max(sheet.width, need_w), max(sheet.height, (row + 1) * fh)), (0, 0, 0, 0))
        big.paste(sheet, (0, 0))
        sheet = big
    # efface la ligne puis place les nouvelles images
    sheet.paste(Image.new("RGBA", (sheet.width, fh), (0, 0, 0, 0)), (0, row * fh))
    if a.strip_ground:
        frames = [strip_ground(fr) for fr in frames]
    if a.head_px or a.height_px:
        cells = fit_character(frames, fw, fh, a.colors or 48, a.head_px, a.anchor_x or fw * 0.55,
                              height_px=a.height_px)
    elif a.uniform:
        cells = fit_strip(frames, fw, fh, a.colors or 48)
    else:
        cells = [fit_frame(fr, fw, fh, a.colors or 48) for fr in frames]
    for i, cell in enumerate(cells):
        sheet.paste(cell, (i * fw, row * fh))
    sheet.save(sheet_path)
    print(f"ecrit {os.path.relpath(sheet_path, ROOT)} : ligne {row}, {len(frames)} image(s) de {fw}x{fh}")
    if a.kind == "sprite":
        print("Rappel : le nombre d'images par animation est fixe dans src/game/game.cpp (ANIM_LEN).")


if __name__ == "__main__":
    main()
