#!/usr/bin/env python3

"""
gen_lang_config.py
===================

Génère lang-config.js (window.LANG_CONFIG = {...}) à partir de
lang.json, pour que le JS embarqué (app.js, exercises.js, progress.js)
lise exactement la même configuration que les scripts Python — sans
introduire de build step ni de dépendance supplémentaire : c'est un
simple fichier .js statique, régénéré à chaque publication.

Usage :
    python3 gen_lang_config.py                    # écrit sur stdout
    python3 gen_lang_config.py --out lang-config.js
    python3 gen_lang_config.py --config autre.json --out lang-config.js
"""

import argparse
import json
import sys
from pathlib import Path

from langconfig import load_config


def render(config):
    payload = json.dumps(config, ensure_ascii=False, indent=2)
    return (
        "// Généré automatiquement depuis lang.json par gen_lang_config.py.\n"
        "// Ne pas éditer à la main : modifier lang.json, puis régénérer.\n"
        f"window.LANG_CONFIG = {payload};\n"
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--config", default=None, help="Chemin vers lang.json")
    parser.add_argument("--out", default=None, help="Fichier de sortie (stdout par défaut)")
    args = parser.parse_args()

    config = load_config(args.config)
    text = render(config)

    if args.out:
        Path(args.out).write_text(text, encoding="utf-8")
        print(f"✓ lang-config.js généré : {args.out}", file=sys.stderr)
    else:
        sys.stdout.write(text)


if __name__ == "__main__":
    main()
