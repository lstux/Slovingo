#!/bin/bash
# Compare la version web de Zajka Quest au simulateur C++ du firmware (memes scripts, memes entrees) :
# journal des evenements (cartes, dialogues, objets, mots...), captures d'ecran (PPM) et son (WAV).
# Usage : compare_quest.sh [dossier de travail]
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
Q="$REPO/esp32/zajka-quest"
W="${1:-/tmp/zajka_compare_quest}"
mkdir -p "$W"
g++ -std=c++17 -O1 -ffp-contract=off -o "$W/sim_c" "$Q/sim/sim.cpp" "$Q"/src/game/*.cpp
status=0
run() {   # nom, options...
  local name="$1"; shift
  rm -rf "$W/$name"; mkdir -p "$W/$name/c" "$W/$name/j"
  # {SAVE} : un fichier de sauvegarde par version (partage entre les executions d'un meme scenario)
  local ca=() ja=()
  for a in "$@"; do ca+=("${a//\{SAVE\}/$W/$name/c.sav}"); ja+=("${a//\{SAVE\}/$W/$name/j.sav}"); done
  ZQ_TRACE=1 "$W/sim_c" "${ca[@]}" --out "$W/$name/c" > "$W/$name/c.txt" || true
  ZQ_TRACE=1 node "$HERE/sim_quest.js" "${ja[@]}" --out "$W/$name/j" > "$W/$name/j.txt" || true
  local d=$(diff "$W/$name/c.txt" "$W/$name/j.txt" | wc -l)
  local bad=0 n=0
  for f in "$W/$name"/c/*.ppm; do
    [ -e "$f" ] || continue
    n=$((n+1)); cmp -s "$f" "$W/$name/j/$(basename "$f")" || { bad=$((bad+1)); echo "   difference : $name/$(basename "$f")"; }
  done
  local wav=-
  if [ -f "$W/$name/c/audio.wav" ]; then wav=ok; cmp -s "$W/$name/c/audio.wav" "$W/$name/j/audio.wav" || wav=DIFFERENT; fi
  echo "$name : journal $d lignes differentes, captures $bad/$n differentes, son $wav"
  [ "$d" = 0 ] && [ "$bad" = 0 ] && [ "$wav" != DIFFERENT ] || status=1
}
S="$Q/sim"
run demo --script "$S/demo.txt" --every 20 --wav
run chapitre1 --script "$S/chapter.txt" --auto --every 150 --wav
run chapitre1_peek --script "$S/chapter.txt" --auto-peek --every 400
run chapitre1_wrong --script "$S/chapter.txt" --auto-wrong --every 400
run chapitre2 --chapter 2 --script "$S/chapter2.txt" --auto --every 150 --wav
# reprise d'une partie : le chapitre 1 enregistre sa partie, puis resume.txt la reprend (chapitre 2, Andrea derriere)
run chapitre1_save --script "$S/chapter.txt" --auto --every 1000 --save {SAVE}
cp "$W/chapitre1_save/c.sav" "$W/resume_c.sav"; cp "$W/chapitre1_save/j.sav" "$W/resume_j.sav"
cmp -s "$W/resume_c.sav" "$W/resume_j.sav" && echo "sauvegarde : octets identiques au firmware" || { echo "sauvegarde : DIFFERENTE"; status=1; }
rm -rf "$W/reprise"; mkdir -p "$W/reprise/c" "$W/reprise/j"
ZQ_TRACE=1 "$W/sim_c" --script "$S/resume.txt" --auto --save "$W/resume_c.sav" --expect reprise,carte:sucho_zahrada --out "$W/reprise/c" > "$W/reprise/c.txt"
ZQ_TRACE=1 node "$HERE/sim_quest.js" --script "$S/resume.txt" --auto --save "$W/resume_j.sav" --expect reprise,carte:sucho_zahrada --out "$W/reprise/j" > "$W/reprise/j.txt"
d=$(diff "$W/reprise/c.txt" "$W/reprise/j.txt" | wc -l); echo "reprise : journal $d lignes differentes"; [ "$d" = 0 ] || status=1
# menus : onglets du menu START, volume, ecran titre avec partie (Pokracovat / Nova hra)
run menus --script "$HERE/menus1.txt" --auto --every 25 --wav --save {SAVE}
cp "$W/menus/c.sav" "$W/menus_c.sav"; cp "$W/menus/j.sav" "$W/menus_j.sav"
for v in c j; do
  rm -rf "$W/titre_$v"; mkdir -p "$W/titre_$v"
done
ZQ_TRACE=1 "$W/sim_c" --script "$HERE/menus2.txt" --save "$W/menus_c.sav" --every 10 --out "$W/titre_c" > "$W/titre_c.txt"
ZQ_TRACE=1 node "$HERE/sim_quest.js" --script "$HERE/menus2.txt" --save "$W/menus_j.sav" --every 10 --out "$W/titre_j" > "$W/titre_j.txt"
d=$(diff "$W/titre_c.txt" "$W/titre_j.txt" | wc -l); b=0
for f in "$W"/titre_c/*.ppm; do cmp -s "$f" "$W/titre_j/$(basename "$f")" || b=$((b+1)); done
echo "titre avec partie : journal $d lignes differentes, captures $b differentes"; [ "$d" = 0 ] && [ "$b" = 0 ] || status=1
exit $status
