#!/usr/bin/env python3
"""
export_story.py -- the whole story of a course on one page (histoire.html).

For readers who already speak the target language: only the narration and
the dialogue lines of the series sheets, chapter after chapter, without
vocabulary tables, grammar notes or exercises. Each line keeps its
translation, hidden by default (one button shows them all, a tap on a line
shows that one).

Opt-in, per course, in lang.json:

    "story": {
        "title": "L'histoire complète",          // page title
        "subtitle": "Celý príbeh",                // optional
        "intro": "…",                             // optional, a short paragraph
        "user_name": "Eric",                      // replaces [USER_NAME]
        "exclude_subgroups": ["kronika_*"],       // fnmatch patterns
        "headings": ["Le dialogue", "La suite de l'épisode"],
        "chapter_label": "Kapitola",
        "show_translations": "Afficher les traductions",
        "hide_translations": "Masquer les traductions",
        "back": "Retour au cours"
    }

Without a "story" key nothing is generated. Only the audio cards under one
of the `headings` (the dialogue sections) are kept; the speaker names come
from the sheet's character list ("Les personnages"), and an illustration
(img/illus_*) of a sheet is shown at its place in the story.

Usage (normally called by build.py):
    python3 src/export_story.py --lang-dir ../slovingo-sk-fr-friends
"""

from __future__ import annotations

import argparse
import fnmatch
import html
import json
import re
from pathlib import Path

DEFAULT_HEADINGS = ["Le dialogue", "La suite de l'épisode", "Les dialogues"]
NARRATOR = "💬"
CHARACTER_HEADINGS = ["Les personnages", "Personnages"]


def _clean(text: str, user_name: str) -> str:
    text = text.replace("[USER_NAME]", user_name)
    text = re.sub(r"\[\[fr:(.*?)\]\]", r"\1", text)
    text = re.sub(r"\[\[(.*?)\]\]", r"\1", text)
    return text.strip()


def _sheet_characters(sheet: dict, user_name: str) -> dict[str, str]:
    """Speaker emoji -> display name, from the sheet's character list."""
    names: dict[str, str] = {}
    current = None
    for block in sheet.get("content", []):
        if block.get("type") == "heading":
            current = block.get("text", "")
        elif block.get("type") == "list" and current in CHARACTER_HEADINGS:
            for item in block.get("items", []):
                parts = item.split(" ", 1)
                if len(parts) != 2:
                    continue
                emoji, rest = parts
                name = re.split(r"[,:(]", rest, 1)[0].strip()
                name = re.split(r" qui ", name, 1)[0].strip()
                names[emoji] = name
    return names


def _speaker_name(emoji: str | None, sheet_names: dict, global_names: dict, user_name: str,
                  chapter_names: dict | None = None) -> str:
    """Name for a speaker: the sheet's list, then the chapter's, then a proper
    name from the course-wide list; "" when unknown (the emoji is shown)."""
    if not emoji:
        return ""
    if emoji == "🦊":
        return user_name
    name = sheet_names.get(emoji) or ""
    if not name:
        # From another sheet, only a proper name is safe to reuse: a generic
        # role ("Un pompier…", "La directrice…") under the same emoji is
        # often another person.
        for cand in ((chapter_names or {}).get(emoji, ""),
                     (global_names.get(emoji) or "").split(":")[0].split(",")[0].strip()):
            if cand and not re.match(r"^(Un|Une|Le|La|Les|L')\b", cand):
                name = cand
                break
    if name in ("Toi", "toi"):
        return user_name
    return name


def build_story(data: dict, lang_cfg: dict) -> str | None:
    cfg = lang_cfg.get("story")
    if not cfg:
        return None
    site = lang_cfg.get("site", {})
    user_name = cfg.get("user_name") or site.get("user_name_default") or ""
    headings = cfg.get("headings") or DEFAULT_HEADINGS
    excluded = cfg.get("exclude_subgroups", [])
    labels = lang_cfg.get("subgroups", {})
    global_names = data.get("characters", {})
    chapter_label = cfg.get("chapter_label", "Kapitola")
    target = lang_cfg.get("target_lang", {})
    target_code = target.get("code", "sk") if isinstance(target, dict) else "sk"

    chapters = []
    for group in data.get("groups", []):
        sub = group.get("subgroup")
        if group.get("category") != "series" or not sub:
            continue
        if any(fnmatch.fnmatch(sub, pat) for pat in excluded):
            continue
        scenes = []
        chapter_names: dict[str, str] = {}
        for sheet in group.get("sheets", []):
            chapter_names.update(_sheet_characters(sheet, user_name))
        for sheet in group.get("sheets", []):
            names = _sheet_characters(sheet, user_name)
            lines = []
            current = None
            for block in sheet.get("content", []):
                t = block.get("type")
                if t == "heading":
                    current = block.get("text", "")
                elif t == "audio-card" and current in headings:
                    phrase = _clean(block.get("phrase", ""), user_name)
                    if not phrase:
                        continue
                    lines.append({
                        "speaker": block.get("speaker"),
                        "name": _speaker_name(block.get("speaker"), names, global_names, user_name, chapter_names),
                        "text": phrase,
                        "fr": _clean(block.get("natural", ""), user_name),
                    })
            if not lines:
                continue
            title = sheet.get("title", "")
            title = title.split("—", 1)[1].strip() if "—" in title else title
            image = sheet.get("image") or {}
            illus = image.get("src") if "illus_" in (image.get("src") or "") else None
            scenes.append({"id": sheet["id"], "title": title, "lines": lines, "image": illus})
        if scenes:
            chapters.append({"id": sub, "label": labels.get(sub, group.get("subgroup_label") or sub), "scenes": scenes})
    if not chapters:
        return None
    return render_html(chapters, cfg, site, chapter_label, target_code)


CSS = """
:root { color-scheme: light dark;
  --bg: light-dark(#fbf8f2, #1b1a17); --text: light-dark(#2b2118, #efe7da);
  --soft: light-dark(#7a6a58, #a8998a); --accent: light-dark(#0b4ea2, #8db7f0);
  --line: light-dark(#e6dccd, #3a352d); --fr: light-dark(#5d6f84, #9fb2c8); }
* { box-sizing: border-box; }
body { margin: 0; background: var(--bg); color: var(--text);
  font: 18px/1.65 Georgia, "Iowan Old Style", "Palatino Linotype", serif; }
main { max-width: 42rem; margin: 0 auto; padding: 2.5rem 1rem 5rem; }
header.top { text-align: center; margin-bottom: 2.5rem; }
header.top h1 { font-size: 2rem; margin: 0 0 .2rem; }
header.top .sub { color: var(--soft); font-style: italic; margin: 0; }
header.top .intro { color: var(--soft); font-size: .9rem; margin: 1rem auto 0; max-width: 34rem; }
.bar { position: sticky; top: 0; z-index: 2; display: flex; gap: .6rem; justify-content: space-between;
  align-items: center; padding: .5rem 0; background: var(--bg); border-bottom: 1px solid var(--line);
  font: 14px/1.3 system-ui, sans-serif; }
.bar a, .bar button { color: var(--accent); background: none; border: 1px solid var(--line);
  border-radius: 999px; padding: .35rem .8rem; font: inherit; cursor: pointer; text-decoration: none; }
nav.toc { font: 15px/1.6 system-ui, sans-serif; margin: 1.5rem 0 2.5rem; columns: 2; }
nav.toc a { color: var(--accent); text-decoration: none; }
h2 { font-size: 1.5rem; margin: 3.5rem 0 .3rem; text-align: center; }
h2 small { display: block; font-size: .8rem; letter-spacing: .15em; text-transform: uppercase; color: var(--soft); }
h3 { font-size: 1.05rem; margin: 2.2rem 0 .8rem; color: var(--soft); font-style: italic; font-weight: normal; text-align: center; }
h3::before, h3::after { content: " · "; }
figure { margin: 1.2rem 0; } figure img { width: 100%; height: auto; border-radius: 10px; display: block; }
p.l { margin: .45rem 0; cursor: pointer; }
p.l .who { font-variant: small-caps; letter-spacing: .03em; font-weight: bold; }
p.n { font-style: italic; color: var(--soft); }
p.l .fr { display: none; font: italic 15px/1.5 system-ui, sans-serif; color: var(--fr); margin-top: .1rem; }
p.l.open .fr, body.all-fr p.l .fr { display: block; }
footer { margin-top: 4rem; text-align: center; color: var(--soft); font: 13px/1.5 system-ui, sans-serif; }
@media (max-width: 520px) { body { font-size: 17px; } nav.toc { columns: 1; } }
"""

JS = """
const KEY = 'story-show-fr';
const btn = document.getElementById('toggle-fr');
function apply(on) {
  document.body.classList.toggle('all-fr', on);
  btn.textContent = on ? btn.dataset.hide : btn.dataset.show;
}
let on = false;
try { on = localStorage.getItem(KEY) === '1'; } catch (e) {}
apply(on);
btn.addEventListener('click', () => {
  on = !on; apply(on);
  try { localStorage.setItem(KEY, on ? '1' : '0'); } catch (e) {}
});
document.querySelectorAll('p.l').forEach((p) => p.addEventListener('click', () => p.classList.toggle('open')));
"""


def render_html(chapters, cfg, site, chapter_label, lang_code) -> str:
    e = html.escape
    title = cfg.get("title", "L'histoire complète")
    course = site.get("short_name") or site.get("title") or ""
    out = [f"<!DOCTYPE html><html lang=\"{e(lang_code)}\"><head><meta charset=\"utf-8\">",
           "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">",
           f"<title>{e(title)} — {e(course)}</title><style>{CSS}</style></head><body><main>",
           "<div class=\"bar\">",
           f"<a href=\"./\">← {e(cfg.get('back', 'Retour au cours'))}</a>",
           f"<button id=\"toggle-fr\" type=\"button\" data-show=\"{e(cfg.get('show_translations', 'Afficher les traductions'))}\" "
           f"data-hide=\"{e(cfg.get('hide_translations', 'Masquer les traductions'))}\"></button></div>",
           "<header class=\"top\">", f"<h1>{e(course)}</h1>"]
    sub = cfg.get("subtitle")
    out.append(f"<p class=\"sub\">{e(title)}{(' · ' + e(sub)) if sub else ''}</p>")
    if cfg.get("intro"):
        out.append(f"<p class=\"intro\">{e(cfg['intro'])}</p>")
    out.append("</header><nav class=\"toc\">")
    for i, ch in enumerate(chapters, 1):
        out.append(f"<div><a href=\"#{e(ch['id'])}\">{i}. {e(ch['label'])}</a></div>")
    out.append("</nav>")
    for i, ch in enumerate(chapters, 1):
        out.append(f"<section id=\"{e(ch['id'])}\"><h2><small>{e(chapter_label)} {i}</small>{e(ch['label'])}</h2>")
        for sc in ch["scenes"]:
            out.append(f"<h3>{e(sc['title'])}</h3>")
            if sc["image"]:
                out.append(f"<figure><img src=\"{e(sc['image'])}\" alt=\"\" loading=\"lazy\"></figure>")
            for ln in sc["lines"]:
                fr = f"<span class=\"fr\">{e(ln['fr'])}</span>" if ln["fr"] else ""
                if ln["speaker"] == NARRATOR or not ln["name"] and not ln["speaker"]:
                    out.append(f"<p class=\"l n\">{e(ln['text'])}{fr}</p>")
                else:
                    who = e(ln["name"]) if ln["name"] else e(ln["speaker"] or "")
                    out.append(f"<p class=\"l\"><span class=\"who\">{who}</span> — {e(ln['text'])}{fr}</p>")
        out.append("</section>")
    out.append(f"<footer>{e(course)}</footer></main><script>{JS}</script></body></html>")
    return "\n".join(out)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--lang-dir", required=True)
    args = ap.parse_args()
    lang_dir = Path(args.lang_dir)
    lang_cfg = json.loads((lang_dir / "lang.json").read_text(encoding="utf-8"))
    data = json.loads((lang_dir / "dist" / "data.json").read_text(encoding="utf-8"))
    page = build_story(data, lang_cfg)
    if page is None:
        print("Story: no `story` section in lang.json (or no dialogue found), nothing written")
        return
    (lang_dir / "dist" / "histoire.html").write_text(page, encoding="utf-8")
    print(f"Story: {lang_dir / 'dist' / 'histoire.html'}")


if __name__ == "__main__":
    main()
