#!/usr/bin/env python3
"""Exporte les donnees de Zajka Run (images, polices, niveaux) pour la version web.

    python3 web-games/tools/export_run.py

Lit esp32/zajka-run/src/game/gen/assets_data.inc (ce que le firmware embarque, genere par
esp32/zajka-run/tools/build_assets.py) et ecrit web-games/zajka-run/data/assets.js.
A relancer apres chaque regeneration des assets cote ESP32.
"""
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cdata  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
SRC = os.path.join(REPO, "esp32", "zajka-run", "src", "game", "gen", "assets_data.inc")
OUT = os.path.join(REPO, "web-games", "zajka-run", "data", "assets.js")


def ref(s):
    return s.lstrip("&") if isinstance(s, str) and s != "nullptr" else None


def main():
    d = cdata.parse_inc(open(SRC, encoding="utf-8").read())
    images, sheets, themes, fonts = {}, {}, {}, {}
    for name, (typ, v) in d.items():
        if typ == "Image8":
            w, h, pal, px = v
            images[name] = {"w": w, "h": h, "pal": d[pal][1], "px": cdata.pack_b64(d[px][1])}
        elif typ == "Sheet":
            img, fw, fh, cols = v
            sheets[name] = {"img": ref(img), "fw": fw, "fh": fh, "cols": cols}
        elif typ == "Theme":
            far, mid, tiles, check, decor, u1, u2 = v
            themes[name[len("theme_"):]] = {"far": ref(far), "mid": ref(mid), "tiles": ref(tiles),
                                            "check": ref(check), "decor": ref(decor), "under": u1, "under2": u2}
        elif typ == "Font":
            height, count, cp, w, off, bits = v
            fonts[name] = {"height": height, "cp": d[cp][1], "w": d[w][1], "off": d[off][1],
                           "bits": cdata.pack_b64(d[bits][1])}
    theme_of = {}
    for tn, t in themes.items():
        theme_of["&theme_" + tn] = tn
    levels = []
    for lv in d["ALL_LEVELS"][1]:
        _, v = d[ref(lv)]
        name, subtitle, hint, theme, music, npc, weather, nxt, w, h, rows = v
        levels.append({"name": name, "subtitle": subtitle, "hint": hint, "theme": theme_of[theme],
                       "music": music, "npc": ref(npc), "weather": weather, "next": nxt,
                       "w": w, "h": h, "rows": d[rows][1]})
    data = {"images": images, "sheets": sheets, "themes": themes, "fonts": fonts, "levels": levels}
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        f.write("// Genere par web-games/tools/export_run.py a partir de esp32/zajka-run : ne pas modifier a la main.\n")
        f.write("globalThis.ZR_DATA = " + json.dumps(data, ensure_ascii=False, separators=(",", ":")) + ";\n")
    print(f"{os.path.relpath(OUT, REPO)} : {len(images)} images, {len(sheets)} planches, "
          f"{len(themes)} themes, {len(fonts)} polices, {len(levels)} niveaux, {os.path.getsize(OUT) // 1024} Ko")


if __name__ == "__main__":
    main()
