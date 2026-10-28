#!/bin/bash
# Compare la version web de Zajka Run au simulateur C++ du firmware (meme robot, memes entrees) :
# etat du joueur a chaque image, captures d'ecran et son. Usage : compare_run.sh [dossier de travail]
#
# Le simulateur C++ est compile ici avec un volume non nul (sinon son WAV est muet).
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
W="${1:-/tmp/zajka_compare_run}"
mkdir -p "$W"
sed -e "s#\"../src/game/game.h\"#\"$REPO/esp32/zajka-run/src/game/game.h\"#" \
    -e 's#static Game game;#static Game game; audio.setVolume(2);#' \
    "$REPO/esp32/zajka-run/sim/sim.cpp" > "$W/sim_patched.cpp"
g++ -std=c++17 -O1 -ffp-contract=off -o "$W/sim_c" "$W/sim_patched.cpp" "$REPO"/esp32/zajka-run/src/game/*.cpp
status=0
run() {   # nom, options...
  local name="$1"; shift
  rm -rf "$W/$name"; mkdir -p "$W/$name/c" "$W/$name/j"
  ZR_TRACE=1 "$W/sim_c" "$@" --out "$W/$name/c" > "$W/$name/c.txt"
  ZR_TRACE=1 node "$HERE/sim_run.js" "$@" --out "$W/$name/j" > "$W/$name/j.txt"
  local d=$(diff "$W/$name/c.txt" "$W/$name/j.txt" | wc -l)
  local bad=0 n=0
  for f in "$W/$name"/c/*.ppm; do
    n=$((n+1)); cmp -s "$f" "$W/$name/j/$(basename "$f")" || { bad=$((bad+1)); echo "   difference : $name/$(basename "$f")"; }
  done
  local wav=ok
  if [ -f "$W/$name/c/audio.wav" ]; then cmp -s "$W/$name/c/audio.wav" "$W/$name/j/audio.wav" || wav=DIFFERENT; else wav=-; fi
  echo "$name : trace $d lignes differentes, captures $bad/$n differentes, son $wav"
  [ "$d" = 0 ] && [ "$bad" = 0 ] && [ "$wav" != DIFFERENT ] || status=1
}
for L in 1 2 3 4; do
  run "bot_niveau$L" --bot --level $L --frames 7000 --every 250 --wav
done
run "bot_andrea2" --bot --level 2 --andrea --frames 5000 --every 250
run "bot_andrea4" --bot --level 4 --andrea --frames 5000 --every 250
exit $status
