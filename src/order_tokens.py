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

Exercises already cut word by word keep their chips; a side with no
parasite word at all gets some (short sentences of the kids courses
used to end up with none). Parasites come from the same file first, then
from the other files of the same series, then from the whole course.
Files with "sync": "off" are left alone.

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
        # Another form of a target word ("polievku" for "Polievka") would
        # be a mean trap, not a distractor.
        if len(b) >= 4 and any(len(t) >= 4 and t[:4] == b[:4] for t in taken):
            continue
        seen.add(b)
        pool.append(w.strip(PUNCT))
    rng.shuffle(pool)
    return pool[:n]


def ranked_sources(ex: dict[str, Any], tiers: list[list[tuple[str, str]]]) -> list[tuple[str, str]]:
    """Other sentences to take parasites from: same file first, then same
    series, then the whole course; within a tier, the ones sharing the
    fewest words with the target first."""
    target_words = {bare(w) for w in ex["l2"].split()}
    seen = {(ex["l1"], ex["l2"])}
    out: list[tuple[str, str]] = []
    for tier in tiers:
        others = [p for p in tier if p not in seen and len(p[1].split()) >= 2 and len(p[0].split()) >= 2]
        others.sort(key=lambda p: (len(target_words & {bare(w) for w in p[1].split()}), p[1]))
        for p in others:
            seen.add(p)
            out.append(p)
    return out


def pick_source(ex: dict[str, Any], pairs: list[tuple[str, str]]) -> tuple[str, str] | None:
    """The other sentence of the file sharing the fewest words with the target."""
    ranked = ranked_sources(ex, [[p for p in pairs if len(p[0].split()) >= 3 and len(p[1].split()) >= 3]])
    return ranked[0] if ranked else None


def gather_parasites(target: str, sources: list[str], n: int, rng: random.Random) -> list[str]:
    """n parasite words for `target`, from the first sources that have some."""
    out: list[str] = []
    for src in sources:
        if len(out) >= n:
            break
        taken = target + " " + " ".join(out)
        out += parasite_words(taken, src, n - len(out), rng)
    return out


def file_pairs(exercises: list[dict[str, Any]]) -> list[tuple[str, str]]:
    """Sentence pairs of the file, from every exercise that has plain l1/l2."""
    seen, out = set(), []
    for ex in exercises:
        l1, l2 = ex.get("l1"), ex.get("l2")
        if isinstance(l1, str) and isinstance(l2, str) and (l1, l2) not in seen:
            seen.add((l1, l2))
            out.append((l1, l2))
    return out


def rework(ex: dict[str, Any], tiers: list[list[tuple[str, str]]], n_range: tuple[int, int], rng: random.Random) -> dict[str, Any] | None:
    if ex.get("type") != "order":
        return None
    s1 = split_target(ex.get("tokens_l1") or ex["l1"].split(), ex["l1"])
    s2 = split_target(ex.get("tokens_l2") or ex["l2"].split(), ex["l2"])
    if s1 is None or s2 is None:
        return None
    blocky = is_blocky(s1[0]) or is_blocky(s2[0])
    p1, p2 = s1[1], s2[1]
    if not blocky and p1 and p2:
        return None  # word by word, with parasites: left alone
    t1, t2 = (rechip(s1[0]), rechip(s2[0])) if blocky else (s1[0], s2[0])
    if not p1 or not p2:
        sources = ranked_sources(ex, tiers)
        n = rng.randint(*n_range)
        if not p2:
            p2 = gather_parasites(ex["l2"], [src[1] for src in sources], n, rng)
        if not p1:
            p1 = gather_parasites(ex["l1"], [src[0] for src in sources], len(p2) or n, rng)
    if not blocky and (p1, p2) == (s1[1], s2[1]):
        return None
    new = json.loads(json.dumps(ex))
    new["tokens_l1"] = t1 + p1
    new["tokens_l2"] = t2 + p2
    for chips, text in ((t1, ex["l1"]), (t2, ex["l2"])):
        if " ".join(chips).split() != text.split():
            raise ValueError(f"{ex.get('id')}: chips {chips} do not spell {text!r}")
    return new


def rewrite(text: str, data: dict[str, Any], changed: list[tuple[dict[str, Any], dict[str, Any]]]) -> str:
    """The file with only the changed exercises replaced, keeping its own
    layout (one exercise per line, or indented): a small diff."""
    out = text
    for old, new in changed:
        line = json.dumps(old, ensure_ascii=False)
        if out.count(line) != 1:
            return json.dumps(data, ensure_ascii=False, indent=2) + "\n"
        out = out.replace(line, json.dumps(new, ensure_ascii=False))
    return out


def series_key(path: Path) -> str:
    """'10_Series_01_Bratislava_03_x.exercises.json' -> '10_Series_01_Bratislava'."""
    return re.sub(r"_\d+_[^_]+\.exercises\.json$", "", path.name)


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

    paths = sorted((args.lang_dir / "exercises").glob("*.exercises.json"))
    all_pairs: dict[Path, list[tuple[str, str]]] = {}
    for path in paths:
        data = json.loads(path.read_text(encoding="utf-8"))
        all_pairs[path] = file_pairs(data.get("exercises") or [])
    course = [p for pairs in all_pairs.values() for p in pairs]

    total = changed_files = 0
    for path in paths:
        data = json.loads(path.read_text(encoding="utf-8"))
        if data.get("sync") == "off":
            continue
        exercises = data.get("exercises") or []
        series = [p for q, pairs in all_pairs.items() if q != path and series_key(q) == series_key(path) for p in pairs]
        tiers = [all_pairs[path], series, course]
        changed = []
        for i, ex in enumerate(exercises):
            new = rework(ex, tiers, n_range, rng)
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
            path.write_text(rewrite(path.read_text(encoding="utf-8"), data, changed), encoding="utf-8")
    verb = "Reworked" if args.apply else "Would rework"
    print(f"\n{verb} {total} order exercise(s) in {changed_files} file(s).")
    if not args.apply and total:
        print("Dry run -- pass --apply to write.")


if __name__ == "__main__":
    try:
        main()
    except ValueError as exc:
        sys.exit(f"Error: {exc}")
