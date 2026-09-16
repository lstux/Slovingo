#!/usr/bin/env python3
"""gen_service_worker.py

Generate service-worker.js from service-worker.template.js, by
substituting CACHE_VERSION (site.storage_prefix + "-v1") and SCOPE
(site.url_path). The rest of the service worker (cache strategy,
CORE_ASSETS...) is generic and never changes per language.

Usage:
    python3 gen_service_worker.py --lang lang.json -o dist/service-worker.js
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

TEMPLATE_PATH = Path(__file__).parent / "service-worker.template.js"


def render(lang_cfg: dict[str, Any]) -> str:
    text = TEMPLATE_PATH.read_text(encoding="utf-8")
    cache_version = f"{lang_cfg['site']['storage_prefix']}-v1"
    scope = lang_cfg["site"]["url_path"]
    return text.replace("__CACHE_VERSION__", cache_version).replace("__SCOPE__", scope)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--lang", type=Path, required=True, help="Path to lang.json")
    parser.add_argument("-o", "--output", type=Path, required=True, help="Output service-worker.js path")
    args = parser.parse_args()

    with args.lang.open(encoding="utf-8") as f:
        lang_cfg = json.load(f)

    text = render(lang_cfg)
    args.output.write_text(text, encoding="utf-8")
    print(f"Wrote {args.output}")


if __name__ == "__main__":
    main()
