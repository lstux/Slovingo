#!/usr/bin/env python3
"""Exporte les donnees de Zajka Quest (images, polices, cartes, cours, scenes) pour la version web.

    python3 web-games/tools/export_quest.py

Lit esp32/zajka-quest/src/game/gen/ (assets_data.inc, course_data.inc, assets.h, scripts.h : ce que
le firmware embarque) et ecrit web-games/zajka-quest/data/assets.js et course.js.
A relancer apres chaque regeneration des assets ou du cours cote ESP32 (build_assets.py, build_course.py).
"""
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cdata  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
GAME = os.path.join(REPO, "esp32", "zajka-quest", "src", "game")
GEN = os.path.join(GAME, "gen")
OUT = os.path.join(REPO, "web-games", "zajka-quest", "data")


def read(path):
    return open(path, encoding="utf-8").read()


def strip_comments(t):
    return re.sub(r'//[^\n]*', '', t)


def enum_values(text):
    """Toutes les valeurs d'enum (enum X : t { A, B = 3, C }) et constexpr (constexpr int A = 3;)."""
    vals = {}
    text = strip_comments(text)
    for m in re.finditer(r'enum\s+(?:class\s+)?\w*\s*(?::\s*[\w ]+?)?\s*\{([^}]*)\}', text):
        n = 0
        for item in m.group(1).split(','):
            item = item.strip()
            if not item:
                continue
            if '=' in item:
                name, expr = [x.strip() for x in item.split('=', 1)]
                n = eval(expr, {"__builtins__": {}}) if re.fullmatch(r'[\d\s<>|()-]+', expr) else vals[expr]
            else:
                name = item
            vals[name] = n
            n += 1
    for m in re.finditer(r'constexpr\s+\w+\s+(\w+)\s*=\s*(-?\d+)\s*;', text):
        vals[m.group(1)] = int(m.group(2))
    return vals


def ref(s):
    return s.lstrip("&") if isinstance(s, str) and s != "nullptr" else None


def main():
    enums = {}
    for f in (os.path.join(GAME, "world.h"), os.path.join(GAME, "course.h"), os.path.join(GAME, "input.h"),
              os.path.join(GEN, "assets.h"), os.path.join(GEN, "scripts.h")):
        enums.update(enum_values(read(f)))

    def num(v):
        if isinstance(v, str):
            if v == "nullptr":
                return -1
            return enums[v]
        return v

    # ---------------------------------------------------------------- images, polices, cartes
    d = cdata.parse_inc(strip_comments(read(os.path.join(GEN, "assets_data.inc"))))
    images, sheets, fonts, tilesets, maps = {}, {}, {}, {}, {}
    for name, (typ, v) in d.items():
        if typ == "Image8":
            w, h, pal, px = v
            images[name] = {"w": w, "h": h, "pal": d[pal][1], "px": cdata.pack_b64(d[px][1])}
        elif typ == "Sheet":
            img, fw, fh, cols = v
            sheets[name] = {"img": ref(img), "fw": fw, "fh": fh, "cols": cols}
        elif typ == "Font":
            height, count, cp, w, off, bits = v
            fonts[name] = {"height": height, "cp": d[cp][1], "w": d[w][1], "off": d[off][1],
                           "bits": cdata.pack_b64(d[bits][1])}
        elif typ == "Tileset":
            img, kinds, count = v
            tilesets[name] = {"img": ref(img), "kinds": [num(k) for k in d[kinds][1]], "count": count}
    portraits = [ref(p) for p in d["PORTRAITS"][1]]
    items = [{"sk": a, "fr": b} for a, b in d["ITEMS"][1]]
    map_order = []
    for m in d["ALL_MAPS"][1]:
        map_order.append(ref(m))
    for name in map_order:
        _, v = d[name]
        mid, mname, tiles, w, h, base, over, marks, nmarks, music, night = v
        mk = []
        for r in d[marks][1]:
            mtype, x, y, mdir, target, tx, ty, npc, say_ep, say_from, say_count, tag, item, pickup, word = [num(z) for z in r]
            mk.append({"type": mtype, "x": x, "y": y, "dir": mdir, "target": target, "tx": tx, "ty": ty, "npc": npc,
                       "sayEp": say_ep, "sayFrom": say_from, "sayCount": say_count, "tag": tag, "item": item,
                       "pickup": pickup, "word": word})
        maps[name] = {"id": mid, "name": mname, "tiles": ref(tiles), "w": w, "h": h, "base": d[base][1],
                      "over": d[over][1] if over != "nullptr" else None, "marks": mk, "music": music, "night": night}
    sign_words = d["SIGN_WORDS"][1]
    pick_count = int(re.search(r'const int NUM_PICKUPS = (\d+);', read(os.path.join(GEN, "assets_data.inc"))).group(1))
    consts = {k: v for k, v in enums.items() if k.split("_")[0] in ("SPK", "DIR", "TK", "M", "TL", "ITEM", "TAG", "MAP", "SC", "CH", "BTN")
              and k != "SPK_COUNT"}
    assets = {"images": images, "sheets": sheets, "fonts": fonts, "tilesets": tilesets, "portraits": portraits,
              "items": items, "maps": [dict(maps[n], key=n) for n in map_order], "signWords": sign_words,
              "numPickups": pick_count, "consts": consts, "numSpeakers": enums["SPK_COUNT"]}

    os.makedirs(OUT, exist_ok=True)
    with open(os.path.join(OUT, "assets.js"), "w", encoding="utf-8") as f:
        f.write("// Genere par web-games/tools/export_quest.py a partir de esp32/zajka-quest : ne pas modifier a la main.\n")
        f.write("globalThis.ZQ_DATA = " + json.dumps(assets, ensure_ascii=False, separators=(",", ":")) + ";\n")

    # ---------------------------------------------------------------- cours et scenes
    c = cdata.parse_inc(strip_comments(read(os.path.join(GEN, "course_data.inc"))))

    def line(r):
        speaker, sk, fr, gloss, ngloss, help_, choice = r
        out = {"s": num(speaker), "sk": sk, "fr": fr}
        if help_ != "nullptr":
            out["help"] = help_
        if choice != "nullptr":
            _, cv = c[ref(choice)]
            ctype, ex, items_name, n = cv
            out["choice"] = {"type": num(ctype), "items": c[items_name][1][:n]}
        return out

    def lines(name, n):
        if name == "nullptr":
            return []
        return [line(r) for r in c[name][1][:n]]

    episodes = []
    for e in c["COURSE_EPISODES"][1]:
        _, v = c[ref(e)]
        (eid, series, series_num, ep_num, title, place, dialog, ndialog, extra, nextra, words, nwords,
         exercises, nexercises, bonus, nbonus) = v
        episodes.append({"id": eid, "series": series, "seriesNum": series_num, "episode": ep_num, "title": title,
                         "place": place, "dialog": lines(dialog, ndialog), "extra": lines(extra, nextra),
                         "bonus": lines(bonus, nbonus)})
    vocab = [{"sk": a, "fr": b} for a, b in c["COURSE_VOCAB"][1]]
    scripts = [lines(ref(name), n) for name, n in c["SCRIPTS"][1]]
    text = read(os.path.join(GEN, "course_data.inc"))
    default_name = re.search(r'COURSE_DEFAULT_NAME = "([^"]*)"', text).group(1)
    course = {"episodes": episodes, "vocab": vocab, "scripts": scripts, "defaultName": default_name,
              "speakerNames": c["SPEAKER_NAMES"][1]}
    with open(os.path.join(OUT, "course.js"), "w", encoding="utf-8") as f:
        f.write("// Genere par web-games/tools/export_quest.py a partir de esp32/zajka-quest : ne pas modifier a la main.\n")
        f.write("globalThis.ZQ_COURSE = " + json.dumps(course, ensure_ascii=False, separators=(",", ":")) + ";\n")
    size = sum(os.path.getsize(os.path.join(OUT, n)) for n in ("assets.js", "course.js"))
    print(f"web-games/zajka-quest/data : {len(images)} images, {len(maps)} cartes, {len(episodes)} episodes, "
          f"{len(scripts)} scenes, {len(vocab)} mots, {size // 1024} Ko")


if __name__ == "__main__":
    main()
