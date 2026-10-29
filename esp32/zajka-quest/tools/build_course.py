#!/usr/bin/env python3
"""Convertit le cours Zajka Friends (md/ + exercises/) en donnees C pour le jeu (src/game/gen/).

    python3 src/langs.py sk-fr-friends               # a la racine du depot : recupere le cours
    python3 tools/build_course.py                     # episodes par defaut (EPISODES ci-dessous)
    python3 tools/build_course.py --course ~/Slovingo-sk-fr-friends 10_Series_01_Kosice_01_to-si-ty
    python3 tools/build_course.py --dump              # affiche ce qui a ete lu, sans rien ecrire

Ce qui est repris de chaque fiche (format : docs/Format-friends.md du cours) :
  « # Série Košice (1/5) — To si ty? »   serie et titre
  « % lat, lon | {{lieu}} | z12 »        le lieu (le premier)
  « ## Le dialogue » (extra : « ## La suite de l'épisode »)
                                          les repliques : « ! 🦊 texte », puis « > traduction »,
                                          « > mot = sens » (explications), « + aide »
  « ## Encore quelques répliques »        repliques sans avatar, pour les PNJ de passage
  « ## Les nouveaux mots »                le vocabulaire
  exercises/<fiche>.exercises.json        exercices qcm, listen et order

Les repliques du renard deviennent des choix quand un exercice s'y prete : « order » dont la
phrase est la replique (remettre les morceaux dans l'ordre), sinon « qcm » dont le mot figure
dans la replique (choisir la bonne phrase parmi celles ou le mot est remplace par les mauvaises
reponses). Les repliques secretes (bonus de comprehension) viennent de story/bonus.txt.

Le marquage {{fr:...}} / {{...}} est retire, [USER_NAME] devient NAME_MARK (\\001) et les
caracteres absents de la police du jeu (emojis surtout) sont enleves, avec un avertissement.
Les fichiers generes sont commites : la compilation PlatformIO n'a pas besoin du cours.
"""
import argparse
import glob
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
GEN = os.path.join(ROOT, "src", "game", "gen")
BONUS = os.path.join(ROOT, "story", "bonus.txt")
LINES = os.path.join(ROOT, "story", "lines.txt")
DEFAULT_COURSE = os.path.normpath(os.path.join(ROOT, "..", "..", "langs", "sk-fr-friends"))

sys.path.insert(0, HERE)
from build_assets import CHARSET  # noqa: E402  (les caracteres que les polices savent dessiner)

# Les fiches jouees, dans l'ordre du jeu (COURSE_EPISODES[0] = la premiere). Chapitre 1 : Kosice.
EPISODES = [
    "10_Series_01_Kosice_01_to-si-ty",
    "10_Series_01_Kosice_02_ako-sa-mas",
    "10_Series_01_Kosice_03_co-tu-robis",
    "10_Series_01_Kosice_04_pamatas-sa",
    "10_Series_01_Kosice_05_v-elektricke",
    "10_Series_01_Kosice_06_extra",
    "10_Series_02_Sucho_01_je-horuco",
    "10_Series_02_Sucho_02_vela-alebo-velmi",
    "10_Series_02_Sucho_03_chyba-mi-voda",
    "10_Series_02_Sucho_04_co-budeme-robit",
    "10_Series_02_Sucho_05_fontana",
    "10_Series_02_Sucho_06_extra",
    "10_Series_03_Raj_01_kam-teraz",
    "10_Series_03_Raj_02_pocujes",
    "10_Series_03_Raj_03_je-tu-voda",
    "10_Series_03_Raj_04_rebriky",
    "10_Series_03_Raj_05_roklina",
    "10_Series_03_Raj_06_extra",
]

# emoji du cours -> constante Speaker de src/game/course.h, nom affiche dans la boite de dialogue
SPEAKERS = [
    ("", "SPK_NONE", ""),
    ("💬", "SPK_NARRATOR", ""),
    ("🦊", "SPK_FOX", "\x01"),
    ("🐰", "SPK_ANDREA", "Andrea"),
    ("🐻", "SPK_MATO", "Maťo"),
    ("🐹", "SPK_KATKA", "Katka"),
    ("🐑", "SPK_BABKA", "Babka Zuzana"),
    ("🦅", "SPK_OROL", "Pán Orol"),
    ("🦔", "SPK_JEZKOVA", "Pani Ježková"),
    ("🧑", "SPK_HUMAN", ""),
    ("🐿", "SPK_JAN", "Ján"),
    ("🦡", "SPK_MAREK", "Marek"),
    ("🐭", "SPK_MOUSE", ""),
    ("🐥", "SPK_CHICK", ""),
    ("🐦", "SPK_BIRD", ""),
]
EXERCISE_TYPES = {"qcm": "EX_QCM", "listen": "EX_LISTEN", "order": "EX_ORDER"}
FONT_CHARS = set(CHARSET)
NAME_MARK = "\x01"


class Warn:
    def __init__(self):
        self.dropped = {}   # caractere -> nombre
        self.other = []

    def drop(self, ch):
        self.dropped[ch] = self.dropped.get(ch, 0) + 1

    def report(self, where):
        if self.dropped:
            chars = " ".join(f"{c!r}×{n}" for c, n in sorted(self.dropped.items()))
            print(f"  {where} : caracteres hors police retires : {chars}")
        for w in self.other:
            print(f"  {where} : {w}")


def clean(text, warn):
    """Texte du cours -> texte affichable par le jeu."""
    text = re.sub(r"\{\{fr:(.*?)\}\}", r"\1", text)
    text = re.sub(r"\{\{(.*?)\}\}", r"\1", text)
    text = text.replace("[USER_NAME]", NAME_MARK)
    text = text.replace(" ", " ").replace(" ", " ")
    out = []
    for ch in text:
        if ch == NAME_MARK or ch in FONT_CHARS:
            out.append(ch)
        elif ch in "️‍":
            continue  # selecteurs de variante des emojis : rien a signaler
        else:
            warn.drop(ch)
    return re.sub(r"\s+", " ", "".join(out)).strip()


def speaker_of(rest):
    """« 🦊 Počkaj... » -> ("SPK_FOX", "Počkaj...")."""
    for emoji, const, _ in SPEAKERS[1:]:
        if rest.startswith(emoji):
            rest = rest[len(emoji):]
            # avatar compose (« 🦊🦺 ») : on garde le personnage, on jette l'accessoire
            rest = re.sub(r"^[^\w\s\"«(\[.¡¿-]+", "", rest)
            return const, rest.strip()
    return "SPK_NONE", rest.strip()


def parse_cards(lines, warn):
    """Les cartes « ! » d'une section -> liste de repliques."""
    cards, cur = [], None
    for raw in lines:
        line = raw.rstrip()
        if line.startswith("! "):
            spk, text = speaker_of(line[2:])
            cur = {"speaker": spk, "sk": clean(text, warn), "fr": None, "gloss": [], "help": []}
            cards.append(cur)
        elif cur is None:
            continue
        elif line.startswith("> "):
            body = line[2:].strip()
            if cur["fr"] is None:
                cur["fr"] = clean(body, warn)
            elif " = " in body:
                sk, fr = body.split(" = ", 1)
                cur["gloss"].append((clean(sk, warn), clean(fr, warn)))
            else:
                warn.other.append(f"ligne « > » sans « = » ignoree : {body[:40]}")
        elif line.startswith("+ "):
            cur["help"].append(clean(line[2:], warn))
        elif line.strip() == "":
            continue
        else:
            cur = None  # fin de la carte
    for c in cards:
        if c["fr"] is None:
            warn.other.append(f"replique sans traduction : {c['sk'][:40]}")
            c["fr"] = ""
    return cards


def parse_table(lines, warn):
    rows = []
    for line in lines:
        if not line.startswith("|"):
            continue
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        if len(cells) < 2 or set(cells[0]) <= set("-: ") or cells[0] == "Slovenčina":
            continue
        rows.append((clean(cells[0], warn), clean(cells[1], warn)))
    return rows


def sections(text):
    out, name = {}, ""
    for line in text.split("\n"):
        if line.startswith("## "):
            name = line[3:].strip()
            out.setdefault(name, [])
        else:
            out.setdefault(name, []).append(line)
    return out


def parse_episode(course, stem):
    warn = Warn()
    text = open(os.path.join(course, "md", stem + ".md"), encoding="utf-8").read()
    m = re.match(r"\d+_Series_(\d+)_[^_]+_(\d+)_", stem)
    if not m:
        sys.exit(f"{stem} : nom de fiche inattendu (attendu NN_Series_SS_Nom_EE_...)")
    ep = {"id": stem, "seriesNum": int(m.group(1)), "episodeNum": int(m.group(2))}
    t = re.search(r"^# (?:Série )?(.+?) \((?:\d+/\d+|extra)\) — (.+)$", text, re.M)
    if not t:
        sys.exit(f"{stem} : titre « # Série X (n/5) — titre » ou « # X (extra) — titre » introuvable")
    ep["series"], ep["title"] = clean(t.group(1), warn), clean(t.group(2), warn)
    p = re.search(r"^% [-\d.]+, [-\d.]+ \| (.+?) \|", text, re.M)
    ep["place"] = clean(p.group(1), warn) if p else ep["series"]
    sec = sections(text)
    # fiches 01 a 05 : « Le dialogue » ; fiche 06 (extra) : « La suite de l'épisode »
    ep["dialog"] = parse_cards(sec.get("Le dialogue", []) or sec.get("La suite de l'épisode", []), warn)
    ep["extra"] = parse_cards(sec.get("Encore quelques répliques", []), warn)
    ep["words"] = parse_table(sec.get("Les nouveaux mots", []), warn)
    # tout le vocabulaire de la fiche (nouveaux mots, complementaire...) : pour le Slovnik
    ep["vocab"] = []
    for name, lines in sec.items():
        if "ocabulaire" in name or "nouveaux mots" in name:
            ep["vocab"] += parse_table(lines, warn)
    if not ep["dialog"]:
        warn.other.append("pas de section « ## Le dialogue »")
    ep["exercises"] = []
    exf = os.path.join(course, "exercises", stem + ".exercises.json")
    if os.path.exists(exf):
        for ex in json.load(open(exf, encoding="utf-8")).get("exercises", []):
            if ex.get("type") not in EXERCISE_TYPES:
                continue
            e = {"type": EXERCISE_TYPES[ex["type"]], "id": ex["id"],
                 "sk": clean(ex.get("l2", ""), warn), "fr": clean(ex.get("l1", ""), warn),
                 "choicesSk": [clean(c, warn) for c in ex.get("choices_l2", [])],
                 "choicesFr": [clean(c, warn) for c in ex.get("choices_l1", [])],
                 "tokensSk": [clean(c, warn) for c in ex.get("tokens_l2", [])],
                 "tokensFr": [clean(c, warn) for c in ex.get("tokens_l1", [])]}
            if e["choicesSk"] and len(e["choicesSk"]) != len(e["choicesFr"]):
                warn.other.append(f"{e['id']} : choix SK et FR de longueurs differentes")
            ep["exercises"].append(e)
    add_choices(ep, warn)
    warn.report(stem)
    return ep


def norm(s):
    return re.sub(r"\s+", " ", s).strip().lower()


def add_choices(ep, warn):
    """Repliques du renard -> choix tires des exercices order, puis qcm."""
    for c in ep["dialog"]:
        c["choice"] = None
        if c["speaker"] != "SPK_FOX":
            continue
        for e in ep["exercises"]:
            if e["type"] == "EX_ORDER" and norm(e["sk"]) == norm(c["sk"]) and len(e["tokensSk"]) >= 2:
                c["choice"] = ("CH_ORDER", e["id"], e["tokensSk"])
                break
        if c["choice"]:
            continue
        for e in ep["exercises"]:
            if e["type"] != "EX_QCM" or not e["choicesSk"]:
                continue
            pat = re.compile(r"(?<!\w)" + re.escape(e["sk"]) + r"(?!\w)", re.I)
            if not pat.search(c["sk"]):
                continue
            wrong = []
            for alt in e["choicesSk"]:
                # un mot deja dans la phrase donnerait « Aj ja ťa ťa vidím » : on l'ecarte
                if re.search(r"(?<!\w)" + re.escape(alt) + r"(?!\w)", c["sk"], re.I):
                    continue
                cand = pat.sub(lambda m: alt[0].upper() + alt[1:] if m.group(0)[0].isupper() else alt, c["sk"], 1)
                if norm(cand) != norm(c["sk"]) and cand not in wrong:
                    wrong.append(cand)
            if wrong:
                c["choice"] = ("CH_PICK", e["id"], [c["sk"]] + wrong[:2])
                break


def load_bonus(warn):
    """story/bonus.txt -> {fiche: [replique, ...]}."""
    out, cur = {}, None
    if not os.path.exists(BONUS):
        return out
    for n, line in enumerate(open(BONUS, encoding="utf-8"), 1):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if line.startswith("[") and line.endswith("]"):
            cur = out.setdefault(line[1:-1], [])
            continue
        if cur is None or "|" not in line:
            sys.exit(f"story/bonus.txt:{n} : ligne inattendue (attendu « [fiche] » ou « emoji texte | traduction »)")
        sk, fr = (t.strip() for t in line.split("|", 1))
        spk, text = speaker_of(sk)
        cur.append({"speaker": spk, "sk": clean(text, warn), "fr": clean(fr, warn), "gloss": [], "help": [],
                    "choice": None})
    return out


def load_lines(warn):
    """story/lines.txt -> {nom: [replique, ...]} : les repliques propres au jeu.

    [nom]
    emoji texte | traduction
    emoji texte | traduction || mauvaise reponse || autre      (replique a choisir)
    emoji texte | traduction >> morceau / morceau / morceau    (replique a remettre dans l'ordre)
    + aide (pour la replique precedente)
    """
    out, cur = {}, None
    if not os.path.exists(LINES):
        return out
    for n, line in enumerate(open(LINES, encoding="utf-8"), 1):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if line.startswith("[") and line.endswith("]"):
            cur = out.setdefault(line[1:-1], [])
            continue
        if cur is None:
            sys.exit(f"story/lines.txt:{n} : replique hors section « [nom] »")
        if line.startswith("+ "):
            if not cur:
                sys.exit(f"story/lines.txt:{n} : aide sans replique")
            cur[-1]["help"].append(clean(line[2:], warn))
            continue
        tokens = None
        if ">>" in line:
            line, tok = line.split(">>", 1)
            tokens = [clean(t.strip(), warn) for t in tok.split(" / ")]
        main, *alts = [t.strip() for t in line.split("||")]
        if "|" not in main:
            sys.exit(f"story/lines.txt:{n} : « texte | traduction » attendu")
        sk, fr = (t.strip() for t in main.split("|", 1))
        spk, text = speaker_of(sk)
        c = {"speaker": spk, "sk": clean(text, warn), "fr": clean(fr, warn), "gloss": [], "help": [], "choice": None}
        if alts:
            c["choice"] = ("CH_PICK", "lines", [c["sk"]] + [clean(a, warn) for a in alts])
        if tokens:
            if norm(" ".join(tokens)) != norm(c["sk"]):
                sys.exit(f"story/lines.txt:{n} : les morceaux ne refont pas la replique")
            c["choice"] = ("CH_ORDER", "lines", tokens)
        cur.append(c)
    return out


# ---------------------------------------------------------------- sortie C
def c_str(s):
    if s is None:
        return "nullptr"
    s = s.replace("\\", "\\\\").replace('"', '\\"').replace(NAME_MARK, "\\001")
    return '"' + s + '"'


def c_strs(name, items, out):
    if not items:
        return "nullptr"
    out.append(f"static const char* const {name}[{len(items)}] = {{ " + ", ".join(c_str(s) for s in items) + " };")
    return name


def c_lines(name, cards, out):
    if not cards:
        return "nullptr"
    rows = []
    for i, c in enumerate(cards):
        gname = "nullptr"
        if c["gloss"]:
            gname = f"{name}_g{i}"
            out.append(f"static const Gloss {gname}[{len(c['gloss'])}] = {{ "
                       + ", ".join(f"{{{c_str(a)}, {c_str(b)}}}" for a, b in c["gloss"]) + " };")
        help_ = " ".join(c["help"]) if c["help"] else None
        chname = "nullptr"
        if c.get("choice"):
            kind, exid, items = c["choice"]
            iname = c_strs(f"{name}_c{i}_items", items, out)
            chname = f"&{name}_c{i}"
            out.append(f"static const Choice {name}_c{i} = {{ {kind}, {c_str(exid)}, {iname}, {len(items)} }};")
        rows.append(f"  {{{c['speaker']}, {c_str(c['sk'])}, {c_str(c['fr'])}, {gname}, "
                    f"{len(c['gloss'])}, {c_str(help_)}, {chname}}},")
    out.append(f"static const Line {name}[{len(cards)}] = {{\n" + "\n".join(rows) + "\n};")
    return name


def write(episodes, default_name, scripts):
    out = ["// Genere par tools/build_course.py depuis le cours Slovingo-sk-fr-friends : ne pas modifier.",
           '#include "../course.h"', ""]
    names = []
    for k, ep in enumerate(episodes):
        p = f"ep{k}"
        dialog = c_lines(f"{p}_dialog", ep["dialog"], out)
        extra = c_lines(f"{p}_extra", ep["extra"], out)
        words = "nullptr"
        if ep["words"]:
            words = f"{p}_words"
            out.append(f"static const Word {words}[{len(ep['words'])}] = {{\n"
                       + "\n".join(f"  {{{c_str(a)}, {c_str(b)}}}," for a, b in ep["words"]) + "\n};")
        exs = "nullptr"
        if ep["exercises"]:
            rows = []
            for i, e in enumerate(ep["exercises"]):
                q = f"{p}_x{i}"
                cs = c_strs(f"{q}_csk", e["choicesSk"], out)
                cf = c_strs(f"{q}_cfr", e["choicesFr"], out)
                ts = c_strs(f"{q}_tsk", e["tokensSk"], out)
                tf = c_strs(f"{q}_tfr", e["tokensFr"], out)
                rows.append(f"  {{{e['type']}, {c_str(e['id'])}, {c_str(e['sk'])}, {c_str(e['fr'])}, "
                            f"{cs}, {cf}, {max(len(e['choicesSk']), len(e['choicesFr']))}, "
                            f"{ts}, {tf}, {len(e['tokensSk'])}}},")
            exs = f"{p}_exercises"
            out.append(f"static const Exercise {exs}[{len(rows)}] = {{\n" + "\n".join(rows) + "\n};")
        bonus = c_lines(f"{p}_bonus", ep["bonus"], out)
        out.append(f"static const Episode {p} = {{ {c_str(ep['id'])}, {c_str(ep['series'])}, {ep['seriesNum']}, "
                   f"{ep['episodeNum']}, {c_str(ep['title'])}, {c_str(ep['place'])},\n"
                   f"  {dialog}, {len(ep['dialog'])}, {extra}, {len(ep['extra'])}, {words}, {len(ep['words'])}, "
                   f"{exs}, {len(ep['exercises'])}, {bonus}, {len(ep['bonus'])} }};")
        out.append("")
        names.append(p)
    out.append("const Episode* const COURSE_EPISODES[] = { " + ", ".join(f"&{n}" for n in names) + " };")
    # tout le vocabulaire (Slovnik), sans doublon
    vocab, seen = [], set()
    for ep in episodes:
        for sk, fr in ep["vocab"]:
            if sk.lower() not in seen:
                seen.add(sk.lower())
                vocab.append((sk, fr))
    out.append(f"const Word COURSE_VOCAB[{max(1, len(vocab))}] = {{\n"
               + "\n".join(f"  {{{c_str(a)}, {c_str(b)}}}," for a, b in vocab or [("", "")]) + "\n};")
    out.append(f"const int COURSE_NUM_VOCAB = {len(vocab)};")
    # repliques propres au jeu (story/lines.txt)
    snames = []
    for k, (name, cards) in enumerate(scripts.items()):
        arr = c_lines(f"gl{k}", cards, out)
        snames.append((name, arr, len(cards)))
    out.append(f"const Script SCRIPTS[{max(1, len(snames))}] = {{ "
               + ", ".join(f"{{{a}, {n}}}" for _, a, n in snames or [("", "nullptr", 0)]) + " };")
    out.append(f"const int NUM_SCRIPTS = {len(snames)};")
    with open(os.path.join(GEN, "scripts.h"), "w", encoding="utf-8") as f:
        f.write("// Genere par tools/build_course.py depuis story/lines.txt : ne pas modifier.\n#pragma once\n\n"
                + "\n".join(f"constexpr int SC_{re.sub(r'[^A-Za-z0-9]', '_', n).upper()} = {i};"
                             for i, (n, _, _) in enumerate(snames)) + "\n")
    out.append(f"const int COURSE_NUM_EPISODES = {len(names)};")
    out.append(f"const char* const COURSE_DEFAULT_NAME = {c_str(default_name)};")
    out.append("const char* const SPEAKER_NAMES[SPK_COUNT] = { "
               + ", ".join(c_str(n) for _, _, n in SPEAKERS) + " };")
    os.makedirs(GEN, exist_ok=True)
    path = os.path.join(GEN, "course_data.inc")
    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join(out) + "\n")
    print(f"src/game/gen/course_data.inc : {len(episodes)} episode(s), {os.path.getsize(path) // 1024} Ko de source")


def dump(ep):
    print(f"\n== {ep['id']} : série {ep['series']} ({ep['seriesNum']}), épisode {ep['episodeNum']} "
          f"« {ep['title']} », lieu {ep['place']}")
    for title, cards in (("dialogue", ep["dialog"]), ("répliques en plus", ep["extra"])):
        print(f"-- {title} ({len(cards)})")
        for c in cards:
            print(f"  [{c['speaker'][4:]:8}] {c['sk'].replace(NAME_MARK, '<NOM>')}")
            print(f"  {'':10} = {c['fr'].replace(NAME_MARK, '<NOM>')}   ({len(c['gloss'])} mots, aide : {'oui' if c['help'] else 'non'})")
            if c.get("choice"):
                kind, exid, items = c["choice"]
                print(f"  {'':10} choix {kind[3:].lower()} ({exid}) : " + " / ".join(items))
    print(f"-- répliques secrètes ({len(ep['bonus'])}) : " + " / ".join(b["sk"] for b in ep["bonus"]))
    print(f"-- mots ({len(ep['words'])}) : " + ", ".join(f"{a} = {b}" for a, b in ep["words"]))
    print(f"-- exercices ({len(ep['exercises'])}) : " + ", ".join(f"{e['type'][3:].lower()} {e['id']}" for e in ep["exercises"]))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("episodes", nargs="*", help="fiches a convertir (nom sans .md) ; defaut : EPISODES")
    ap.add_argument("--course", default=DEFAULT_COURSE, help=f"dossier du cours (defaut : {DEFAULT_COURSE})")
    ap.add_argument("--dump", action="store_true", help="afficher ce qui est lu, sans ecrire")
    args = ap.parse_args()
    if not os.path.isdir(os.path.join(args.course, "md")):
        sys.exit(f"cours introuvable dans {args.course} : lancer « python3 src/langs.py sk-fr-friends » "
                 "a la racine du depot, ou passer --course")
    stems = args.episodes or EPISODES
    for s in stems:
        if not glob.glob(os.path.join(args.course, "md", s + ".md")):
            sys.exit(f"fiche introuvable : md/{s}.md")
    episodes = [parse_episode(args.course, s) for s in stems]
    bwarn = Warn()
    bonus = load_bonus(bwarn)
    bwarn.report("story/bonus.txt")
    for ep in episodes:
        ep["bonus"] = bonus.get(ep["id"], [])
    lang = json.load(open(os.path.join(args.course, "lang.json"), encoding="utf-8"))
    default_name = lang.get("site", {}).get("user_name_default", "Gab")
    if args.dump:
        for ep in episodes:
            dump(ep)
        print(f"\nnom par défaut du joueur : {default_name}")
        return
    lwarn = Warn()
    scripts = load_lines(lwarn)
    lwarn.report("story/lines.txt")
    write(episodes, default_name, scripts)


if __name__ == "__main__":
    main()
