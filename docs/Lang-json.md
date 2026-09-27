# Format de lang.json

Chaque cours Slovingo (un repo `Slovingo-<langue>`) est piloté par un unique
fichier `lang.json` à sa racine. C'est la seule chose qui change entre deux
instances du moteur : rien à toucher côté Python.

Ce document couvre l'intégralité de ses clés. Le format des fiches (SMD)
est documenté séparément dans [Format-SMD.txt](Format-SMD.txt), et la
structure de série adulte dans [Fiches-Serie.txt](Fiches-Serie.txt).

---

## target_lang / native_lang

Décrivent respectivement la langue apprise et la langue de l'apprenant.

```json
"target_lang": {
  "code": "sk",
  "name": "slovaque",
  "tts_code": "sk-SK",
  "flag": "🇸🇰",
  "css_class": "col-target",
  "voice_hint": "slovak",
  "tts_sample_phrase": "Vitajte na Slovingu, slovačtinu sa budete učiť s radosťou!"
},
"native_lang": {
  "code": "fr",
  "name": "français",
  "tts_code": "fr-FR",
  "flag": "🇫🇷",
  "css_class": "col-native"
}
```

- `code` : code ISO court, utilisé notamment par `generator` (voir plus bas).
- `tts_code` : code de langue BCP-47 passé à l'API de synthèse vocale du
  navigateur (`speechSynthesis`).
- `voice_hint` : mot-clé utilisé pour préférer une voix installée correspondant
  à la langue quand plusieurs voix système sont disponibles.
- `tts_sample_phrase` : phrase jouée quand l'utilisateur teste une voix dans
  les réglages.
- `css_class` : classe appliquée aux cellules de la colonne correspondante
  dans une translate-table (voir Format-SMD.txt).

## site

Métadonnées générales du site généré (PWA, manifeste, thème).

```json
"site": {
  "title": "Ahoj Slovenčina",
  "short_name": "Ahoj SK",
  "description": "Apprentissage du slovaque - fiches et exercices",
  "theme_color": "#0b4ea2",
  "storage_prefix": "sk-fr",
  "url_path": "/slovingo/sk-fr/",
  "user_name_default": "Eric",
  "user_name_placeholder": "Ton prénom",
  "deploy_host": "www.example.org",
  "deploy_path": "/var/www/example.org/htdocs/slovingo/sk-fr/"
}
```

- `storage_prefix` : préfixe utilisé pour toutes les clés `localStorage` de ce
  cours (progression, réglages, prénom saisi...). Il doit être unique entre
  cours si plusieurs cours partagent un même domaine, pour ne pas mélanger
  leurs données.
- `user_name_default` / `user_name_placeholder` : voir la section
  **USER_NAME** ci-dessous.
- `deploy_host` / `deploy_path` : utilisés par `src/publish.py` pour le
  déploiement (optionnel, seulement si tu utilises ce script tel quel).

## categories

Traduit les 5 catégories fixes du moteur (`introduction`, `series`, `dialog`,
`vocabulary`, `annex`) dans la langue de l'apprenant. Ces clés sont fixes,
n'en ajoute pas d'autres ici : une catégorie non listée dans ce dict retombe
simplement sur son nom brut.

```json
"categories": {
  "introduction": "Avant de commencer",
  "series": "Séries",
  "dialog": "Dialogues",
  "vocabulary": "Vocabulaire",
  "annex": "Annexe"
}
```

## subgroups — ⚠️ point d'attention

C'est la clé la plus souvent mal renseignée : voir l'incident documenté plus
bas. Un sous-groupe existe pour une fiche nommée avec le schéma à 6+
segments `XX_Category_XX_Subgroup_XX_Title.md` (voir Fiches-Serie.txt) — par
exemple `10_Series_01_Rodina_03_praca.md` a pour sous-groupe `Rodina`.

**La clé attendue dans `subgroups` est le sous-groupe tel qu'il apparaît
dans le nom de fichier, in extenso et en minuscules — jamais préfixé par
la catégorie.** Concrètement, `parse_sheet_filename()` dans
`src/smd2data.py` calcule :

```python
subgroup_slug = "_".join(middle).lower()
```

où `middle` est la liste des segments du nom de fichier strictement entre
`(order1, category)` et `(order, title)` — la catégorie (`Series`,
`Introduction`...) n'en fait **pas** partie. `resolveSubgroupLabel()` dans
`static/app.js` cherche ensuite `LANG.subgroups[group.subgroup]`.

| Nom de fichier (segment sous-groupe) | Clé attendue dans `subgroups` | Clé **incorrecte**, ne matchera jamais |
|---|---|---|
| `..._Rodina_...` | `"rodina"` | ~~`"series_rodina"`~~ |
| `..._VelkaNoc_...` | `"velkanoc"` | ~~`"series_velkanoc"`~~ |
| `..._Kronika_VstupDoEU_...` (deux segments) | `"kronika_vstupdoeu"` | ~~`"series_kronika_vstupdoeu"`~~ |

```json
"subgroups": {
  "velkanoc": "Veľká noc",
  "kronika_vstupdoeu": "Kronika : Vstup do EÚ"
}
```

Cette entrée n'est **qu'un override optionnel**, jamais requise : en son
absence, `humanize_subgroup()` dérive automatiquement un libellé lisible à
partir de la casse du nom de fichier (découpage du camelCase, ex.
`VelkaNoc` → `Velka Noc`). C'est un filet de sécurité, pas une solution :
il ne peut pas restituer des diacritiques absents d'un nom de fichier
ASCII (`VelkaNoc` → `Velka Noc`, jamais `Veľká noc`). C'est justement pour
ce cas que `subgroups` existe : donner le vrai libellé accentué là où le
nom de fichier ne peut pas le porter.

**Incident documenté (2026-09) :** deux cours du dépôt ont utilisé la
mauvaise convention (préfixe `series_` devant le sous-groupe), ce qui fait
qu'aucune de leurs entrées `subgroups` ne matche jamais rien. L'effet est
resté invisible pour un cours dont les sous-groupes n'ont pas de
diacritiques, mais s'est traduit par une perte réelle de diacritiques
partout où l'override existait précisément pour ça. Vérifie ta propre
`lang.json` si tu as un doute — voir les issues liées à ce document dans les
dépôts concernés.

## ui

Toutes les chaînes d'interface (boutons, réglages, messages). Vois la liste
complète directement dans un `lang.json` existant : elle est longue mais
plate, une simple table clé → texte traduit.

## generator

Mots-clés utilisés par `smd2data.py` pour détecter automatiquement, dans
l'en-tête d'une translate-table, quelle colonne correspond à la langue
apprise et laquelle correspond à la langue native (voir la section
Translate-table de Format-SMD.txt).

```json
"generator": {
  "target_headers": ["slovak", "slovaque", "slovenčina", "sk", "..."],
  "target_header_roots": ["slovak", "slovenčina"],
  "native_headers": ["french", "français", "fr"],
  "native_header_roots": ["français", "french"]
}
```

- `target_headers` / `native_headers` : correspondance exacte (insensible à
  la casse) contre l'en-tête de colonne.
- `target_header_roots` / `native_header_roots` : correspondance par
  préfixe, pour couvrir les variantes fléchies d'un même mot-clé qu'on ne
  veut pas toutes lister explicitement.

---

## Checklist rapide pour une nouvelle instance

1. Copie un `lang.json` existant en entier (pas seulement un sous-ensemble
   de ses clés).
2. Adapte `target_lang`, `native_lang`, `site`, `categories`.
3. Laisse `subgroups` vide (`{}`) tant que tu n'as pas de sous-groupe avec
   des diacritiques que l'ASCII du nom de fichier ne peut pas porter — et
   quand tu en ajoutes un, vérifie la convention ci-dessus (pas de préfixe
   de catégorie).
4. Adapte `generator` aux mots que les en-têtes de tes translate-tables
   utiliseront réellement.
5. `ui` peut rester identique à un cours existant dans la même langue
   native, en ajustant seulement ce qui te semble mal traduit.
