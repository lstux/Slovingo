#!/usr/bin/env python3

"""
langconfig.py
=============

Chargement de la configuration de langue (lang.json), partagée par
smd2html.py et smd2exercises.py pour ne plus dupliquer les mots-clés
de détection de colonne, les libellés de catégories/séries, etc.

Un nouveau cours (breton, anglais...) se crée en copiant un lang.json
existant (voir langs/sk/lang.json) dans langs/<code>/lang.json et en
changeant ses valeurs — aucun code à toucher.

Résolution du chemin de lang.json, par ordre de priorité :
1. L'argument `path` passé explicitement à load_config().
2. La variable d'environnement LANG_CONFIG_PATH (positionnée par
   publish.sh selon le dossier de langue choisi avec -l).
3. src/lang.json, pour un usage ponctuel d'un script (smd2html.py...)
   sans passer par publish.sh, en gardant un lang.json à cet endroit.
"""

import json
import os
from pathlib import Path

DEFAULT_CONFIG_PATH = Path(__file__).parent / "lang.json"

_cache = {}


def load_config(path=None):
    """
    Charge lang.json (mis en cache par chemin résolu, pour éviter de
    relire le fichier à chaque appel dans un même process).
    """

    if path is None:
        path = os.environ.get("LANG_CONFIG_PATH")

    resolved = Path(path) if path else DEFAULT_CONFIG_PATH
    resolved = resolved.resolve()

    if resolved not in _cache:
        if not resolved.is_file():
            raise FileNotFoundError(
                f"lang.json introuvable : {resolved}\n"
                "-> passe par publish.sh -l langs/<code>, ou positionne "
                "LANG_CONFIG_PATH, ou place un lang.json à côté de langconfig.py."
            )
        with resolved.open(encoding="utf-8") as f:
            _cache[resolved] = json.load(f)

    return _cache[resolved]


def is_header_match(text, lang_cfg):
    """
    Vérifie si un en-tête de colonne désigne la langue décrite par
    lang_cfg (target_lang ou native_lang) : comparaison exacte contre
    "headers", puis recherche de racine parmi "header_roots".
    """

    normalized = text.strip().lower()

    if normalized in lang_cfg.get("headers", []):
        return True

    return any(
        root in normalized
        for root in lang_cfg.get("header_roots", [])
    )


def fiche_footer(config):
    """
    Texte de pied de page d'une fiche individuelle, construit depuis
    le gabarit "site.fiche_footer" de lang.json.
    """

    template = config.get("site", {}).get("fiche_footer", "")
    return template.format(
        target_name=config["target_lang"]["name"],
        target_tts=config["target_lang"]["tts_code"],
    )
