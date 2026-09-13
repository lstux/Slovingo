#!/usr/bin/env python3

"""
smd2exercises.py
=================

Génère des exercices interactifs (QCM, phrases à trous, écoute) au format
JSON à partir de fiches SMD (translate-tables et audio-cards).

Approche générique : le script lit TOUTES les fiches passées en argument
en une seule passe et construit un corpus global (vocabulaire + phrases +
fréquence des mots). Chaque fiche individuelle pioche ses distracteurs et
identifie ses "trous" grammaticaux à partir de ce corpus commun plutôt que
de listes codées en dur : plus on ajoute de fiches, plus le résultat est
pertinent, sans aucune maintenance de liste de mots.

=> Toujours lancer le script sur l'ensemble des fiches d'un coup
   (comme le fait déjà publish-slovak.sh pour le HTML) :

    python3 smd2exercises.py md/*.md --index

Ne modifie pas les fichiers .md. Ne touche pas au HTML généré par
smd2html.py : flux totalement séparé, en sortie il produit :

    NomFiche.exercises.json   (un par fiche traitée)
    exercises-index.json      (récapitulatif, avec --index)
"""

import argparse
import json
import random
import re
import sys
from collections import Counter
from pathlib import Path

from langconfig import load_config, is_header_match

# ============================================================================
# Détection des tables (config partagée avec smd2html.py — voir lang.json)
# ============================================================================

CONFIG = load_config()


def split_table_row(line):
    line = line.strip()
    if line.startswith("|"):
        line = line[1:]
    if line.endswith("|"):
        line = line[:-1]
    return [cell.strip() for cell in line.split("|")]


def is_table_separator(line):
    cells = split_table_row(line)
    if not cells:
        return False
    return all(re.match(r'^:?-{3,}:?$', cell.strip()) for cell in cells)


def is_target_header(text):
    """Colonne "langue cible" (slovaque par défaut, selon lang.json)."""
    return is_header_match(text, CONFIG["target_lang"])


def clean_cell(text):
    """Retire la syntaxe SMD/Markdown résiduelle d'une cellule ou d'un texte."""
    text = re.sub(r'\{\{(.*?)\}\}', r'\1', text)
    text = re.sub(r'\*\*(.+?)\*\*', r'\1', text)
    text = re.sub(r'\*(.+?)\*', r'\1', text)
    text = re.sub(r'`([^`]+)`', r'\1', text)
    return text.strip()


def strip_punct(word):
    return word.strip(".,!?;:\"'()„“”«»…")


def split_speaker(text):
    """
    Les fiches dialogue préfixent chaque réplique par l'émoticône du
    personnage (ex: "☕ Dobrý deň..."). Ce n'est pas du slovaque à
    prononcer ni un mot à retrouver : on le sépare du texte utile.
    """
    tokens = text.split()
    if tokens and not any(ch.isalpha() for ch in tokens[0]):
        return tokens[0], " ".join(tokens[1:]).strip()
    return None, text


# ============================================================================
# Extraction du contenu d'une fiche SMD
# ============================================================================

def extract_content(md_text):
    """
    Parcourt le texte SMD et en extrait :
    - vocab_pairs : liste de tuples (slovaque, traduction) issus des
      translate-tables
    - sentences : liste de {"text", "translation", "speaker"} issus des
      audio-cards
    """

    lines = md_text.splitlines()
    vocab_pairs = []
    sentences = []
    index = 0

    while index < len(lines):
        stripped = lines[index].strip()

        if not stripped:
            index += 1
            continue

        # Translate-table
        if "|" in stripped and index + 1 < len(lines) and is_table_separator(lines[index + 1]):
            table_lines = [lines[index], lines[index + 1]]
            index += 2
            while index < len(lines) and "|" in lines[index] and lines[index].strip():
                table_lines.append(lines[index])
                index += 1
            vocab_pairs.extend(extract_vocab_from_table(table_lines))
            continue

        # Audio-card
        if stripped.startswith("!"):
            raw_text = clean_cell(stripped[1:].strip())
            speaker, audio_text = split_speaker(raw_text)
            next_index = index + 1
            translations = []
            while next_index < len(lines) and lines[next_index].strip().startswith(">"):
                translations.append(clean_cell(lines[next_index].strip()[1:].strip()))
                next_index += 1
            while next_index < len(lines) and lines[next_index].strip().startswith("+"):
                next_index += 1
            if audio_text:
                sentences.append({
                    "text": audio_text,
                    "translation": translations[0] if translations else "",
                    "speaker": speaker,
                })
            index = next_index
            continue

        index += 1

    return vocab_pairs, sentences


def extract_vocab_from_table(lines):
    if len(lines) < 3:
        return []

    headers = split_table_row(lines[0])
    target_col = -1
    for i, header in enumerate(headers):
        if is_target_header(header):
            target_col = i
            break
    if target_col < 0:
        return []

    other_col = next((i for i in range(len(headers)) if i != target_col), -1)
    if other_col < 0:
        return []

    pairs = []
    for line in lines[2:]:
        cells = split_table_row(line)
        if len(cells) <= max(target_col, other_col):
            continue
        sk = clean_cell(cells[target_col])
        fr = clean_cell(cells[other_col])
        if sk and fr:
            pairs.append((sk, fr))
    return pairs


def dedupe_pairs(vocab_pairs):
    seen = set()
    unique = []
    for sk, fr in vocab_pairs:
        key = sk.lower()
        if key in seen:
            continue
        seen.add(key)
        unique.append((sk, fr))
    return unique


def dedupe_keep_order(items):
    seen = set()
    unique = []
    for item in items:
        if item in seen:
            continue
        seen.add(item)
        unique.append(item)
    return unique


# ============================================================================
# Construction du corpus global (toutes les fiches passées en argument)
# ============================================================================

def build_corpus(paths):
    """
    Lit chaque fichier une seule fois.

    Retourne :
    - per_file : {path: (vocab_pairs, sentences)}
    - global_pairs : vocab_pairs de toutes les fiches combinées
    - global_sentences : sentences de toutes les fiches combinées
    """
    per_file = {}
    global_pairs = []
    global_sentences = []

    for path in paths:
        md_text = path.read_text(encoding="utf-8")
        vocab_pairs, sentences = extract_content(md_text)
        per_file[path] = (vocab_pairs, sentences)
        global_pairs.extend(vocab_pairs)
        global_sentences.extend(sentences)

    return per_file, global_pairs, global_sentences


def build_word_frequency(sentences):
    """
    Fréquence de chaque mot à travers tout le corpus de phrases.

    Un mot qui revient dans beaucoup de phrases différentes (pronoms,
    prépositions, verbes courants...) est un bon candidat de "trou"
    grammatical — sans avoir besoin d'une liste écrite à la main.
    """
    freq = Counter()
    for s in sentences:
        for token in s["text"].split():
            word = strip_punct(token).lower()
            if word:
                freq[word] += 1
    return freq


# ============================================================================
# Génération : QCM de traduction
# ============================================================================

def generate_qcm(local_pairs, global_pairs, n_choices=4):
    """
    Génère le QCM dans les deux sens — "l2-l1" (langue cible -> langue
    native) et "l1-l2" (langue native -> langue cible) : le choix du
    sens à pratiquer se fait côté JavaScript, pas à la génération.

    Les questions portent sur le vocabulaire de LA FICHE (local_pairs).
    Les distracteurs sont piochés dans TOUT LE CORPUS (global_pairs), ce
    qui permet de générer un QCM même pour une petite table, dès que le
    reste du corpus fournit assez d'alternatives.
    """
    exercises = []
    local_unique = dedupe_pairs(local_pairs)
    global_unique = dedupe_pairs(global_pairs)
    if not local_unique:
        return exercises

    for direction in ("l2-l1", "l1-l2"):
        for idx, (sk, fr) in enumerate(local_unique):
            question = sk if direction == "l2-l1" else fr
            answer = fr if direction == "l2-l1" else sk

            other_answers = dedupe_keep_order([
                (f if direction == "l2-l1" else s)
                for (s, f) in global_unique
                if (f if direction == "l2-l1" else s) != answer
            ])
            if len(other_answers) < n_choices - 1:
                continue  # pas encore assez de corpus pour distinguer cette réponse

            distractors = random.sample(other_answers, n_choices - 1)
            choices = distractors + [answer]
            random.shuffle(choices)

            exercises.append({
                "id": f"qcm-{direction}-{idx}",
                "type": "qcm",
                "direction": direction,
                "question": question,
                "answer": answer,
                "choices": choices,
                "audio": sk,
            })
    return exercises


# ============================================================================
# Génération : phrases à trous
# ============================================================================

def choose_blank_index(tokens, freq):
    """
    Choisit l'indice du mot à transformer en trou :
    le mot le plus fréquent dans le corpus global parmi les candidats
    (on exclut le premier mot, souvent en début de phrase et capitalisé).

    Si aucun mot ne se distingue (fréquence <= 1 partout, cas d'un
    corpus encore petit), on se rabat sur le mot le plus court parmi
    ceux du milieu de la phrase.
    """
    candidates = list(range(1, len(tokens))) or [0]
    candidate_idx = max(candidates, key=lambda i: freq.get(strip_punct(tokens[i]).lower(), 0))
    best_freq = freq.get(strip_punct(tokens[candidate_idx]).lower(), 0)

    if best_freq <= 1:
        middle = list(range(1, len(tokens) - 1)) or candidates
        candidate_idx = min(middle, key=lambda i: len(strip_punct(tokens[i])))

    return candidate_idx


def generate_fill_blank(local_sentences, freq, n_choices=3):
    exercises = []

    # Les mots les plus fréquents du corpus servent de vivier de
    # distracteurs plausibles (des mots "grammaticaux" par construction,
    # puisqu'ils reviennent dans des phrases très différentes).
    frequent_words = [w for w, _ in freq.most_common(40)]

    for idx, s in enumerate(local_sentences):
        tokens = s["text"].split()
        if len(tokens) < 3:
            continue

        candidate_idx = choose_blank_index(tokens, freq)
        removed = strip_punct(tokens[candidate_idx])
        if not removed:
            continue

        display_tokens = tokens.copy()
        display_tokens[candidate_idx] = "___"

        pool = [w for w in frequent_words if w != removed.lower()]
        if len(pool) < n_choices:
            # corpus encore petit : on complète avec n'importe quel autre
            # mot rencontré, mieux que de bloquer la génération
            pool = dedupe_keep_order(pool + [w for w in freq if w != removed.lower()])
        if len(pool) < n_choices:
            continue

        distractors = random.sample(pool, n_choices)
        choices = distractors + [removed]
        random.shuffle(choices)

        exercise = {
            "id": f"blank-{idx}",
            "type": "fill-blank",
            "tokens": display_tokens,
            "blank_index": candidate_idx,
            "answer": removed,
            "choices": choices,
            "audio": s["text"],
            "translation": s["translation"],
        }
        if s.get("speaker"):
            exercise["speaker"] = s["speaker"]
        exercises.append(exercise)
    return exercises


# ============================================================================
# Génération : écoute et devine
# ============================================================================

def utterances_from(vocab_pairs, sentences):
    utterances = [{"audio": sk, "translation": fr} for sk, fr in dedupe_pairs(vocab_pairs)]
    utterances += [
        {"audio": s["text"], "translation": s["translation"], "speaker": s.get("speaker")}
        for s in sentences
    ]
    return utterances


def generate_listen(local_utterances, global_utterances, n_choices=4):
    exercises = []
    global_texts = dedupe_keep_order([u["audio"] for u in global_utterances])

    for idx, u in enumerate(local_utterances):
        others = [t for t in global_texts if t != u["audio"]]
        if len(others) < n_choices - 1:
            continue

        distractors = random.sample(others, n_choices - 1)
        choices = distractors + [u["audio"]]
        random.shuffle(choices)

        exercise = {
            "id": f"listen-{idx}",
            "type": "listen",
            "audio": u["audio"],
            "answer": u["audio"],
            "choices": choices,
            "translation": u["translation"],
        }
        if u.get("speaker"):
            exercise["speaker"] = u["speaker"]
        exercises.append(exercise)
    return exercises


# ============================================================================
# Assemblage par fiche
# ============================================================================

def build_exercises_for_file(local_pairs, local_sentences, global_pairs, global_sentences,
                              freq, types, n_choices):
    exercises = []
    if "qcm" in types:
        exercises += generate_qcm(local_pairs, global_pairs, n_choices=n_choices)
    if "fill-blank" in types:
        exercises += generate_fill_blank(local_sentences, freq, n_choices=n_choices - 1)
    if "listen" in types:
        local_utterances = utterances_from(local_pairs, local_sentences)
        global_utterances = utterances_from(global_pairs, global_sentences)
        exercises += generate_listen(local_utterances, global_utterances, n_choices=n_choices)
    return exercises


def format_title(text):
    text = re.sub(r'[_-]', ' ', text)
    return (text[:1].upper() + text[1:]) if text else text


def guess_category(stem):
    prefix = stem.split("_")[0]
    known = {
        "Revisions": "Révisions", "Dialogues": "Dialogues", "Dialogue": "Dialogues",
        "Grammaire": "Grammaire", "Introduction": "Introduction",
        "Vocabulaire": "Vocabulaire", "Situations": "Situations",
        "Serie": "Séries",
    }
    return known.get(prefix, "Autres")


def guess_theme(stem):
    parts = stem.split("_")
    if parts[0] == "Revisions" and len(parts) >= 4:
        return parts[1]
    # Serie_01_Rodina_02_kolko-mas-rokov -> thème "Rodina"
    if parts[0] == "Serie" and len(parts) >= 4:
        return parts[2]
    return None


def guess_title(stem):
    parts = stem.split("_")
    if parts[0] == "Revisions" and len(parts) >= 4:
        return format_title(parts[3])
    if parts[0] == "Serie" and len(parts) >= 5:
        return format_title("_".join(parts[4:]))
    if len(parts) >= 2 and parts[0] in ("Dialogues", "Dialogue", "Grammaire",
                                         "Introduction", "Vocabulaire", "Situations"):
        return format_title("_".join(parts[1:]))
    return format_title(stem)


# ============================================================================
# Programme principal
# ============================================================================

def parse_parcours(path):
    """
    Lit un fichier de parcours (même format que celui utilisé par
    publish-slovak.sh pour l'index principal — voir src/parcours.txt)
    et retourne la liste ORDONNÉE des unités pédagogiques :

        [{"label": "👨‍👩‍👧‍👦 1 · Rodina — la famille",
          "files": ["Serie_01_Rodina_01_moja-rodina", ...]}, ...]

    Une ligne "= Titre" ouvre une nouvelle unité ; toute autre ligne
    non vide et non commentée ("#") ajoute une fiche (son nom sans
    ".md") à l'unité en cours. Les fiches listées avant la première
    unité sont ignorées (le format attend un "= Titre" en tête).

    Ainsi l'écran de sélection des exercices peut se regrouper
    exactement comme la page d'accueil, sans dupliquer le parcours :
    un seul fichier fait foi pour les deux.
    """
    groups = []
    current = None
    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        if line.startswith("="):
            current = {"label": line[1:].strip(), "files": []}
            groups.append(current)
            continue
        if current is None:
            continue
        stem = line[:-3] if line.endswith(".md") else line
        current["files"].append(stem)
    return groups


def parse_types(raw):
    if raw is None or raw.strip().lower() == "all":
        return {"qcm", "fill-blank", "listen"}
    valid = {"qcm", "fill-blank", "listen"}
    chosen = {t.strip() for t in raw.split(",") if t.strip()}
    unknown = chosen - valid
    if unknown:
        sys.exit(f"Erreur : type(s) inconnu(s) : {', '.join(sorted(unknown))} "
                  f"(valides : qcm, fill-blank, listen)")
    return chosen


def update_index(index_path, entries, groups=None):
    """
    Met à jour exercises-index.json.

    Format : {"entries": [...], "groups": [...]}.

    - "entries" se fusionne comme avant (par "source"), pour permettre
      des appels successifs sur des sous-ensembles de fiches (public
      puis privé, chacun avec son propre --out-dir).
    - "groups" (les unités du parcours) est en revanche remplacé en
      bloc quand --parcours est fourni : il n'y a qu'un seul parcours
      par site, pas de fusion incrémentale à faire. Si --parcours
      n'est pas fourni sur cet appel, les groupes déjà présents dans
      le fichier (générés par un appel précédent) sont conservés.

    Migration transparente depuis l'ancien format (liste plate) :
    un fichier existant de ce type est relu comme des "entries" sans
    "groups".
    """
    existing = {"entries": [], "groups": []}
    if index_path.exists():
        try:
            data = json.loads(index_path.read_text(encoding="utf-8"))
            if isinstance(data, list):
                existing = {"entries": data, "groups": []}
            elif isinstance(data, dict):
                existing = {
                    "entries": data.get("entries", []),
                    "groups": data.get("groups", []),
                }
        except (json.JSONDecodeError, OSError):
            pass

    by_source = {entry["source"]: entry for entry in existing["entries"]}
    for entry in entries:
        by_source[entry["source"]] = entry

    merged_entries = sorted(by_source.values(), key=lambda e: (e["category"], e.get("theme") or "", e["title"]))
    payload = {
        "entries": merged_entries,
        "groups": groups if groups is not None else existing["groups"],
    }
    index_path.write_text(
        json.dumps(payload, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    return len(merged_entries)


def main():
    parser = argparse.ArgumentParser(
        description="Génère des exercices JSON (QCM, phrases à trous, écoute) depuis des fiches SMD. "
                    "À lancer sur toutes les fiches d'un coup pour un pool de distracteurs riche."
    )
    parser.add_argument("files", nargs="+", help="Fichier(s) .md à traiter")
    parser.add_argument(
        "--types", default="all",
        help="Types à générer, séparés par des virgules : qcm,fill-blank,listen (défaut : all)"
    )
    parser.add_argument(
        "--n-choices", type=int, default=4,
        help="Nombre de propositions par question (défaut : 4)"
    )
    parser.add_argument(
        "--out-dir", type=Path, default=None,
        help="Dossier de sortie des .exercises.json (défaut : à côté du .md)"
    )
    parser.add_argument(
        "--index", action="store_true",
        help="Met à jour exercises-index.json"
    )
    parser.add_argument(
        "--index-file", type=Path, default=None,
        help="Chemin de l'index (défaut : exercises-index.json dans --out-dir ou à côté du 1er .md)"
    )
    parser.add_argument(
        "--parcours", type=Path, default=None,
        help="Fichier de parcours (ex. src/parcours.txt) pour organiser l'écran de "
             "sélection par unité pédagogique, comme la page d'accueil"
    )
    parser.add_argument(
        "--seed", default=None,
        help="Graine aléatoire (pour des sorties reproductibles, utile en test)"
    )

    args = parser.parse_args()
    types = parse_types(args.types)

    paths = []
    for raw_path in args.files:
        path = Path(raw_path)
        if not path.exists():
            print(f"⚠ fichier introuvable, ignoré : {path}", file=sys.stderr)
            continue
        paths.append(path)

    if not paths:
        sys.exit("Aucun fichier valide à traiter.")

    if args.seed is not None:
        random.seed(args.seed)

    per_file, global_pairs, global_sentences = build_corpus(paths)
    freq = build_word_frequency(global_sentences)

    print(f"Corpus : {len(paths)} fiche(s), {len(dedupe_pairs(global_pairs))} mots de vocabulaire uniques, "
          f"{len(global_sentences)} phrases.")

    entries = []
    for path in paths:
        local_pairs, local_sentences = per_file[path]
        exercises = build_exercises_for_file(
            local_pairs, local_sentences, global_pairs, global_sentences,
            freq, types, args.n_choices
        )

        stem = path.stem
        title = guess_title(stem)
        payload = {"source": path.name, "title": title, "exercises": exercises}

        output_dir = args.out_dir if args.out_dir else path.parent
        output_file = output_dir / f"{stem}.exercises.json"
        output_file.write_text(
            json.dumps(payload, ensure_ascii=False, indent=2),
            encoding="utf-8",
        )

        counts = {}
        for ex in exercises:
            counts[ex["type"]] = counts.get(ex["type"], 0) + 1

        entries.append({
            "file": output_file.name,
            "source": path.name,
            "title": title,
            "category": guess_category(stem),
            "theme": guess_theme(stem),
            "counts": counts,
        })

        total = sum(counts.values())
        detail = ", ".join(f"{k}: {v}" for k, v in counts.items()) or "aucun exercice généré"
        print(f"✓ {output_file.name}  ({total} exercices — {detail})")

    if args.index:
        out_dir = args.out_dir if args.out_dir else paths[0].parent
        index_path = args.index_file if args.index_file else out_dir / "exercises-index.json"
        groups = None
        if args.parcours:
            if args.parcours.exists():
                groups = parse_parcours(args.parcours)
            else:
                print(f"⚠ fichier de parcours introuvable, ignoré : {args.parcours}", file=sys.stderr)
        total_fiches = update_index(index_path, entries, groups)
        print(f"✓ index mis à jour : {index_path} ({total_fiches} fiches référencées)")


if __name__ == "__main__":
    main()
