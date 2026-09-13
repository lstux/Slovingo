#!/usr/bin/env python3

"""
gen_service_worker.py
======================

Génère service-worker.js depuis service-worker.template.js, en
substituant CACHE_VERSION (site.storage_prefix + "-v1") et SCOPE
(site.url_path). Le reste du service worker (stratégie de cache,
CORE_ASSETS...) est générique et ne change jamais.

Usage :
    python3 gen_service_worker.py                          # stdout
    python3 gen_service_worker.py --out service-worker.js
"""

import argparse
import sys
from pathlib import Path

from langconfig import load_config

TEMPLATE_PATH = Path(__file__).parent / "service-worker.template.js"


def render(config):
    text = TEMPLATE_PATH.read_text(encoding="utf-8")
    cache_version = f"{config['site']['storage_prefix']}-v1"
    scope = config["site"]["url_path"]
    return text.replace("__CACHE_VERSION__", cache_version).replace("__SCOPE__", scope)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", default=None, help="Fichier de sortie (stdout par défaut)")
    args = parser.parse_args()

    config = load_config()
    text = render(config)

    if args.out:
        Path(args.out).write_text(text, encoding="utf-8")
        print(f"✓ service-worker.js généré : {args.out}", file=sys.stderr)
    else:
        sys.stdout.write(text)


if __name__ == "__main__":
    main()
