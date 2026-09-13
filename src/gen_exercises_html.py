#!/usr/bin/env python3

"""
gen_exercises_html.py
======================

Génère exercises.html depuis exercises.template.html + lang.json
(titre de page, couleur de thème, footer). Le reste de la page est
fixe : structure, scripts, PWA.

Usage :
    python3 gen_exercises_html.py                       # stdout
    python3 gen_exercises_html.py --out exercises.html
"""

import argparse
import sys
from pathlib import Path

from langconfig import load_config

TEMPLATE_PATH = Path(__file__).parent / "exercises.template.html"


def elided_de(word):
    """"de slovaque" mais "d'anglais" — élision devant voyelle."""
    if word[:1].lower() in "aeiouyàâäéèêëïîôöùûü":
        return f"d'{word}"
    return f"de {word}"


def render(config):
    text = TEMPLATE_PATH.read_text(encoding="utf-8")
    title = f"{config['site']['title']} — Exercices"
    theme_color = config["site"].get("theme_color", "#0b4ea2")
    footer = f"Exercices {elided_de(config['target_lang']['name'])} \u2022 généré depuis les fiches SMD"
    return (
        text
        .replace("__TITLE__", title)
        .replace("__THEME_COLOR__", theme_color)
        .replace("__FOOTER__", footer)
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", default=None, help="Fichier de sortie (stdout par défaut)")
    args = parser.parse_args()

    config = load_config()
    text = render(config)

    if args.out:
        Path(args.out).write_text(text, encoding="utf-8")
        print(f"✓ exercises.html généré : {args.out}", file=sys.stderr)
    else:
        sys.stdout.write(text)


if __name__ == "__main__":
    main()
