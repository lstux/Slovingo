#!/usr/bin/env python3

"""
gen_manifest.py
===============

Génère manifest.json (manifeste PWA) depuis lang.json. Les champs qui
dépendent du cours (nom, description, chemin de déploiement) viennent
de la config ; le reste (icônes, orientation, affichage) est une
convention d'app fixe, indépendante de la langue apprise.

Usage :
    python3 gen_manifest.py                     # écrit sur stdout
    python3 gen_manifest.py --out manifest.json
"""

import argparse
import json
import sys
from pathlib import Path

from langconfig import load_config


def build_manifest(config):
    site = config["site"]
    return {
        "name": site["title"],
        "short_name": site.get("short_name", site["title"]),
        "description": site.get("description", ""),
        "start_url": site["url_path"] + "index.html",
        "scope": site["url_path"],
        "display": "standalone",
        "orientation": "portrait-primary",
        "background_color": site.get("theme_color", "#0b4ea2"),
        "theme_color": site.get("theme_color", "#0b4ea2"),
        "lang": "fr",
        "icons": [
            {"src": "icons/icon-192.png", "sizes": "192x192", "type": "image/png", "purpose": "any"},
            {"src": "icons/icon-512.png", "sizes": "512x512", "type": "image/png", "purpose": "any"},
            {"src": "icons/icon-maskable-512.png", "sizes": "512x512", "type": "image/png", "purpose": "maskable"},
        ],
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", default=None, help="Fichier de sortie (stdout par défaut)")
    args = parser.parse_args()

    config = load_config()
    manifest = build_manifest(config)
    text = json.dumps(manifest, ensure_ascii=False, indent=2) + "\n"

    if args.out:
        Path(args.out).write_text(text, encoding="utf-8")
        print(f"✓ manifest.json généré : {args.out}", file=sys.stderr)
    else:
        sys.stdout.write(text)


if __name__ == "__main__":
    main()
