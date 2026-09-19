#!/usr/bin/env python3
"""smd2exercises.py

Generate practice exercises (qcm, fill-blank, listen, order) from a corpus of
SMD sheets, in the merged l1/l2 format (see conventions-and-principles
and areas/v2-rewrite): l1 is always the native language, l2 always the
target language, audio is always l2.

Replaces smd2exercises.py from Slovingo v1. What's new compared to v1:

- Merged format: one exercise object covers both practice directions
  (l1->l2 and l2->l1), instead of two separate entries.
- Distractors are shape-matched: a single-word answer never gets a
  full-sentence distractor and vice versa (see word_shape()).
- For `series` sheets, distractors are restricted to vocabulary
  already introduced by *earlier* series sheets ("deja vu"), falling
  back to the whole corpus only when that pool is too thin to build a
  full set of choices. Other categories draw from the whole corpus,
  unrestricted by order, as v1 always did.
- Sheets are read directly from their .md source (via smd2data's
  parser) rather than from pre-built content.json files, so this
  script has no ordering dependency on the content-generation step.

Generation is persistent: an existing <id>.exercises.json is never
overwritten unless --force is passed, so hand-edited exercises survive
a re-run over the corpus.

Usage:
    python3 smd2exercises.py md_dir/ --lang lang.json -o json_dir/
"""

from __future__ import annotations

import argparse
import json
import random
import re
import sys
from collections import Counter
from pathlib import Path
from typing import Any, Iterable

from smd2data import CATEGORIES, SheetNameError, parse_sheet, parse_sheet_filename


# ============================================================================
# Tuning constants
# ============================================================================

MIN_TOKENS_FOR_BLANK = 3     # v1's guard: too short a sentence makes a
                              # blank either trivial or nonsensical.
WORD_SHAPE_MAX_TOKENS = 2     # <=2 tokens: "word"-like answer/distractor.
                              # >2 tokens: "phrase"-like.
QCM_CHOICES = 4               # total options shown, answer included
                              # (so 3 distractors are generated).
FILL_BLANK_CHOICES = 3
LISTEN_CHOICES = 4
FREQUENT_WORDS_CONSIDERED = 40  # how many top-frequency words feed the
                                  # fill-blank distractor pool.


# ============================================================================
# Small text helpers
# ============================================================================

SPEAKABLE_MARKER_RE = re.compile(r"\[\[(.+?)\]\]")
BOLD_RE = re.compile(r"\*\*(.+?)\*\*")
SLUG_RE = re.compile(r"[^a-z0-9]+")
PUNCT_CHARS = ".,!?;:\"'()„“”«»…"


def strip_markup(text: str) -> str:
    """Remove the [[speakable]] and **bold** markers left by smd2data,
    so exercise text is plain and TTS-safe."""
    text = SPEAKABLE_MARKER_RE.sub(r"\1", text)
    text = BOLD_RE.sub(r"\1", text)
    return text


def strip_punct(word: str) -> str:
    return word.strip(PUNCT_CHARS)


def tokenize(text: str) -> list[str]:
    return text.split()


def word_shape(text: str) -> str:
    """Classify an answer/distractor as 'word' or 'phrase' by length,
    so a single vocabulary word never gets a full-sentence distractor
    (or the reverse)."""
    return "word" if len(tokenize(text)) <= WORD_SHAPE_MAX_TOKENS else "phrase"


def slugify(text: str) -> str:
    text = strip_markup(text).lower()
    text = SLUG_RE.sub("-", text).strip("-")
    return text or "x"


def dedupe_keep_order(items: Iterable[str]) -> list[str]:
    seen: set[str] = set()
    out = []
    for item in items:
        key = item.lower()
        if key not in seen:
            seen.add(key)
            out.append(item)
    return out


def dedupe_pairs(pairs: Iterable[tuple[str, str]]) -> list[tuple[str, str]]:
    seen: set[tuple[str, str]] = set()
    out = []
    for l1, l2 in pairs:
        key = (l1.lower(), l2.lower())
        if key not in seen:
            seen.add(key)
            out.append((l1, l2))
    return out


# ============================================================================
# Extracting exercise raw material from parsed sheet content
# ============================================================================

def extract_vocab_pairs(content: list[dict[str, Any]]) -> list[tuple[str, str]]:
    """Return (l1, l2) pairs from every 2-column translate-table in
    `content`. A table without a detected speakable_column, or with
    more than 2 columns, is not vocabulary in the l1/l2 sense and is
    skipped (e.g. the "Element/Info" table in Introduction sheets).
    """
    pairs = []
    for block in content:
        if block["type"] != "table" or block.get("speakable_column") is None:
            continue
        if len(block["columns"]) != 2:
            continue
        l2_col = block["speakable_column"]
        l1_col = 1 - l2_col
        for row in block["rows"]:
            l1 = strip_markup(row[l1_col]).strip()
            l2 = strip_markup(row[l2_col]).strip()
            if l1 and l2:
                pairs.append((l1, l2))
    return pairs


def extract_sentences(content: list[dict[str, Any]]) -> list[dict[str, Any]]:
    """Return {l1, l2, speaker} dicts from every audio-card in
    `content` that has a natural translation (cards without one can't
    build an l1 side and are skipped)."""
    sentences = []
    for block in content:
        if block["type"] != "audio-card" or not block.get("natural"):
            continue
        sentences.append({
            "l1": strip_markup(block["natural"]).strip(),
            "l2": strip_markup(block["phrase"]).strip(),
            "speaker": block.get("speaker"),
        })
    return sentences


# ============================================================================
# Corpus loading
# ============================================================================

def load_corpus(md_dir: Path, lang_cfg: dict[str, Any]) -> list[dict[str, Any]]:
    """Parse every sheet in `md_dir` (filesystem order) into the raw
    material exercises are built from. Introduction sheets are
    excluded outright -- no exercises are generated for that category.
    """
    records = []
    for path in sorted(md_dir.glob("*.md")):
        meta = parse_sheet_filename(path)
        if meta["category"] == "introduction":
            continue
        md_text = path.read_text(encoding="utf-8")
        parsed = parse_sheet(md_text, lang_cfg)
        records.append({
            "id": meta["id"],
            "category": meta["category"],
            "vocab_pairs": extract_vocab_pairs(parsed["content"]),
            "sentences": extract_sentences(parsed["content"]),
        })
    return records


# ============================================================================
# Distractor selection
# ============================================================================

def pick_shape_matched_distractors(
    answer: str, primary_pool: list[str], fallback_pool: list[str], n: int
) -> list[str] | None:
    """Pick n distractors of the same shape (word/phrase) as `answer`.

    Prefers `primary_pool` (e.g. already-seen vocabulary for a series
    sheet); tops up from `fallback_pool` (the whole corpus) when the
    primary pool is too thin, rather than failing outright -- mirrors
    v1's "corpus still small" fallback. Returns None if there still
    aren't enough candidates even after falling back.
    """
    shape = word_shape(answer)
    answer_l = answer.lower()

    candidates = dedupe_keep_order(
        w for w in primary_pool if w.lower() != answer_l and word_shape(w) == shape
    )
    if len(candidates) < n:
        extra = dedupe_keep_order(
            w for w in fallback_pool
            if w.lower() != answer_l and word_shape(w) == shape and w.lower() not in {c.lower() for c in candidates}
        )
        candidates = candidates + extra

    if len(candidates) < n:
        return None
    return random.sample(candidates, n)


def build_word_frequency(texts: Iterable[str]) -> Counter:
    """Word -> occurrence count, across `texts`. A token that's pure
    punctuation (e.g. a standalone "-" or "»") strips down to an empty
    string and is dropped rather than counted -- left in, it could win
    a spot among the most frequent "words" and get offered as a
    fill-blank distractor, which is how a handful of existing
    <id>.exercises.json files ended up with a "" in choices_l1/l2 (see
    conventions-and-principles: bugs found during testing are fixed
    inline)."""
    freq: Counter = Counter()
    for text in texts:
        for token in tokenize(text):
            word = strip_punct(token).lower()
            if word:
                freq[word] += 1
    return freq


def choose_blank_index(tokens: list[str], selection_freq: Counter) -> int:
    """Pick which word to blank: the most frequent one in the corpus
    among candidates (skipping the first token, often capitalised and
    sentence-specific). Falls back to the shortest word in the middle
    of the sentence when nothing stands out (a still-small corpus).

    Uses corpus-wide frequency regardless of the "already seen"
    distractor restriction: picking a grammatically common word to
    blank is a different concern from restricting which *wrong*
    answers are fair game.
    """
    candidates = list(range(1, len(tokens))) or [0]
    idx = max(candidates, key=lambda i: selection_freq.get(strip_punct(tokens[i]).lower(), 0))
    if selection_freq.get(strip_punct(tokens[idx]).lower(), 0) <= 1:
        middle = list(range(1, len(tokens) - 1)) or candidates
        idx = min(middle, key=lambda i: len(strip_punct(tokens[i])))
    return idx


def pick_frequency_distractors(
    answer: str, primary_freq: Counter, global_freq: Counter, n: int
) -> list[str] | None:
    """Pick n plausible fill-blank distractors: frequent words (likely
    grammatical, so a fair trap) from `primary_freq`, topped up from
    `global_freq` when too few, then from any word in the corpus as a
    last resort. Returns None if still short after all fallbacks.
    """
    answer_l = answer.lower()

    candidates = dedupe_keep_order(
        w for w, _ in primary_freq.most_common(FREQUENT_WORDS_CONSIDERED) if w != answer_l
    )
    if len(candidates) < n:
        candidates = dedupe_keep_order(
            candidates + [w for w, _ in global_freq.most_common(FREQUENT_WORDS_CONSIDERED) if w != answer_l]
        )
    if len(candidates) < n:
        candidates = dedupe_keep_order(candidates + [w for w in global_freq if w != answer_l])

    if len(candidates) < n:
        return None
    return random.sample(candidates, n)


# ============================================================================
# Exercise generation
# ============================================================================

def generate_qcm(
    sheet: dict[str, Any], primary_pairs: list[tuple[str, str]],
    global_pairs: list[tuple[str, str]],
) -> list[dict[str, Any]]:
    """Build one merged qcm exercise per unique vocabulary pair of the
    sheet. choices_l1/choices_l2 are distractors only -- the front-end
    adds the correct l1/l2 answer and shuffles at display time, so the
    same exercise doesn't always show options in the same order.
    """
    local_pairs = dedupe_pairs(sheet["vocab_pairs"])
    if not local_pairs:
        return []

    primary = dedupe_pairs(primary_pairs)
    glob = dedupe_pairs(global_pairs)

    exercises = []
    for l1, l2 in local_pairs:
        choices_l1 = pick_shape_matched_distractors(
            l1,
            primary_pool=[p_l1 for p_l1, p_l2 in primary if p_l2.lower() != l2.lower()],
            fallback_pool=[p_l1 for p_l1, p_l2 in glob if p_l2.lower() != l2.lower()],
            n=QCM_CHOICES - 1,
        )
        choices_l2 = pick_shape_matched_distractors(
            l2,
            primary_pool=[p_l2 for p_l1, p_l2 in primary if p_l1.lower() != l1.lower()],
            fallback_pool=[p_l2 for p_l1, p_l2 in glob if p_l1.lower() != l1.lower()],
            n=QCM_CHOICES - 1,
        )
        if choices_l1 is None or choices_l2 is None:
            continue  # not enough corpus yet to build fair distractors

        exercises.append({
            "id": f"qcm-{slugify(l2)}",
            "type": "qcm",
            "l1": l1,
            "l2": l2,
            "choices_l1": choices_l1,
            "choices_l2": choices_l2,
        })
    return exercises


def generate_fill_blank(
    sheet: dict[str, Any],
    primary_sentences: list[dict[str, Any]],
    global_sentences: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    """Build one merged fill-blank exercise per sentence of the sheet,
    with an independent blank computed on each side (l1 and l2)."""
    l1_selection_freq = build_word_frequency(s["l1"] for s in global_sentences)
    l2_selection_freq = build_word_frequency(s["l2"] for s in global_sentences)
    l1_primary_freq = build_word_frequency(s["l1"] for s in primary_sentences)
    l2_primary_freq = build_word_frequency(s["l2"] for s in primary_sentences)
    l1_global_freq = l1_selection_freq
    l2_global_freq = l2_selection_freq

    exercises = []
    for s in sheet["sentences"]:
        l1_blank = _build_blank(s["l1"], l1_selection_freq, l1_primary_freq, l1_global_freq)
        l2_blank = _build_blank(s["l2"], l2_selection_freq, l2_primary_freq, l2_global_freq)
        if l1_blank is None or l2_blank is None:
            continue

        exercises.append({
            "id": f"blank-{slugify(s['l2'])}",
            "type": "fill-blank",
            "l1": s["l1"],
            "l2": s["l2"],
            "missing_l1": l1_blank["missing"],
            "missing_l2": l2_blank["missing"],
            "blank_index_l1": l1_blank["blank_index"],
            "blank_index_l2": l2_blank["blank_index"],
            "choices_l1": l1_blank["choices"],
            "choices_l2": l2_blank["choices"],
        })
    return exercises


def _build_blank(
    text: str, selection_freq: Counter, primary_freq: Counter, global_freq: Counter
) -> dict[str, Any] | None:
    tokens = tokenize(text)
    if len(tokens) < MIN_TOKENS_FOR_BLANK:
        return None
    idx = choose_blank_index(tokens, selection_freq)
    missing = strip_punct(tokens[idx])
    if not missing:
        return None
    choices = pick_frequency_distractors(missing, primary_freq, global_freq, FILL_BLANK_CHOICES)
    if choices is None:
        return None
    return {"blank_index": idx, "missing": missing, "choices": choices}


def generate_listen(
    sheet: dict[str, Any],
    primary_sentences: list[dict[str, Any]],
    global_sentences: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    """Build one listen exercise per sentence: recognise the spoken l2
    sentence among shape-matched l2 distractors."""
    exercises = []
    for s in sheet["sentences"]:
        answer_l2 = s["l2"]
        choices_l2 = pick_shape_matched_distractors(
            answer_l2,
            primary_pool=[p["l2"] for p in primary_sentences if p["l2"].lower() != answer_l2.lower()],
            fallback_pool=[p["l2"] for p in global_sentences if p["l2"].lower() != answer_l2.lower()],
            n=LISTEN_CHOICES - 1,
        )
        if choices_l2 is None:
            continue

        exercises.append({
            "id": f"listen-{slugify(answer_l2)}",
            "type": "listen",
            "l1": s["l1"],
            "l2": answer_l2,
            "choices_l2": choices_l2,
        })
    return exercises


def generate_order(sheet: dict[str, Any]) -> list[dict[str, Any]]:
    """Build one "put the words in order" exercise per sentence. Each exercise
    has the target sentence as l1/l2, but tokens_l1/tokens_l2 are polluted with
    tokens from another random sentence in the sheet, making the reordering task
    harder (more noise to filter).

    Reconstruct l1/l2 from their own shuffled tokens (see exercises.js,
    resolveOrderView()) -- no distractor pool needed, unlike the other
    types, so no "primary"/"global" pools are threaded through here.

    tokens_l1/tokens_l2 are a naive space split, same as the front-end's
    own fallback when they're absent -- written out explicitly so the
    editor has something to regroup (e.g. "ne ... pas" into one chip)
    without having to invent the split itself.
    """
    exercises = []
    sentences = sheet["sentences"]
    valid_sentences = []  # sentences long enough to use as targets or parasites

    # Collect sentences that pass the MIN_TOKENS check
    for s in sentences:
        tokens_l1 = tokenize(s["l1"])
        tokens_l2 = tokenize(s["l2"])
        if len(tokens_l1) >= MIN_TOKENS_FOR_BLANK and len(tokens_l2) >= MIN_TOKENS_FOR_BLANK:
            valid_sentences.append((s, tokens_l1, tokens_l2))

    for i, (target, target_tokens_l1, target_tokens_l2) in enumerate(valid_sentences):
        # Try to find another sentence to pollute tokens with
        candidates = [j for j in range(len(valid_sentences)) if j != i]
        if candidates:
            j = random.choice(candidates)
            parasite, parasite_tokens_l1, parasite_tokens_l2 = valid_sentences[j]
            # Mix tokens: target + parasite
            mixed_tokens_l1 = target_tokens_l1 + parasite_tokens_l1
            mixed_tokens_l2 = target_tokens_l2 + parasite_tokens_l2
        else:
            # No other sentence available, use target alone
            mixed_tokens_l1 = target_tokens_l1
            mixed_tokens_l2 = target_tokens_l2

        exercises.append({
            "id": f"order-{slugify(target['l2'])}",
            "type": "order",
            "l1": target["l1"],
            "l2": target["l2"],
            "tokens_l1": mixed_tokens_l1,
            "tokens_l2": mixed_tokens_l2,
        })

    return exercises


# ============================================================================
# Orchestration
# ============================================================================

def build_exercises_for_corpus(records: list[dict[str, Any]]) -> dict[str, dict[str, Any]]:
    """Generate exercises sheet by sheet, in corpus (filesystem) order.

    For `series` sheets, the "already seen" pool accumulates only
    prior series sheets, in order -- a series sheet never gets
    distractors drawn from its own new vocabulary, nor from series
    sheets that come later. Other categories draw from the whole
    corpus (built upfront) regardless of position.

    Returns a dict of sheet id -> {source pools not included, just
    the "exercises" list} keyed for writing one file per sheet.
    """
    global_pairs = [pair for r in records for pair in r["vocab_pairs"]]
    global_sentences = [s for r in records for s in r["sentences"]]

    series_pairs_so_far: list[tuple[str, str]] = []
    series_sentences_so_far: list[dict[str, Any]] = []

    output: dict[str, dict[str, Any]] = {}

    for record in records:
        is_series = record["category"] == "series"
        primary_pairs = series_pairs_so_far if is_series else global_pairs
        primary_sentences = series_sentences_so_far if is_series else global_sentences

        exercises = []
        exercises += generate_qcm(record, primary_pairs, global_pairs)
        exercises += generate_fill_blank(record, primary_sentences, global_sentences)
        exercises += generate_listen(record, primary_sentences, global_sentences)
        exercises += generate_order(record)

        output[record["id"]] = {"exercises": exercises}

        if is_series:
            series_pairs_so_far = series_pairs_so_far + record["vocab_pairs"]
            series_sentences_so_far = series_sentences_so_far + record["sentences"]

    return output


# ============================================================================
# CLI
# ============================================================================

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate practice exercises for every sheet in a corpus."
    )
    parser.add_argument("md_dir", type=Path, help="Directory of .md sheets")
    parser.add_argument("--lang", type=Path, required=True, help="Path to lang.json")
    parser.add_argument(
        "-o", "--output-dir", type=Path, required=True,
        help="Directory to write <id>.exercises.json files into",
    )
    parser.add_argument(
        "--force", action="store_true",
        help="Overwrite existing <id>.exercises.json files (default: skip, to preserve hand edits)",
    )
    parser.add_argument("--seed", type=int, help="Random seed, for reproducible test runs")
    args = parser.parse_args()

    if args.seed is not None:
        random.seed(args.seed)

    with args.lang.open(encoding="utf-8") as f:
        lang_cfg = json.load(f)

    try:
        records = load_corpus(args.md_dir, lang_cfg)
    except (SheetNameError, ValueError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        sys.exit(1)

    all_exercises = build_exercises_for_corpus(records)

    args.output_dir.mkdir(parents=True, exist_ok=True)
    written, skipped = 0, 0
    for sheet_id, payload in all_exercises.items():
        out_path = args.output_dir / f"{sheet_id}.exercises.json"
        if out_path.exists() and not args.force:
            skipped += 1
            continue
        out_path.write_text(json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8")
        written += 1

    print(f"Wrote {written} file(s), skipped {skipped} already-existing file(s) "
          f"(use --force to regenerate).")


if __name__ == "__main__":
    main()
