#!/usr/bin/env python3
"""smd2data.py

Convert an SMD (Speakable Markdown) sheet into a structured JSON
"content" file, meant to be assembled into the site's data.json and
rendered client-side.

This replaces smd2html.py from Slovingo v1: same parsing rules, but
the output is a JSON block list instead of HTML markup. Rendering
(including inline speakable/bold resolution) now happens in the
browser, not at generation time.

Usage:
    python3 smd2data.py sheet.md --lang lang.json [-o sheet.content.json]

Filename convention (see conventions-and-principles):
    XX_Category_XX_Subgroup_XX_Title.md

    XX are order prefixes used only for filesystem sorting; they are
    never stored in the output. The stable sheet id is derived from
    category/subgroup/title alone, so it survives renumbering.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any


CATEGORIES = ["introduction", "series", "dialog", "vocabulary", "annex"]
"""The categories with dedicated behaviour elsewhere in the pipeline
(e.g. "introduction" is skipped for exercise generation, "series" gets
numbered kickers in the front-end). Any other category segment is
still accepted by parse_sheet_filename() -- see the note there -- this
list is not an exhaustive whitelist."""

SPEAKABLE_RE = re.compile(r"\{\{(.+?)\}\}")
IMAGE_RE = re.compile(r"^@\s+(\S+)\s*\|\s*(.+)$")
IMAGE_MARKDOWN_RE = re.compile(r"^!\[([^\]]*)\]\(([^\s)]+)(?:\s+[\"']([^\"']*)[\"'])?\s*\)$")
HEADING_RE = re.compile(r"^(#{1,6})\s+(.+)$")
UNORDERED_ITEM_RE = re.compile(r"^[-*]\s+(.+)$")
ORDERED_ITEM_RE = re.compile(r"^\d+\.\s+(.+)$")
HR_RE = re.compile(r"^([-*_])(?:\s*\1){2,}\s*$")


class SheetNameError(ValueError):
    """Raised when a filename does not follow the sheet naming convention."""


# ============================================================================
# Filename parsing
# ============================================================================

def humanize_subgroup(raw_segments: list[str]) -> str:
    """Derive a default display label for a subgroup from its raw
    filename segments (original casing, before any lowercasing).

    Splits camelCase words apart (e.g. "VelkaNoc" -> "Velka Noc"),
    joins multiple underscore-separated segments with spaces (e.g.
    ["Kronika", "Pionieri"] -> "Kronika Pionieri"), then lowercases
    everything except the very first letter.

    This is only ever a fallback: diacritics that don't survive an
    ASCII filename (e.g. "Velka Noc" instead of "Veľká noc") are
    expected to be overridden via lang.json's `subgroups` map, keyed
    by the lowercase slug.

    Args:
        raw_segments: The subgroup's filename segments, in their
            original casing (the `middle` parts from the filename
            split, before lowercasing).

    Returns:
        A human-readable label, or "" if raw_segments is empty (no
        subgroup).
    """
    words: list[str] = []
    for segment in raw_segments:
        spaced = re.sub(r"(?<=[a-z])(?=[A-Z])", " ", segment)
        words.extend(spaced.split())
    if not words:
        return ""
    label = " ".join(w.lower() for w in words)
    return label[0].upper() + label[1:]


def parse_sheet_filename(path: Path) -> dict[str, str | None]:
    """Parse a sheet filename into its stable identifying parts.

    Two forms are accepted:

    - XX_Category_XX_Subgroup_XX_Title.md (6+ segments) -- the normal
      case. Subgroup may itself contain underscores (e.g.
      "Kronika_Pionieri"), so parsing anchors on the first two
      segments (order, category) and the last two (order, title), and
      treats everything in between as the subgroup, however many
      segments that is.
    - XX_Category_XX_Title.md (exactly 4 segments) -- no subgroup.
      Used by categories where every sheet is already its own
      standalone topic (e.g. Vocabulary, Introduction): `subgroup` is
      None, and the sheet is expected to be listed directly under its
      category with no intermediate grouping level.

    The numeric prefixes only encode filesystem display order and are
    discarded here -- the id must stay stable when a sheet is
    renumbered.

    Args:
        path: Path to the .md sheet.

    Returns:
        A dict with keys: id, category, subgroup (None if absent),
        subgroup_label (a default display label, "" if no subgroup --
        see humanize_subgroup()), title_slug.

    The category segment is not restricted to CATEGORIES: any lowercase
    slug is accepted, so a one-off category (e.g. a closing "thank you"
    page) works without touching this list. CATEGORIES only tracks
    which categories get special-cased behaviour elsewhere (see its
    docstring) -- an unlisted category just gets the generic/default
    treatment everywhere that switches on it.

    Raises:
        SheetNameError: If the filename does not have 4 or 6+
            underscore-separated parts, or an order segment is not
            numeric.
    """
    stem = path.stem
    parts = stem.split("_")

    if len(parts) == 4:
        order1, category_raw, order2, title_raw = parts
        orders = (order1, order2)
        middle: list[str] = []
    elif len(parts) >= 6:
        order1, category_raw, order2, *middle, order3, title_raw = parts
        orders = (order1, order2, order3)
    else:
        raise SheetNameError(
            f"{path.name}: expected 4 parts (XX_Category_XX_Title) or "
            f"6+ parts (XX_Category_XX_Subgroup_XX_Title), got "
            f"{len(parts)}: {parts}"
        )

    for order in orders:
        if not order.isdigit():
            raise SheetNameError(
                f"{path.name}: expected numeric order prefixes, got '{order}'"
            )

    category = category_raw.lower()

    subgroup_slug = "_".join(middle).lower() if middle else None
    title_slug = title_raw.lower()

    sheet_id = (
        f"{category}-{subgroup_slug}-{title_slug}"
        if subgroup_slug
        else f"{category}-{title_slug}"
    )

    return {
        "id": sheet_id,
        "category": category,
        "subgroup": subgroup_slug,
        "subgroup_label": humanize_subgroup(middle),
        "title_slug": title_slug,
    }


# ============================================================================
# Inline text helpers
# ============================================================================

def resolve_speakable(text: str) -> str:
    """Rewrite {{word}} markers into the [[word]] form consumed by the
    front-end's text renderer.

    Bold/italic markdown (**...**, *...*) is deliberately left as-is:
    the front-end resolves both speakable spans and inline emphasis in
    a single pass, so the generator does not need to know about HTML
    at all.
    """
    return SPEAKABLE_RE.sub(lambda m: f"[[{m.group(1)}]]", text)


def split_speaker(text: str) -> tuple[str | None, str]:
    """Split a leading speaker marker (an emoji) off a dialogue line.

    A line such as "👦 Dobrý deň..." is prefixed by the speaker's
    emoji. Detected the same way as in v1's smd2exercises.py: the
    first whitespace-separated token has no alphabetic character.

    Args:
        text: The raw audio-card text, speaker marker included.

    Returns:
        A (speaker, rest) tuple. speaker is None when no marker is
        found, in which case rest equals the original text.
    """
    tokens = text.split(None, 1)
    if tokens and not any(ch.isalpha() for ch in tokens[0]):
        speaker = tokens[0]
        rest = tokens[1].strip() if len(tokens) > 1 else ""
        return speaker, rest
    return None, text


# ============================================================================
# Tables
# ============================================================================

def is_table_separator(line: str) -> bool:
    """Return True if `line` is a markdown table separator row (---|---)."""
    stripped = line.strip()
    if not stripped.startswith("|") and "|" not in stripped:
        return False
    return bool(re.match(r"^\|?[\s:|-]+\|?$", stripped)) and "-" in stripped


def split_table_row_raw(line: str) -> list[str]:
    """Split a markdown table row into raw (unresolved) cell strings."""
    return [cell.strip() for cell in line.strip().strip("|").split("|")]


def is_header_match(text: str, headers: list[str], header_roots: list[str]) -> bool:
    """Check whether a column header designates the described language.

    Mirrors v1's langconfig.is_header_match(): exact match against
    `headers` first, then substring match against `header_roots`.
    """
    normalized = text.strip().lower()
    if normalized in headers:
        return True
    return any(root in normalized for root in header_roots)


def parse_table(
    lines: list[str], target_headers: list[str], target_header_roots: list[str]
) -> dict[str, Any]:
    """Parse a markdown table block into a `table` content block.

    `speakable_column` is set to the index of the column whose header
    matches the target language config, if any -- this is what makes
    a table a "translate-table" in the old HTML output. Only the
    target column is required; there is no dependency on a native
    column being present or recognised (see Vocabulaire_01_Nombres.md,
    whose two headers are both Slovak).
    """
    raw_headers = split_table_row_raw(lines[0])
    columns = [resolve_speakable(h) for h in raw_headers]
    rows = [
        [resolve_speakable(cell) for cell in split_table_row_raw(line)]
        for line in lines[2:]
        if line.strip()
    ]

    block: dict[str, Any] = {"type": "table", "columns": columns, "rows": rows}

    for index, header in enumerate(raw_headers):
        if is_header_match(header, target_headers, target_header_roots):
            block["speakable_column"] = index
            break

    return block


# ============================================================================
# Body parsing (everything after title + leading illustration)
# ============================================================================

def parse_content(
    md_text: str, target_headers: list[str], target_header_roots: list[str]
) -> list[dict[str, Any]]:
    """Parse the body of an SMD sheet into a list of typed content blocks.

    Mirrors smd2html.py's convert_markdown(), block for block, but
    emits JSON dicts instead of HTML strings. Handles the SMD-specific
    constructs (audio-card, translate-table, speakable, illustration)
    on top of generic markdown (headings, lists, blockquotes, hr,
    paragraphs) -- both are needed: pedagogical fiches lean on
    audio-cards, but Introduction/Grammaire/Kronika fiches make heavy
    use of plain markdown too.

    Args:
        md_text: The sheet body, i.e. everything after the title line
            and the optional leading "@ ..." illustration line.
        target_headers: Exact-match header strings for the target
            language (from lang.json's generator.target_headers).
        target_header_roots: Substring-match header roots for the
            target language.

    Returns:
        A list of content block dicts, in document order.
    """
    lines = md_text.splitlines()
    n = len(lines)
    blocks: list[dict[str, Any]] = []
    index = 0

    pending_list: dict[str, Any] | None = None

    def flush_list() -> None:
        nonlocal pending_list
        if pending_list is not None:
            blocks.append(pending_list)
            pending_list = None

    while index < n:
        line = lines[index]
        stripped = line.strip()

        # Blank line: paragraph/list boundary.
        if not stripped:
            flush_list()
            index += 1
            continue

        # Table (translate-table or plain -- see parse_table()).
        if "|" in stripped and index + 1 < n and is_table_separator(lines[index + 1]):
            flush_list()
            table_lines = [lines[index], lines[index + 1]]
            index += 2
            while index < n and "|" in lines[index] and lines[index].strip():
                table_lines.append(lines[index])
                index += 1
            blocks.append(parse_table(table_lines, target_headers, target_header_roots))
            continue

        # Illustration mid-content. The leading illustration (right
        # after the title) is consumed separately by parse_sheet();
        # this only fires for an "@ ..." line appearing later in the
        # body, which is tolerated but not expected in current sheets.
        # Supports both SMD format "@ url | caption" and
        # classic Markdown format "![alt](url)" or "![alt](url "title")".
        img_match = IMAGE_RE.match(stripped)
        if img_match:
            flush_list()
            src, caption = img_match.groups()
            blocks.append({
                "type": "image",
                "src": src,
                "caption": resolve_speakable(caption),
            })
            index += 1
            continue
        
        img_markdown_match = IMAGE_MARKDOWN_RE.match(stripped)
        if img_markdown_match:
            flush_list()
            alt_text, url, title = img_markdown_match.groups()
            # Use title if present, otherwise use alt text
            caption = title if title else alt_text
            blocks.append({
                "type": "image",
                "src": url,
                "caption": resolve_speakable(caption),
            })
            index += 1
            continue

        # Audio-card: "!" line, followed by ">" translation/breakdown
        # lines (first one natural, rest literal), then "+" note lines.
        if stripped.startswith("!"):
            flush_list()
            raw_text = stripped[1:].strip()
            speaker, phrase = split_speaker(raw_text)

            j = index + 1
            translations: list[str] = []
            while j < n and lines[j].strip().startswith(">"):
                translations.append(lines[j].strip()[1:].strip())
                j += 1
            notes: list[str] = []
            while j < n and lines[j].strip().startswith("+"):
                notes.append(lines[j].strip()[1:].strip())
                j += 1

            natural = translations[0] if translations else None
            literal = translations[1:] if len(translations) > 1 else []

            blocks.append({
                "type": "audio-card",
                "phrase": resolve_speakable(phrase),
                "speaker": speaker,
                "natural": resolve_speakable(natural) if natural is not None else None,
                "literal": [resolve_speakable(t) for t in literal],
                "notes": [resolve_speakable(t) for t in notes],
            })
            index = j
            continue

        # Standalone blockquote (">" lines with no preceding "!").
        if stripped.startswith(">"):
            flush_list()
            quote_lines: list[str] = []
            while index < n and lines[index].strip().startswith(">"):
                quote_lines.append(resolve_speakable(lines[index].strip()[1:].strip()))
                index += 1
            blocks.append({"type": "blockquote", "lines": quote_lines})
            continue

        # Heading (any level -- a level-1 heading can appear mid-body,
        # e.g. Introduction/Kronika sheets use it for subsections).
        heading_match = HEADING_RE.match(stripped)
        if heading_match:
            flush_list()
            level = len(heading_match.group(1))
            text = resolve_speakable(heading_match.group(2).strip())
            blocks.append({"type": "heading", "level": level, "text": text})
            index += 1
            continue

        # Unordered list item. "+" is deliberately excluded here: it
        # is reserved for audio-card notes, never a bullet marker.
        ul_match = UNORDERED_ITEM_RE.match(stripped)
        if ul_match:
            if pending_list is None or pending_list["ordered"]:
                flush_list()
                pending_list = {"type": "list", "ordered": False, "items": []}
            pending_list["items"].append(resolve_speakable(ul_match.group(1)))
            index += 1
            continue

        # Ordered list item.
        ol_match = ORDERED_ITEM_RE.match(stripped)
        if ol_match:
            if pending_list is None or not pending_list["ordered"]:
                flush_list()
                pending_list = {"type": "list", "ordered": True, "items": []}
            pending_list["items"].append(resolve_speakable(ol_match.group(1)))
            index += 1
            continue

        # Horizontal rule (---, ***, ___).
        if HR_RE.match(stripped):
            flush_list()
            blocks.append({"type": "hr"})
            index += 1
            continue

        # Plain paragraph: anything else.
        flush_list()
        blocks.append({"type": "paragraph", "text": resolve_speakable(stripped)})
        index += 1

    flush_list()
    return blocks


# ============================================================================
# Whole-sheet parsing
# ============================================================================

def mark_dialogue_sections(blocks: list[dict[str, Any]]) -> None:
    """Mark sections that contain only audio-cards with speakers as dialogues.
    
    A section is dialogue if:
    - It's bounded by H2/H3 headings (or start/end of content)
    - It contains ONLY audio-card blocks
    - EVERY audio-card has a non-None speaker (emoji)
    
    Modifies the heading block that precedes a dialogue section by adding
    `is_dialogue_section: true`.
    
    Args:
        blocks: The parsed content blocks (modified in-place).
    """
    i = 0
    while i < len(blocks):
        block = blocks[i]
        
        # Look for H2/H3 headings
        if block.get("type") == "heading" and block.get("level") in (2, 3):
            # Find the start and end of the section after this heading
            section_start = i + 1
            section_end = section_start
            
            # Find the next H2/H3 heading or end of blocks
            while section_end < len(blocks):
                if (blocks[section_end].get("type") == "heading" and 
                    blocks[section_end].get("level") in (2, 3)):
                    break
                section_end += 1
            
            # Check if this section is a dialogue
            is_dialogue = True
            if section_start < section_end:
                for j in range(section_start, section_end):
                    block_j = blocks[j]
                    # Must be an audio-card
                    if block_j.get("type") != "audio-card":
                        is_dialogue = False
                        break
                    # And must have a speaker (emoji)
                    if not block_j.get("speaker"):
                        is_dialogue = False
                        break
            else:
                # Empty section
                is_dialogue = False
            
            # Mark the heading if it's a dialogue section
            if is_dialogue:
                block["is_dialogue_section"] = True
        
        i += 1


def parse_sheet(md_text: str, lang_cfg: dict[str, Any]) -> dict[str, Any]:
    """Parse a full SMD sheet: title, optional leading illustration, body.

    Args:
        md_text: The full content of the .md file.
        lang_cfg: The loaded lang.json (v2) for this language pair.

    Returns:
        A dict with keys: title, image (or None), content (block list).

    Raises:
        ValueError: If the sheet does not start with a level-1 heading.
    """
    lines = md_text.splitlines()
    idx = 0
    n = len(lines)

    while idx < n and not lines[idx].strip():
        idx += 1
    if idx >= n or not lines[idx].strip().startswith("# "):
        raise ValueError("Sheet must start with a level-1 heading (# Title)")
    title = resolve_speakable(lines[idx].strip()[2:].strip())
    idx += 1

    image: dict[str, str] | None = None
    while idx < n and not lines[idx].strip():
        idx += 1
    if idx < n:
        stripped_line = lines[idx].strip()
        img_match = IMAGE_RE.match(stripped_line)
        if img_match:
            src, caption = img_match.groups()
            image = {"src": src, "caption": resolve_speakable(caption)}
            idx += 1
        else:
            # Try classic Markdown format: ![alt](url) or ![alt](url "title")
            img_markdown_match = IMAGE_MARKDOWN_RE.match(stripped_line)
            if img_markdown_match:
                alt_text, url, title = img_markdown_match.groups()
                caption = title if title else alt_text
                image = {"src": url, "caption": resolve_speakable(caption)}
                idx += 1

    generator_cfg = lang_cfg.get("generator", {})
    body = "\n".join(lines[idx:])
    content = parse_content(
        body,
        target_headers=generator_cfg.get("target_headers", []),
        target_header_roots=generator_cfg.get("target_header_roots", []),
    )
    
    # Mark dialogue sections (H2/H3 headings followed by only audio-cards with speakers)
    mark_dialogue_sections(content)

    return {"title": title, "image": image, "content": content}


def build_sheet_json(path: Path, lang_cfg: dict[str, Any]) -> dict[str, Any]:
    """Parse one sheet file end-to-end into its final JSON shape."""
    meta = parse_sheet_filename(path)
    md_text = path.read_text(encoding="utf-8")
    parsed = parse_sheet(md_text, lang_cfg)

    return {
        "id": meta["id"],
        "category": meta["category"],
        "subgroup": meta["subgroup"],
        "subgroup_label": meta["subgroup_label"],
        "title": parsed["title"],
        "image": parsed["image"],
        "content": parsed["content"],
    }


# ============================================================================
# CLI
# ============================================================================

def main() -> None:
    parser = argparse.ArgumentParser(description="Convert an SMD sheet into a content JSON file.")
    parser.add_argument("input", type=Path, help="Path to the .md sheet")
    parser.add_argument("--lang", type=Path, required=True, help="Path to lang.json")
    parser.add_argument(
        "-o", "--output", type=Path,
        help="Output JSON path (default: <id>.content.json next to the input file)",
    )
    args = parser.parse_args()

    with args.lang.open(encoding="utf-8") as f:
        lang_cfg = json.load(f)

    try:
        sheet = build_sheet_json(args.input, lang_cfg)
    except (SheetNameError, ValueError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        sys.exit(1)

    output = args.output or args.input.with_name(f"{sheet['id']}.content.json")
    output.write_text(json.dumps(sheet, ensure_ascii=False, indent=2), encoding="utf-8")
    print(f"Wrote {output}")


if __name__ == "__main__":
    main()
