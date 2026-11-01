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
        "back": "Retour au cours",
        "listen_chapter": "Écouter le chapitre",    // optional
        "stop_listening": "Pause"                  // optional
    }

Without a "story" key nothing is generated. Only the audio cards under one
of the `headings` (the dialogue sections) are kept; the speaker names come
from the sheet's character list ("Les personnages"), and an illustration
(img/illus_*) of a sheet is shown at its place in the story.

The page is listenable: the speaker's emoji is a button that reads the line,
and each chapter has a "listen" button reading it all. It uses the browser's
speech synthesis with the learner's settings from the course (same
localStorage key: voice, rate, pitch, per-character voices; the pause between
lines is capped at 0.6 s, a story reads better than a drill).

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


CREDIT_RE = re.compile(
    r"^(.*?)[\s.,;–—-]*(\((?:photo|foto|source|zdroj|crédit|credit)\s*:[^()]*\)|\b(?:photo|foto|source|zdroj|crédit|credit)\s*:.*)$",
    re.I | re.S)


def _split_credit(caption: str) -> tuple[str, str]:
    """Same split as the front end (app.js splitCaptionCredit)."""
    m = CREDIT_RE.match(caption)
    if not m or not m.group(1).strip():
        return caption, ""
    return m.group(1).strip(), re.sub(r"^\((.*)\)$", r"\1", m.group(2).strip(), flags=re.S)


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


def build_story(data: dict, lang_cfg: dict, map_data: dict | None = None) -> str | None:
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
        geo_points: list[dict] = []
        geo_layer = None
        chapter_names: dict[str, str] = {}
        for sheet in group.get("sheets", []):
            chapter_names.update(_sheet_characters(sheet, user_name))
        for sheet in group.get("sheets", []):
            names = _sheet_characters(sheet, user_name)
            lines = []
            current = None
            for block in sheet.get("content", []):
                t = block.get("type")
                if t == "geomap":
                    geo_layer = geo_layer or block.get("layer")
                    for pt in block.get("points", []):
                        label = _clean(pt.get("label") or "", user_name)
                        if not any(abs(q["lat"] - pt["lat"]) < 1e-4 and abs(q["lon"] - pt["lon"]) < 1e-4 for q in geo_points):
                            geo_points.append({"lat": pt["lat"], "lon": pt["lon"], "label": label})
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
            src = image.get("src") or ""
            illus = src if "illus_" in src else None
            photo = None
            if src and not src.startswith("TODO"):
                text, credit = _split_credit(_clean(image.get("caption") or "", user_name))
                photo = {"src": src, "caption": text, "credit": credit}
            scenes.append({"id": sheet["id"], "title": title, "lines": lines,
                           "image": photo if illus else None, "photo": None if illus else photo})
        if scenes:
            geo = None
            if geo_points:
                geo = {"type": "geomap", "title": "", "route": False, "zoom": None,
                       "layer": geo_layer or "osm", "points": geo_points}
            chapters.append({"id": sub, "label": labels.get(sub, group.get("subgroup_label") or sub),
                             "scenes": scenes, "geo": geo})
    if not chapters:
        return None
    cover = None
    if map_data and map_data.get("config", {}).get("background"):
        number = {ch["id"]: i for i, ch in enumerate(chapters, 1)}
        marks = []
        for step in map_data.get("steps", []):
            sid = step.get("id", "")
            sub = sid[len("series-"):] if sid.startswith("series-") else None
            if sub in number:
                marks.append({"x": step["x"], "y": step["y"], "n": number[sub], "id": sub,
                              "label": labels.get(sub, step.get("title", sub)), "icon": step.get("icon", "")})
        cover = {"src": map_data["config"]["background"], "marks": marks}
    tts = {"code": (target.get("tts_code") if isinstance(target, dict) else None) or target_code,
           "hint": (target.get("voice_hint") if isinstance(target, dict) else None) or "",
           "prefix": site.get("storage_prefix") or "slovingo"}
    return render_html(chapters, cfg, site, chapter_label, target_code, cover, tts)


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
figure { margin: 1.2rem 0; text-align: center; }
figure img { max-width: 100%; max-height: 70vh; width: auto; height: auto; border-radius: 10px; display: block; margin: 0 auto; }
figure.zoomable img { cursor: zoom-in; }
dialog.lightbox { outline: none; border: 0; padding: 0; background: transparent; max-width: 96vw; max-height: 96vh; overflow: visible; }
dialog.lightbox::backdrop { background: rgb(0 0 0 / .85); }
dialog.lightbox img { max-width: 96vw; max-height: 80vh; border-radius: 8px; display: block; margin: 0 auto; }
dialog.lightbox p { color: #eee; font: 14px/1.4 system-ui, sans-serif; text-align: center; margin: .6rem auto 0; max-width: 40rem; }
dialog.lightbox p small { opacity: .7; }
p.l { margin: .25rem 0; cursor: pointer; display: flex; gap: .6rem; align-items: flex-start;
  border-radius: 10px; padding: .1rem .3rem; transition: background .2s; line-height: 1.5; }
p.l .av { flex: none; width: 2.2rem; height: 2.2rem; border-radius: 50%; border: 1px solid var(--line);
  background: light-dark(#fff, #26231f); font-size: 1.3rem; line-height: 1; cursor: pointer; padding: 0;
  display: flex; align-items: center; justify-content: center; margin-top: .1rem; }
p.l .av:hover { border-color: var(--accent); }
body.no-tts p.l .av { cursor: default; }
p.l .body { flex: 1; min-width: 0; }
p.l.speaking { background: light-dark(#fff3cf, #3a3322); }
p.n { font-style: italic; color: var(--text); opacity: .85; padding-left: .9rem; margin: .5rem 0;
  border-left: 3px solid var(--accent); border-radius: 0; }
button.listen { display: block; margin: .2rem auto 1rem; color: var(--accent); background: none;
  border: 1px solid var(--line); border-radius: 999px; padding: .4rem 1rem; font: 14px/1.3 system-ui, sans-serif; cursor: pointer; }
body.no-tts button.listen { display: none; }
p.l .fr { display: none; font: italic 15px/1.5 system-ui, sans-serif; color: var(--fr); margin-top: .1rem; }
p.l.open .fr, body.all-fr p.l .fr { display: block; }
footer { margin-top: 4rem; text-align: center; color: var(--soft); font: 13px/1.5 system-ui, sans-serif; }
.cover { position: relative; margin: 0 0 1.5rem; }
.cover img { width: 100%; height: auto; display: block; border-radius: 12px; }
.cover a { position: absolute; transform: translate(-50%, -50%); width: 1.9rem; height: 1.9rem;
  border-radius: 50%; background: rgb(255 255 255 / 0.92); color: #1b1a17; border: 2px solid #0b4ea2;
  display: flex; align-items: center; justify-content: center; text-decoration: none;
  font: bold 14px/1 system-ui, sans-serif; box-shadow: 0 1px 4px rgb(0 0 0 / 0.35); }
.cover a:hover { background: #0b4ea2; color: #fff; }
figure.geo { margin: 1rem 0 1.5rem; }
figure.geo .geomap { height: 220px; border-radius: 10px; overflow: hidden; border: 1px solid var(--line); }
figure.photo img { max-height: 45vh; }
section .scene-end { clear: both; }
.geomap-step span { display: none; }
@media (max-width: 520px) { body { font-size: 17px; } nav.toc { columns: 1; } figure img { max-width: 75%; } }
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
document.querySelectorAll('[data-geo]').forEach((el) => {
  if (window.Geomap) window.Geomap.mount(el, JSON.parse(el.dataset.geo));
});
document.querySelectorAll('p.l').forEach((p) => p.addEventListener('click', () => p.classList.toggle('open')));

// ---- Zoom on images (the caption and credit appear only there) ----
const lb = document.getElementById('lightbox');
document.querySelectorAll('figure.zoomable img').forEach((img) => img.addEventListener('click', () => {
  lb.querySelector('img').src = img.src;
  const cap = lb.querySelector('p'); cap.textContent = img.dataset.caption || '';
  if (img.dataset.credit) { const sm = document.createElement('small'); sm.textContent = ' ' + img.dataset.credit; cap.appendChild(sm); }
  lb.showModal();
}));
lb.addEventListener('click', () => lb.close());

// ---- Listening (browser speech synthesis, same settings as the course) ----
const synth = window.speechSynthesis;
function settings() {
  const d = { voiceURI: null, rate: 0.9, pitch: 1, dialoguePauseDuration: 2, characterVoices: {} };
  try { return Object.assign(d, JSON.parse(localStorage.getItem(TTS.prefix + '-settings')) || {}); }
  catch (e) { return d; }
}
function findVoice(uri) {
  const vs = synth.getVoices();
  if (!vs.length) return null;
  if (uri) { const c = vs.find((v) => v.voiceURI === uri); if (c) return c; }
  const code = (TTS.code || '').toLowerCase(), pre = code.split('-')[0];
  const norm = (v) => v.lang.toLowerCase().replace(/_/g, '-');
  return vs.find((v) => norm(v) === code) || vs.find((v) => norm(v).split('-')[0] === pre)
    || (TTS.hint && vs.find((v) => v.name.toLowerCase().includes(TTS.hint.toLowerCase()))) || null;
}
function refreshAvailability() {
  document.body.classList.toggle('no-tts', !('speechSynthesis' in window) || !findVoice(settings().voiceURI));
}
let token = 0, activeBtn = null, resume = new Map();
function stopAll() {
  token++;
  if (synth) synth.cancel();
  document.querySelectorAll('p.l.speaking').forEach((p) => p.classList.remove('speaking'));
  if (activeBtn) { activeBtn.textContent = '▶ ' + activeBtn.dataset.play; activeBtn = null; }
}
function speakLine(p, done) {
  const s = settings(), sp = p.dataset.speaker;
  const o = (sp && s.characterVoices && s.characterVoices[sp]) || {};
  const u = new SpeechSynthesisUtterance(p.querySelector('.t').textContent);
  u.lang = TTS.code;
  u.rate = o.rate != null ? o.rate : s.rate;
  u.pitch = o.pitch != null ? o.pitch : s.pitch;
  const v = findVoice(o.voiceURI != null ? o.voiceURI : s.voiceURI);
  if (v) u.voice = v;
  const my = token;
  p.classList.add('speaking');
  let fired = false;
  const end = () => { if (fired) return; fired = true; p.classList.remove('speaking'); if (my === token) done(); };
  u.onend = end; u.onerror = end;
  synth.speak(u);
}
function playFrom(lines, i, btn) {
  if (i >= lines.length) { resume.delete(btn); stopAll(); return; }
  resume.set(btn, i);
  const my = token;
  lines[i].scrollIntoView({ block: 'center', behavior: 'smooth' });
  speakLine(lines[i], () => {
    if (my !== token) return;
    const pause = Math.min(0.6, Math.max(0, Number(settings().dialoguePauseDuration) || 0)) * 1000; // reading: short pause
    setTimeout(() => { if (my === token) playFrom(lines, i + 1, btn); }, pause);
  });
}
if ('speechSynthesis' in window) {
  refreshAvailability();
  synth.addEventListener('voiceschanged', refreshAvailability);
  document.querySelectorAll('button.listen').forEach((btn) => {
    btn.textContent = '▶ ' + btn.dataset.play;
    btn.addEventListener('click', () => {
      const was = activeBtn === btn;
      stopAll();
      if (was) { btn.textContent = '▶ ' + btn.dataset.play; return; }
      const lines = [...btn.closest('section').querySelectorAll('p.l')];
      activeBtn = btn; btn.textContent = '⏸ ' + btn.dataset.pause;
      playFrom(lines, resume.get(btn) || 0, btn);
    });
  });
  document.querySelectorAll('p.l .av').forEach((b) => b.addEventListener('click', (ev) => {
    ev.stopPropagation();
    if (document.body.classList.contains('no-tts')) return;
    stopAll();
    speakLine(b.closest('p.l'), () => {});
  }));
  window.addEventListener('pagehide', stopAll);
} else {
  document.body.classList.add('no-tts');
}
"""


def render_html(chapters, cfg, site, chapter_label, lang_code, cover=None, tts=None) -> str:
    e = html.escape
    title = cfg.get("title", "L'histoire complète")
    listen_line = cfg.get("listen_line", "Écouter")
    tts = tts or {}
    course = site.get("short_name") or site.get("title") or ""
    out = [f"<!DOCTYPE html><html lang=\"{e(lang_code)}\"><head><meta charset=\"utf-8\">",
           "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">",
           f"<title>{e(title)} — {e(course)}</title><style>{CSS}</style>",
           "<link rel=\"stylesheet\" href=\"vendor/leaflet/leaflet.css\"><link rel=\"stylesheet\" href=\"geomap.css\">",
           "</head><body><main>",
           "<div class=\"bar\">",
           f"<a href=\"./\">← {e(cfg.get('back', 'Retour au cours'))}</a>",
           f"<button id=\"toggle-fr\" type=\"button\" data-show=\"{e(cfg.get('show_translations', 'Afficher les traductions'))}\" "
           f"data-hide=\"{e(cfg.get('hide_translations', 'Masquer les traductions'))}\"></button></div>",
           "<header class=\"top\">", f"<h1>{e(course)}</h1>"]
    sub = cfg.get("subtitle")
    out.append(f"<p class=\"sub\">{e(title)}{(' · ' + e(sub)) if sub else ''}</p>")
    if cfg.get("intro"):
        out.append(f"<p class=\"intro\">{e(cfg['intro'])}</p>")
    out.append("</header>")
    if cover:
        out.append(f"<figure class=\"cover\"><img src=\"{e(cover['src'])}\" alt=\"\">")
        for m in cover["marks"]:
            out.append(f"<a href=\"#{e(m['id'])}\" style=\"left:{m['x']}%;top:{m['y']}%\" title=\"{e(str(m['n']) + '. ' + m['label'])}\">{m['n']}</a>")
        out.append("</figure>")
    out.append("<nav class=\"toc\">")
    for i, ch in enumerate(chapters, 1):
        out.append(f"<div><a href=\"#{e(ch['id'])}\">{i}. {e(ch['label'])}</a></div>")
    out.append("</nav>")
    for i, ch in enumerate(chapters, 1):
        out.append(f"<section id=\"{e(ch['id'])}\"><h2><small>{e(chapter_label)} {i}</small>{e(ch['label'])}</h2>")
        out.append(f"<button class=\"listen\" type=\"button\" data-play=\"{e(cfg.get('listen_chapter', 'Écouter le chapitre'))}\" "
                   f"data-pause=\"{e(cfg.get('stop_listening', 'Pause'))}\"></button>")
        if ch.get("geo"):
            out.append(f"<figure class=\"geo\"><div class=\"geomap\" data-geo='{e(json.dumps(ch['geo'], ensure_ascii=False))}'></div></figure>")
        for sc in ch["scenes"]:
            out.append(f"<h3>{e(sc['title'])}</h3>")
            if sc["image"]:
                ph = sc["image"]
                out.append(f"<figure class=\"zoomable\"><img src=\"{e(ph['src'])}\" alt=\"{e(ph['caption'])}\" "
                           f"data-caption=\"{e(ph['caption'])}\" data-credit=\"{e(ph['credit'])}\" loading=\"lazy\"></figure>")
            elif sc.get("photo"):
                ph = sc["photo"]
                out.append(f"<figure class=\"photo zoomable\"><img src=\"{e(ph['src'])}\" alt=\"{e(ph['caption'])}\" "
                           f"data-caption=\"{e(ph['caption'])}\" data-credit=\"{e(ph['credit'])}\" loading=\"lazy\"></figure>")
            for ln in sc["lines"]:
                fr = f"<span class=\"fr\">{e(ln['fr'])}</span>" if ln["fr"] else ""
                if ln["speaker"] == NARRATOR or not ln["name"] and not ln["speaker"]:
                    out.append(f"<p class=\"l n\"><span class=\"body\"><span class=\"t\">{e(ln['text'])}</span>{fr}</span></p>")
                else:
                    sp = ln["speaker"] or ""
                    tip = e(ln["name"] or listen_line)
                    out.append(f"<p class=\"l\" data-speaker=\"{e(sp)}\"><button class=\"av\" type=\"button\" "
                               f"title=\"{tip}\" aria-label=\"{tip}\">{e(sp)}</button>"
                               f"<span class=\"body\"><span class=\"t\">{e(ln['text'])}</span>{fr}</span></p>")
            out.append("<div class=\"scene-end\"></div>")
        out.append("</section>")
    out.append(f"<footer>{e(course)}</footer></main><dialog class=\"lightbox\" id=\"lightbox\"><img alt=\"\"><p></p></dialog><script src=\"geomap.js\"></script><script>const TTS = {json.dumps(tts, ensure_ascii=False)};{JS}</script></body></html>")
    return "\n".join(out)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--lang-dir", required=True)
    args = ap.parse_args()
    lang_dir = Path(args.lang_dir)
    lang_cfg = json.loads((lang_dir / "lang.json").read_text(encoding="utf-8"))
    data = json.loads((lang_dir / "dist" / "data.json").read_text(encoding="utf-8"))
    map_path = lang_dir / "dist" / "map.json"
    map_data = json.loads(map_path.read_text(encoding="utf-8")) if map_path.exists() else None
    page = build_story(data, lang_cfg, map_data)
    if page is None:
        print("Story: no `story` section in lang.json (or no dialogue found), nothing written")
        return
    (lang_dir / "dist" / "histoire.html").write_text(page, encoding="utf-8")
    print(f"Story: {lang_dir / 'dist' / 'histoire.html'}")


if __name__ == "__main__":
    main()
