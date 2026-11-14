#!/usr/bin/env python3
"""order_tokens.py

Rework the "put the words in order" exercises of a course's manual
exercise files (exercises/*.exercises.json) whose chips are big blocks
("Ty si tu?" / "V Košiciach?" / "V električke?"): three chips to order
is no exercise at all.

For each such exercise, on both sides (tokens_l1 and tokens_l2):

- the target sentence is cut back into words, one chip per word,
  punctuation kept on its word. A hand-made group of two words is kept
  when its first word is a short link word (a preposition, an article:
  "na západe.", "du thé"), as those read as one unit;
- a few parasite words are added after it: words of another sentence of
  the same file (the one sharing the fewest words with the target), not
  a capitalised word, without punctuation, never a word of the target.

Exercises already cut word by word (with or without parasites) are left
alone, and so are files with "sync": "off".

    python3 src/order_tokens.py --lang-dir ../Slovingo-sk-fr-friends --parasites 2-3
    python3 src/order_tokens.py --lang-dir ../Slovingo-sk-fr-kids --parasites 1-2 --apply

Dry run by default. Each answer is checked: the target chips, joined,
must give back the sentence exactly.
"""

from __future__ import annotations

import argparse
import json
import random
import re
import sys
from pathlib import Path
from typing import Any

PUNCT = ".,!?;:…«»\"“”„()–—"
MAX_LINK_WORD = 2          # "v", "na", "do", "le", "du", "à"...
LINK_WORDS = {"les", "des", "une", "aux", "sur", "dans", "pod", "nad", "pri", "pre", "bez", "cez", "zo", "so", "vo", "ku"}


def bare(word: str) -> str:
    """'Košiciach?' -> 'košiciach' (for comparisons and parasites)."""
    return word.strip(PUNCT).lower()


def split_target(tokens: list[str], text: str) -> tuple[list[str], list[str]] | None:
    """(target chips, parasite chips), or None if the chips don't start
    with `text`."""
    acc: list[str] = []
    for i, tok in enumerate(tokens):
        acc.append(tok)
        if " ".join(acc).split() == text.split():
            return tokens[:i + 1], tokens[i + 1:]
    return None


def keeps_group(chip: str) -> bool:
    words = chip.split()
    if len(words) != 2:
        return False
    first = bare(words[0])
    return len(first) <= MAX_LINK_WORD or first in LINK_WORDS or first.endswith("'")


def rechip(target_chips: list[str]) -> list[str]:
    out: list[str] = []
    for chip in target_chips:
        if len(chip.split()) == 1 or keeps_group(chip):
            out.append(chip)
        else:
            out.extend(chip.split())
    return out


def is_blocky(target_chips: list[str]) -> bool:
    return any(len(c.split()) > 1 and not keeps_group(c) for c in target_chips)


def parasite_words(target: str, source: str, n: int, rng: random.Random) -> list[str]:
    """n words of `source` that are not in `target`: lowercase words only
    (a capital gives away a sentence start, and a name can't be
    lowercased), no punctuation, no duplicates."""
    taken = {bare(w) for w in target.split()}
    words = source.split()[1:]
    seen: set[str] = set()
    pool = []
    for w in words:
        b = bare(w)
        if not b or b in taken or b in seen or not w.strip(PUNCT)[:1].islower():
            continue
        seen.add(b)
        pool.append(w.strip(PUNCT))
    rng.shuffle(pool)
    return pool[:n]


def pick_source(ex: dict[str, Any], pairs: list[tuple[str, str]]) -> tuple[str, str] | None:
    """The other sentence of the file sharing the fewest words with the target."""
    target_words = {bare(w) for w in ex["l2"].split()}
    others = [p for p in pairs if p != (ex["l1"], ex["l2"]) and len(p[1].split()) >= 3 and len(p[0].split()) >= 3]
    if not others:
        return None
    return min(others, key=lambda p: (len(target_words & {bare(w) for w in p[1].split()}), p[1]))


def file_pairs(exercises: list[dict[str, Any]]) -> list[tuple[str, str]]:
    """Sentence pairs of the file, from every exercise that has plain l1/l2."""
    seen, out = set(), []
    for ex in exercises:
        l1, l2 = ex.get("l1"), ex.get("l2")
        if isinstance(l1, str) and isinstance(l2, str) and (l1, l2) not in seen:
            seen.add((l1, l2))
            out.append((l1, l2))
    return out


def rework(ex: dict[str, Any], pairs: list[tuple[str, str]], n_range: tuple[int, int], rng: random.Random) -> dict[str, Any] | None:
    if ex.get("type") != "order":
        return None
    s1 = split_target(ex.get("tokens_l1") or ex["l1"].split(), ex["l1"])
    s2 = split_target(ex.get("tokens_l2") or ex["l2"].split(), ex["l2"])
    if s1 is None or s2 is None:
        return None
    if not (is_blocky(s1[0]) or is_blocky(s2[0])):
        return None  # already word by word: left alone
    new = json.loads(json.dumps(ex))
    t1, t2 = rechip(s1[0]), rechip(s2[0])
    p1, p2 = s1[1], s2[1]
    if not p1 and not p2:
        source = pick_source(ex, pairs)
        if source:
            n = rng.randint(*n_range)
            p2 = parasite_words(ex["l2"], source[1], n, rng)
            p1 = parasite_words(ex["l1"], source[0], len(p2) or n, rng)
    new["tokens_l1"] = t1 + p1
    new["tokens_l2"] = t2 + p2
    for chips, text in ((t1, ex["l1"]), (t2, ex["l2"])):
        if " ".join(chips).split() != text.split():
            raise ValueError(f"{ex.get('id')}: chips {chips} do not spell {text!r}")
    return new


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[1])
    ap.add_argument("--lang-dir", type=Path, required=True)
    ap.add_argument("--parasites", default="2-3", help="parasite words per side, e.g. 2-3 or 1-2")
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--apply", action="store_true")
    args = ap.parse_args()
    lo, _, hi = args.parasites.partition("-")
    n_range = (int(lo), int(hi or lo))
    rng = random.Random(args.seed)

    total = changed_files = 0
    for path in sorted((args.lang_dir / "exercises").glob("*.exercises.json")):
        data = json.loads(path.read_text(encoding="utf-8"))
        if data.get("sync") == "off":
            continue
        exercises = data.get("exercises") or []
        pairs = file_pairs(exercises)
        changed = []
        for i, ex in enumerate(exercises):
            new = rework(ex, pairs, n_range, rng)
            if new is not None:
                changed.append((ex, new))
                exercises[i] = new
        if not changed:
            continue
        changed_files += 1
        total += len(changed)
        print(f"\n{path.name}: {len(changed)}")
        for old, new in changed[:2]:
            print(f"  {' | '.join(old.get('tokens_l2') or [])}\n  -> {' | '.join(new['tokens_l2'])}")
        if args.apply:
            path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    verb = "Reworked" if args.apply else "Would rework"
    print(f"\n{verb} {total} order exercise(s) in {changed_files} file(s).")
    if not args.apply and total:
        print("Dry run -- pass --apply to write.")


if __name__ == "__main__":
    try:
        main()
    except ValueError as exc:
        sys.exit(f"Error: {exc}")
