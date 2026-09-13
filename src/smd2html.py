#!/usr/bin/env python3

"""
md2html-v2.py
=============

Convertisseur Markdown simplifié spécialisé pour les mémos de langue.

Syntaxe spéciale
----------------

1. Tableau de traduction

    | Français | Slovaque |
    |----------|----------|
    | Bonjour  | Dobrý deň |

Si un en-tête contient un mot indiquant la langue slovaque,
la table reçoit automatiquement :

    class="translate-table"


2. Carte audio

    ! Dobrý deň!
    > Bonjour !
    > Littéralement : "bon jour".
    + Remarque éventuelle

Devient :

    <div class="audio-card">
        <div class="audio-text">
            Dobrý deň!
        </div>
        <div class="audio-translation">
            <p>Bonjour !</p>
            <p>Littéralement : "bon jour".</p>
            <p class="comments">Remarque éventuelle</p>
        </div>
    </div>


3. Élément isolé prononçable

    {{jeden}}

Devient :

    <span class="speakable">jeden</span>


4. Illustration de fiche

    ![Vue des Tatras](img/tatry.jpg "Ideme do hôr.")

La légende (entre guillemets) est optionnelle. Devient :

    <figure class="fiche-illustration">
        <img src="img/tatry.jpg" alt="Vue des Tatras" loading="lazy">
        <figcaption>Ideme do hôr.</figcaption>
    </figure>

Si le fichier référencé n'existe pas (ex : image pas encore
téléchargée dans html/img/), le thème de la fiche prend le relais en
arrière-plan de la figure — rien ne reste vide.


Thème visuel
------------

Chaque fiche reçoit un attribut data-theme déduit de son nom de
fichier (voir detect_theme()) : une fiche de série
(Serie_08_Tatry_01_do-hor.md) suit le thème de son unité ("tatry"),
toute autre fiche retombe sur le thème neutre "uvod". C'est ce que
consomme style.css pour changer palette, police et motif de fond
d'une fiche à l'autre.


Fichiers externes
-----------------

Le HTML généré référence :

    style.css
    slovak.js

Ces fichiers doivent se trouver dans le même dossier que le HTML.
Les polices (Google Fonts) sont chargées par lien direct dans le
<head>, avec repli sur les polices système en cas d'absence de
réseau.


Usage
-----

    python3 md2html-v2.py memo.md

Produit :

    memo.html
"""


import sys
import re
import html
import urllib.parse
from pathlib import Path

from langconfig import load_config, is_header_match, fiche_footer


# ============================================================================
# Configuration
# ============================================================================

# Toutes les valeurs dépendant de la langue (mots-clés de colonnes,
# libellés de catégories/séries, footer...) viennent de lang.json —
# voir langconfig.py. Rien de linguistique n'est plus codé en dur ici.
CONFIG = load_config()

# Syntaxe d'illustration de fiche : @ chemin | légende
IMAGE_RE = re.compile(r'^@ (.+?)\s*\|\s*(.+?)$')
# Syntaxe d'illustration de fiche dépréciée : ![alt](chemin "légende optionnelle")
IMAGE_RE_DEPRECATED = re.compile(r'^!\[(.*?)\]\((\S+?)(?:\s+"(.*?)")?\)$')

# Une fiche de série (Serie_08_Tatry_01_do-hor) suit le thème de son
# unité. Le nom de l'unité (2e groupe, ex. "Tatry") est mis en
# minuscules pour former le slug de thème ("tatry"), qui doit
# correspondre à un sélecteur [data-theme="..."] dans style.css.
SERIE_THEME_RE = re.compile(r'^Serie_\d+_([A-Za-z]+)_(\d+)_')

# Intitulé court de chaque unité de série, pour le bandeau (kicker).
# Vient de lang.json ("series") — doit rester aligné avec THEME_LABELS
# dans exercises.js (même source à terme, voir lang-config.js).
THEME_UNIT_LABELS = CONFIG["series"]

# Intitulé du kicker pour les fiches hors série, par préfixe de nom
# de fichier (aligné avec card_emoji() dans publish-slovak.sh).
CATEGORY_LABELS = CONFIG["categories"]

# Polices web (Google Fonts), avec repli sur les polices système déjà
# défini dans style.css : si le réseau manque au chargement, la page
# reste lisible avec les fallbacks (voir --font-* dans style.css).
FONT_LINKS = """  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&family=Fraunces:opsz,wght@9..144,500;9..144,700&family=Space+Grotesk:wght@500;700&family=JetBrains+Mono:wght@500;700&display=swap" rel="stylesheet">"""


def detect_theme(stem):
    """
    Déduit le thème visuel d'une fiche à partir de son nom de fichier.

    Une fiche de série (Serie_08_Tatry_01_do-hor) suit le thème de
    son unité (ici "tatry"). Toute autre fiche (Introduction,
    Vocabulaire, Grammaire, Dialogues, Revisions, Adulte, Endy...)
    retombe sur le thème neutre "uvod".
    """

    match = SERIE_THEME_RE.match(stem)
    if match:
        return match.group(1).lower()
    return "uvod"


def build_kicker(stem):
    """
    Petite ligne affichée au-dessus du titre, dans le bandeau :
    l'unité et le numéro de la fiche pour une série ("8 · Tatry ·
    fiche 01"), sinon la catégorie ("Grammaire", "Dialogues"...).
    Chaîne vide si le préfixe n'est pas reconnu.
    """

    match = SERIE_THEME_RE.match(stem)
    if match:
        theme_key = match.group(1).lower()
        fiche_num = match.group(2)
        label = THEME_UNIT_LABELS.get(theme_key, match.group(1))
        return f"{label} · fiche {fiche_num}"

    prefix = stem.split("_")[0]
    return CATEGORY_LABELS.get(prefix, "")


# ============================================================================
# Utilitaires
# ============================================================================

def escape_html(text):
    """
    Échappe le texte pour une insertion HTML sûre.
    """

    return html.escape(
        text,
        quote=True
    )


def process_speakable(text):
    """
    Transforme :

        {{jeden}}

    en :

        <span class="speakable">jeden</span>

    Le contenu interne est d'abord échappé.
    """

    pattern = r'\{\{(.*?)\}\}'
    def replace(match):
        content = match.group(1).strip()
        if not content:
            return ""
        return (
            '<span class="speakable speakable-hint">'
            f'{escape_html(content)}'
            '</span>'
        )

    return re.sub(
        pattern,
        replace,
        text
    )


# ============================================================================
# Markdown inline
# ============================================================================

def markdown_inline(text):
    """
    Conversion Markdown inline minimale.

    Supporte :

        **gras**
        *italique*
        `code`
        [lien](https://...)
        {{texte prononçable}}
    """

    # On protège temporairement les éléments {{...}}
    speakable_elements = []
    def store_speakable(match):
        content = match.group(1).strip()
        placeholder = f"___SPEAKABLE_{len(speakable_elements)}___"
        speakable_elements.append(content)
        return placeholder
    text = re.sub(
            r'\{\{(.*?)\}\}',
            store_speakable,
            text
        )

    # Échappement HTML et process des éléments markdown
    text = escape_html(text)
    # Liens Markdown
    text = re.sub(
            r'\[([^\]]+)\]\((https?://[^)]+)\)',
            r'<a href="\2" target="_blank">\1</a>',
            text
        )
    # Code inline
    text = re.sub(
            r'`([^`]+)`',
            r'<code>\1</code>',
            text
        )
    # Gras
    text = re.sub(
            r'\*\*(.+?)\*\*',
            r'<strong>\1</strong>',
            text
        )
    # Italique
    text = re.sub(
            r'\*(.+?)\*',
            r'<em>\1</em>',
            text
        )

    # Restauration des éléments speakable
    for index, content in enumerate(speakable_elements):
        placeholder = f"___SPEAKABLE_{index}___"
        replacement = (
            '<span class="speakable speakable-hint">'
            f'{escape_html(content)}'
            '</span>'
        )
        text = text.replace(placeholder, replacement)

    return text


# ============================================================================
# Détection des tableaux
# ============================================================================

def is_table_separator(line):
    """
    Vérifie si une ligne est une ligne de séparation Markdown.

    Exemple :
        |----------|----------|
    ou :
        | :------- | --------: |
    """

    cells = split_table_row(line)
    if not cells:
        return False

    for cell in cells:
        cell = cell.strip()
        if not re.match(
            r'^:?-{3,}:?$',
            cell
        ):
            return False

    return True


def split_table_row(line):
    """
    Découpe une ligne de tableau Markdown.

    Exemple :
        | Français | Slovaque |
    retourne :
        ["Français", "Slovaque"]
    """

    line = line.strip()

    # Retire les pipes extérieurs
    if line.startswith("|"):
        line = line[1:]
    if line.endswith("|"):
        line = line[:-1]

    # Découpe sur les pipes
    cells = line.split("|")

    return [
        cell.strip()
        for cell in cells
    ]


def is_target_header(text):
    """
    Vérifie si un header correspond à la colonne "langue cible"
    (slovaque, breton, anglais... selon lang.json).
    """

    return is_header_match(text, CONFIG["target_lang"])


def is_native_header(text):
    """
    Vérifie si un header correspond à la colonne "langue maternelle"
    (français par défaut, selon lang.json).
    """

    return is_header_match(text, CONFIG["native_lang"])


def header_flag(text):
    """
    Retourne l'emoji drapeau correspondant à un header (langue cible
    ou langue maternelle), ou une chaîne vide si aucun des deux ne
    correspond.
    """

    if is_target_header(text):
        return CONFIG["target_lang"]["flag"] + " "
    if is_native_header(text):
        return CONFIG["native_lang"]["flag"] + " "

    return ""


def convert_table(lines):
    """
    Convertit une liste de lignes Markdown représentant
    un tableau en HTML.

    Détecte automatiquement la colonne slovaque.
    """

    if len(lines) < 2:
        return ""

    headers = split_table_row(lines[0])

    # Ligne de séparation
    separator = split_table_row(lines[1])

    # Détection des colonnes langue cible et langue maternelle

    target_column = -1
    native_column = -1
    for index, header in enumerate(headers):
        if target_column < 0 and is_target_header(header):
            target_column = index
            continue
        if native_column < 0 and is_native_header(header):
            native_column = index

    # Classe de la table
    table_class = "translate-table"
    if target_column < 0:
        table_class = ""
    class_attribute = ""

    if table_class:
        class_attribute = f' class="{table_class}"'

    target_css_class = CONFIG["target_lang"]["css_class"]
    native_css_class = CONFIG["native_lang"]["css_class"]

    output = []
    output.append(
        f'<table{class_attribute}>'
    )

    # ------------------------------------------------------------------------
    # Header
    # ------------------------------------------------------------------------

    output.append("<thead>")
    output.append("<tr>")
    for index, header in enumerate(headers):
        th_class = ""
        if index == target_column:
            th_class = f' class="{target_css_class}"'
        elif index == native_column:
            th_class = f' class="{native_css_class}"'
        output.append(
            f"<th{th_class}>"
            f"{header_flag(header)}{markdown_inline(header)}"
            "</th>"
        )
    output.append("</tr>")
    output.append("</thead>")

    # ------------------------------------------------------------------------
    # Corps
    # ------------------------------------------------------------------------

    output.append("<tbody>")
    for line in lines[2:]:
        cells = split_table_row(line)
        # Ignore les lignes vides
        if not cells:
            continue
        output.append("<tr>")
        for index, cell in enumerate(cells):
            td_class = ""
            if index == target_column:
                td_class = f' class="{target_css_class}"'
            elif index == native_column:
                td_class = f' class="{native_css_class}"'
            output.append(
                f"<td{td_class}>"
                f"{markdown_inline(cell)}"
                "</td>"
            )
        output.append("</tr>")
    output.append("</tbody>")

    output.append("</table>")

    return "\n".join(output)


# ============================================================================
# Conversion Markdown
# ============================================================================

def convert_markdown(md_text):
    lines = md_text.splitlines()
    output = []
    index = 0

    # Fonction pour fermer les listes
    in_ul = False
    in_ol = False
    def close_lists():
        nonlocal in_ul
        nonlocal in_ol
        if in_ul:
            output.append("</ul>")
            in_ul = False
        if in_ol:
            output.append("</ol>")
            in_ol = False

    # Parcours principal
    while index < len(lines):
        line = lines[index]
        stripped = line.strip()

        # Ligne vide
        if not stripped:
            close_lists()
            index += 1
            continue

        # Tableau Markdown
        if ("|" in stripped and index + 1 < len(lines) and is_table_separator(lines[index + 1])):
            close_lists()
            table_lines = [ lines[index], lines[index + 1] ]
            index += 2
            # Récupération des lignes du tableau
            while (index < len(lines) and "|" in lines[index] and lines[index].strip()):
                table_lines.append(lines[index])
                index += 1
            output.append(convert_table(table_lines))
            continue

        # Illustration de fiche : ![alt](chemin "légende optionnelle")
        # Vérifiée avant la carte audio, qui commence aussi par "!".
        img_match = IMAGE_RE_DEPRECATED.match(stripped)
        if img_match:
            close_lists()
            alt, src, caption = img_match.groups()
            output.append('<figure class="fiche-illustration">')
            output.append(
                f'    <img src="{escape_html(src)}" alt="{escape_html(alt)}" loading="lazy">'
            )
            if caption:
                output.append(
                    '    <figcaption>'
                    f'{markdown_inline(caption)}'
                    '</figcaption>'
                )
            output.append('</figure>')
            index += 1
            continue
        img_match = IMAGE_RE.match(stripped)
        if img_match:
            close_lists()
            src, caption = img_match.groups()
            output.append('<figure class="fiche-illustration">')
            output.append(
                f'    <img src="{escape_html(src)}" alt="{escape_html(src)}" loading="lazy">'
            )
            if caption:
                output.append(
                    '    <figcaption>'
                    f'{markdown_inline(caption)}'
                    '</figcaption>'
                )
            output.append('</figure>')
            index += 1
            continue 

        # Carte audio
        if stripped.startswith("!"):
            close_lists()
            audio_text = stripped[1:].strip()
            output.append('<div class="audio-card">')
            # Texte slovaque
            output.append(
                '    <div class="audio-text">'
                f'{markdown_inline(audio_text)}'
                '</div>'
            )
            # Recherche des traductions (>) et remarques (+)
            translations = []
            next_index = index + 1
            while ( next_index < len(lines) and lines[next_index].strip().startswith(">")):
                translation = lines[next_index].strip()[1:].strip()
                translations.append(translation)
                next_index += 1
            comments = []
            # Recherche des remarques (lignes commençant par +)
            while ( next_index < len(lines) and lines[next_index].strip().startswith("+")):
                comment = lines[next_index].strip()[1:].strip()
                comments.append(comment)
                next_index += 1
            # Création du bloc de traduction (traductions puis remarques)
            if translations or comments:
                output.append('    <div class="audio-translation">')
                for translation in translations:
                    output.append(
                        '        <p>'
                        f'{markdown_inline(translation)}'
                        '</p>'
                    )
                for comment in comments:
                    output.append(
                        '        <p class="comments">'
                        f'{markdown_inline(comment)}'
                        '</p>'
                    )
                output.append('    </div>')
            output.append('</div>')
            # On saute les lignes > et + déjà consommées
            index = next_index
            continue

        # Traduction / commentaire hors carte audio
        if stripped.startswith(">"):
            close_lists()
            blockquote_lines = []
            while (index < len(lines) and lines[index].strip().startswith(">")):
                text = lines[index].strip()[1:].strip()
                blockquote_lines.append(text)
                index += 1
            output.append("<blockquote>")
            for text in blockquote_lines:
                output.append(
                    "<p>"
                    f"{markdown_inline(text)}"
                    "</p>"
                )
            output.append(
                "</blockquote>"
            )
            continue

        # Titres
        match = re.match(r'^(#{1,6})\s+(.+)$', stripped)
        if match:
            close_lists()
            level = len(match.group(1))
            title = markdown_inline(match.group(2))
            output.append(
                f"<h{level}>"
                f"{title}"
                f"</h{level}>"
            )
            index += 1
            continue

        # Liste non ordonnée
        # ("+" est exclu : il est réservé aux remarques des audio-cards)
        match = re.match(r'^[-*]\s+(.+)$', stripped)
        if match:
            if not in_ul:
                close_lists()
                output.append("<ul>")
                in_ul = True
            output.append(
                "<li>"
                f"{markdown_inline(match.group(1))}"
                "</li>"
            )
            index += 1
            continue

        # Liste ordonnée
        match = re.match(r'^\d+\.\s+(.+)$', stripped)
        if match:
            if not in_ol:
                close_lists()
                output.append("<ol>")
                in_ol = True
            output.append(
                "<li>"
                f"{markdown_inline(match.group(1))}"
                "</li>"
            )
            index += 1
            continue

        # Séparateur
        if re.match(r'^([-*_])(?:\s*\1){2,}\s*$', stripped):
            close_lists()
            output.append("<hr>")
            index += 1
            continue

        # Paragraphe normal
        close_lists()
        output.append(
            "<p>"
            f"{markdown_inline(stripped)}"
            "</p>"
        )
        index += 1

    # Fermeture finale des listes
    close_lists()

    return "\n".join(output)


# ============================================================================
# Template HTML
# ============================================================================

HTML_TEMPLATE = """<!DOCTYPE html>
<html lang="fr">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=yes">
  <title>__TITLE__</title>
__FONT_LINKS__
  <link rel="stylesheet" href="style.css">
</head>

<body data-theme="__THEME__">

<header class="band">
  <div class="band__photo"></div>
  <div class="band__tint"></div>
  <div class="band__pattern"></div>
  <p class="band__credit"></p>
  <div class="wrap">
__KICKER_HTML__
    <h1>__TITLE__</h1>
  </div>
</header>

<div class="wrap">

  <div class="toolbar">
    <button onclick="window.location.href='index.html'">🏠 Accueil</button>
    <button onclick="toggleAllTranslations()">👀 Traductions</button>
    <button onclick="window.location.href='exercises.html?card=__CARD__'">✏️ Exercices</button>
    <!--
    <button onclick="speakAll()">▶️ Tout lire</button>
    <button onclick="stopSpeaking()">⏹️ Stop</button>
    <input id="search" type="search" placeholder="🔎 Rechercher..." oninput="searchContent()">
    -->
  </div>

  <main id="content">
__CONTENT__
  </main>

</div>

<footer>
__FOOTER__
</footer>

<script src="lang-config.js"></script>
<script src="app.js"></script>

</body>

</html>
"""


# ============================================================================
# Génération du HTML
# ============================================================================

def format_title(stem):
    """
    Transforme le nom de fichier (sans extension) en titre lisible.

    - retire un préfixe de catégorie connu
      (Dialogues_ / Grammaire_ / Revisions_X_XX_) ;
    - remplace "_" et "-" par des espaces ;
    - met en majuscule la première lettre.

    Logique alignée avec card_title() dans publish-slovak.sh,
    pour que le titre d'une fiche corresponde à celui affiché
    sur la page d'accueil.
    """

    text = stem
    text = re.sub(r'^Introduction_\d+_', '', text)
    text = re.sub(r'^Conjugaison_\d+_', '', text)
    text = re.sub(r'^Dialogues_[^_]+_\d+_', '', text)
    text = re.sub(r'^Grammaire_\d+_', '', text)
    text = re.sub(r'^Revisions_[^_]+_\d+_', '', text)
    text = re.sub(r'^Serie_\d+_[^_]+_\d+_', '', text)
    text = re.sub(r'^Situations_\d+_', '', text)
    text = re.sub(r'^Vocabulaire_\d+_', '', text)
    text = re.sub(r'^Adulte_\d+_', '', text)
    text = re.sub(r'^Endy_', '', text)
    text = re.sub(r'[_-]', ' ', text)

    if text:
        text = text[0].upper() + text[1:]

    return text


def generate_html(input_file):
    # Lecture du Markdown
    md_text = input_file.read_text(encoding="utf-8")
    # Conversion
    content = convert_markdown(md_text)
    # Titre
    title = format_title(input_file.stem)
    # Nom de fiche (utilisé comme identifiant "card" pour exercises.html)
    card_id = urllib.parse.quote(input_file.stem)
    # Thème visuel et kicker (bandeau), déduits du nom de fichier
    theme = detect_theme(input_file.stem)
    kicker = build_kicker(input_file.stem)
    kicker_html = (
        f'    <p class="kicker">{escape_html(kicker)}</p>'
        if kicker else ""
    )
    # Génération du document
    document = HTML_TEMPLATE.replace(
            "__TITLE__",
            escape_html(title)
        ).replace(
            "__CONTENT__",
            content
        ).replace(
            "__CARD__",
            card_id
        ).replace(
            "__THEME__",
            theme
        ).replace(
            "__KICKER_HTML__",
            kicker_html
        ).replace(
            "__FONT_LINKS__",
            FONT_LINKS
        ).replace(
            "__FOOTER__",
            escape_html(fiche_footer(CONFIG))
        )
    # Fichier de sortie
    output_file = input_file.with_suffix(".html")
    # Écriture
    output_file.write_text(document, encoding="utf-8")

    return output_file


# ============================================================================
# Programme principal
# ============================================================================

def main():
    if len(sys.argv) != 2:
        print("Usage : python3 smd2html.py fichier.md")
        sys.exit(1)

    input_file = Path(sys.argv[1])

    if not input_file.exists():
        print(f"Erreur : fichier introuvable : {input_file}")
        sys.exit(1)

    output_file = generate_html(input_file)

    print(f"✓ HTML généré : {output_file}")


if __name__ == "__main__":
    main()
