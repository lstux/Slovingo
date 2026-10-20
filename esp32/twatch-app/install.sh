#!/usr/bin/env bash
# Ajoute l'app Slovingo au firmware d'origine (d03n3rfr1tz3/TTGO.T-Watch.2020).
#
# Usage : ./install.sh /chemin/vers/TTGO.T-Watch.2020
#
# Ce que fait le script :
#   1. copie slovingo/            -> <firmware>/src/app/slovingo
#   2. copie conf/slovingo.json.example -> <firmware>/conf/
#   3. ajoute "app/slovingo" à NOT_ESSENTIAL dans support/apps_off.py
#      (les cartes à petite flash n'embarquent pas l'app)
#   4. ajoute data/*.json à .gitignore (tes mots de passe Wi-Fi ne doivent pas être commités)
# Il peut être relancé sans risque.
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FW="${1:-}"

if [ -z "$FW" ] || [ ! -f "$FW/platformio.ini" ] || [ ! -f "$FW/support/apps_off.py" ]; then
    echo "Usage : $0 /chemin/vers/TTGO.T-Watch.2020" >&2
    echo "(le dossier doit contenir platformio.ini et support/apps_off.py)" >&2
    exit 1
fi

mkdir -p "$FW/src/app" "$FW/conf"
rm -rf "$FW/src/app/slovingo"
cp -r "$HERE/slovingo" "$FW/src/app/slovingo"
cp "$HERE/conf/slovingo.json.example" "$FW/conf/slovingo.json.example"

python3 - "$FW/support/apps_off.py" <<'PY'
import re
import sys

path = sys.argv[1]
text = open(path, encoding="utf-8").read()
entry = '    "app/slovingo",\n'
m = re.search(r"^NOT_ESSENTIAL = \[\n(.*?)^\]", text, re.S | re.M)
if m is None:
    sys.exit("NOT_ESSENTIAL introuvable dans " + path)
if '"app/slovingo"' in m.group(1):
    print("apps_off.py : déjà à jour")
else:
    lines = m.group(1).splitlines(keepends=True) + [entry]
    lines.sort(key=lambda l: l.strip().strip(",").strip('"').lower())
    text = text[:m.start(1)] + "".join(lines) + text[m.end(1):]
    open(path, "w", encoding="utf-8").write(text)
    print("apps_off.py : app/slovingo ajouté à NOT_ESSENTIAL")
PY

if ! grep -qx 'data/\*.json' "$FW/.gitignore" 2>/dev/null; then
    printf '\ndata/*.json\n' >> "$FW/.gitignore"
    echo ".gitignore : data/*.json ajouté"
fi

echo "OK : app Slovingo installée dans $FW/src/app/slovingo"
