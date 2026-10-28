#!/usr/bin/env python3
"""Exporte les tables sonores (effets, morceaux) d'un jeu ESP32 pour la version web.

    python3 web-games/tools/export_audio.py run     # esp32/zajka-run/src/game/audio.cpp
    python3 web-games/tools/export_audio.py quest   # esp32/zajka-quest/src/game/audio.cpp

Le moteur de synthese est porte une fois pour toutes dans web-games/common/audio.js ; ici on ne
lit que les donnees : tables d'effets (Step), melodies (Note), accords (Chord) et morceaux (Song).
"""
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cdata  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
GAMES = {"run": ("zajka-run", "ZR_AUDIO"), "quest": ("zajka-quest", "ZQ_AUDIO")}


def main(game):
    folder, var = GAMES[game]
    src = os.path.join(REPO, "esp32", folder, "src", "game", "audio.cpp")
    out = os.path.join(REPO, "web-games", folder, "data", "audio_data.js")
    text = re.sub(r"//[^\n]*", "", open(src, encoding="utf-8").read())
    d = cdata.parse_inc(text)
    chords = {m.group(1): [int(m.group(2)), int(m.group(3)), int(m.group(4))]
              for m in re.finditer(r"(\w+)\s*=\s*\{\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\}", text)}
    sfx = []
    for name in d["SFX_TABLE"][1]:
        steps = d[name][1]
        sfx.append([s for s in steps if s[3] != 0])   # le dernier pas {0,...} sert de terminateur en C
    songs = []
    pat = re.compile(r"\{\s*(\w+)\s*,\s*\(int\)\(sizeof\(\w+\)\s*/\s*sizeof\(\w+\[0\]\)\)\s*,\s*(\w+)\s*,\s*"
                     r"AUDIO_RATE\s*\*\s*60\s*/\s*(\d+)\s*/\s*4\s*,\s*(true|false)\s*,\s*(\d+)\s*\}")
    for m in pat.finditer(text):
        mel, bars, bpm, drums, wave = m.groups()
        songs.append({"mel": [n for n in d[mel][1]], "bars": [chords[b.lstrip("&")] for b in d[bars][1]],
                      "bpm": int(bpm), "drums": drums == "true", "melWave": int(wave)})
    if not songs:   # version a un seul morceau : constantes MELODY, BARS, SIXTEENTH
        bpm = int(re.search(r"SIXTEENTH\s*=\s*AUDIO_RATE\s*\*\s*60\s*/\s*(\d+)\s*/\s*4", text).group(1))
        wave = int(re.search(r"startNote\(v_\[0\],[^;]*?,\s*(\d+),\s*\d+\)", text).group(1))
        songs.append({"mel": d["MELODY"][1], "bars": [chords[b.lstrip("&")] for b in d["BARS"][1]],
                      "bpm": bpm, "drums": True, "melWave": wave})
    os.makedirs(os.path.dirname(out), exist_ok=True)
    with open(out, "w", encoding="utf-8") as f:
        f.write(f"// Genere par web-games/tools/export_audio.py a partir de esp32/{folder}/src/game/audio.cpp : "
                "ne pas modifier a la main.\n")
        f.write(f"globalThis.{var} = " + json.dumps({"sfx": sfx, "songs": songs}, separators=(",", ":")) + ";\n")
    print(f"{os.path.relpath(out, REPO)} : {len(sfx)} effets, {len(songs)} morceau(x)")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "run")
