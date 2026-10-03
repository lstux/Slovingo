#!/usr/bin/env python3
"""check_ui_keys.py

Find the `ui` keys of a lang.json that the front-end uses but the
language does not define.

Why: every interface string (buttons, settings, exercise screens...)
is read from lang.json's `ui` map, with an English fallback hard-coded
in the JavaScript. A missing key is therefore silent -- the learner just
sees English text in a French or Slovak course. This script makes the
gap visible; build.py calls it and prints a warning.

The "reference" list of keys is not maintained by hand: it is read
from the front-end source itself (static/*.js and static/index.html),
so it cannot drift from the code. Three usages are recognised:

    uiLabel("key", "English default")          (exercises.js and co.)
    (LANG.ui && LANG.ui.key) || "English default"
    <span data-ui="key">English default</span>  (filled by app.js)

Keys built dynamically at runtime cannot be seen; they are not
reported, and neither are data-ui placeholders whose text is computed
(`${...}`). Comments are ignored.

Usage:
    # What is missing in one language (with the English default text):
    python3 check_ui_keys.py --lang-dir langs/de-fr

    # Same, plus a ready-to-paste JSON snippet to translate:
    python3 check_ui_keys.py --lang-dir langs/de-fr --template

    # Fail (exit 1) when something is missing, e.g. in CI:
    python3 check_ui_keys.py --lang-dir langs/de-fr --strict
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any

# English default text: a double-quoted JS string (escapes allowed).
_STRING = r'"((?:[^"\\]|\\.)*)"'

UI_LABEL_RE = re.compile(r'uiLabel\(\s*"([a-z0-9_]+)"\s*(?:,\s*' + _STRING + r')?')
LANG_UI_RE = re.compile(r'LANG\.ui\.([a-z0-9_]+)(?:\)?\s*\|\|\s*' + _STRING + r')?')
DATA_UI_RE = re.compile(r'data-ui="([a-z0-9_-]+)"(?:[^>]*>([^<]*)<)?')

BLOCK_COMMENT_RE = re.compile(r"/\*.*?\*/", re.DOTALL)
HTML_COMMENT_RE = re.compile(r"<!--.*?-->", re.DOTALL)
LINE_COMMENT_RE = re.compile(r"(^|\s)//(\s.*|)$", re.MULTILINE)


def _strip_comments(text: str, filename: str) -> str:
    """Drop comments so that documentation like `[data-ui="key"]` in a
    docstring is not mistaken for a real use."""
    if filename.endswith(".html"):
        return HTML_COMMENT_RE.sub("", text)
    text = BLOCK_COMMENT_RE.sub("", text)
    return LINE_COMMENT_RE.sub("", text)


def collect_used_keys(static_dir: Path) -> dict[str, dict[str, Any]]:
    """Scan the front-end source for `ui` keys.

    Returns:
        {key: {"default": English text or None, "files": [file names]}}
    """
    used: dict[str, dict[str, Any]] = {}
    sources = sorted(static_dir.glob("*.js")) + [static_dir / "index.html"]
    for path in sources:
        if not path.exists():
            continue
        text = _strip_comments(path.read_text(encoding="utf-8"), path.name)
        for regex in (UI_LABEL_RE, LANG_UI_RE, DATA_UI_RE):
            for match in regex.finditer(text):
                key, default = match.group(1), match.group(2)
                if default and "${" in default:
                    # data-ui placeholder whose text is built at runtime
                    # (e.g. app.js's "continue-label"): not a translatable key.
                    continue
                entry = used.setdefault(key, {"default": None, "files": []})
                # Kept as written: a leading/trailing space is often
                # significant (" days" after a counter, "answer: " + answer).
                if default and default.strip() and entry["default"] is None:
                    entry["default"] = default
                if path.name not in entry["files"]:
                    entry["files"].append(path.name)
    return used


def missing_ui_keys(lang_cfg: dict[str, Any], static_dir: Path) -> dict[str, dict[str, Any]]:
    """The keys the front-end uses that lang_cfg["ui"] does not define
    (an empty string counts as missing: the front-end falls back too)."""
    defined = {key for key, value in (lang_cfg.get("ui") or {}).items() if value}
    used = collect_used_keys(static_dir)
    return {key: used[key] for key in sorted(used) if key not in defined}


def format_warning(lang_name: str, missing: dict[str, dict[str, Any]], lang_dir: Path) -> str:
    """One-paragraph warning for build.py."""
    keys = ", ".join(missing)
    return (
        f"Warning: {lang_name}/lang.json is missing {len(missing)} `ui` key(s) used by the "
        f"front-end; those texts will show in English: {keys}. "
        f"Details and a template to translate: "
        f"python3 src/check_ui_keys.py --lang-dir {lang_dir} --template"
    )


def _print_report(missing: dict[str, dict[str, Any]], template: bool) -> None:
    width = max(len(key) for key in missing)
    for key, info in missing.items():
        default = f'"{info["default"]}"' if info["default"] else "(no default text)"
        print(f"  {key:<{width}}  {default}  [{', '.join(info['files'])}]")
    if template:
        print("\nTemplate (English defaults -- translate the values, then paste into lang.json `ui`):\n")
        items = list(missing.items())
        for index, (key, info) in enumerate(items):
            comma = "," if index < len(items) - 1 else ""
            print(f"    {json.dumps(key)}: {json.dumps(info['default'] or '', ensure_ascii=False)}{comma}")


def main() -> None:
    parser = argparse.ArgumentParser(description="List the lang.json `ui` keys the front-end needs but the language lacks.")
    parser.add_argument("--lang-dir", type=Path, required=True, help="Language directory containing lang.json (e.g. langs/de-fr)")
    parser.add_argument(
        "--static-dir", type=Path, default=Path(__file__).resolve().parent.parent / "static",
        help="Front-end directory to scan (default: static/ next to src/)",
    )
    parser.add_argument("--template", action="store_true", help="Also print a JSON snippet of the missing keys with their English defaults")
    parser.add_argument("--strict", action="store_true", help="Exit with status 1 when keys are missing")
    args = parser.parse_args()

    with (args.lang_dir / "lang.json").open(encoding="utf-8") as f:
        lang_cfg = json.load(f)

    missing = missing_ui_keys(lang_cfg, args.static_dir)
    if not missing:
        print(f"{args.lang_dir.name}: all `ui` keys used by the front-end are defined.")
        return
    print(f"{args.lang_dir.name}: {len(missing)} `ui` key(s) used by the front-end are missing from lang.json:")
    _print_report(missing, args.template)
    if args.strict:
        sys.exit(1)


if __name__ == "__main__":
    main()
