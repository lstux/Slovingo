#!/usr/bin/env python3

"""
lang_field.py
=============

Lit une valeur de lang.json depuis le shell (publish.sh), sans
introduire de dépendance à jq.

Usage :
    python3 lang_field.py site.title
    python3 lang_field.py target_lang.flag
    python3 lang_field.py --fiche-footer     # texte de pied de page d'une fiche
    python3 lang_field.py --index-footer     # texte de pied de page de l'index
"""

import sys

from langconfig import load_config, fiche_footer


def index_footer(config):
    """
    Texte de pied de page de la page d'index — reprend le titre du
    site tel quel (ex. "Ahoj Slovenčina", "Demat Brezhoneg") suivi de
    la mention de la synthèse vocale, sur le modèle de fiche_footer().
    """

    return f"{config['site']['title']} \u2022 Synthèse vocale {config['target_lang']['tts_code']}"


def main():
    if len(sys.argv) != 2:
        print(
            "usage: lang_field.py <clé.pointée> | --fiche-footer | --index-footer",
            file=sys.stderr,
        )
        sys.exit(1)

    config = load_config()
    arg = sys.argv[1]

    if arg == "--fiche-footer":
        print(fiche_footer(config))
        return
    if arg == "--index-footer":
        print(index_footer(config))
        return

    value = config
    for part in arg.split("."):
        value = value[part]
    print(value)


if __name__ == "__main__":
    main()
