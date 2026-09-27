# Pipeline CI : build & release d'une instance de langue

Ce document décrit comment une instance de langue (un repo `Slovingo-<code>`,
par ex. `Slovingo-sk-fr`, `Slovingo-fr-sk`, `Slovingo-bzh-fr`) est buildée et
publiée en Release GitHub, et comment brancher une nouvelle instance sur ce
mécanisme (par ex. `Slovingo-de-fr`).

## Principe général

- **Ce repo (`Slovingo`)** contient uniquement le "code" : le moteur
  (`src/`), le front-end statique (`static/`), la licence. C'est la seule
  source de vérité pour la logique de build.
- **Un repo par instance de langue** (`Slovingo-<code>`) contient uniquement
  le "contenu" : les fiches SMD, les images, les exceptions d'exercices, la
  config de langue. Chaque instance a son propre cycle de vie et ses propres
  releases, indépendants du code.
- La CI relie les deux à la demande : elle récupère le contenu d'un côté, le
  code de l'autre, les assemble, build, et publie le résultat comme Release
  GitHub **sur le repo de contenu**.

Aucune pipeline n'est dupliquée d'un repo de contenu à l'autre : la logique
vit une seule fois ici, dans un *reusable workflow*, et chaque repo de
contenu se contente de l'appeler.

## Structure attendue d'un repo de contenu

Un repo `Slovingo-<code>` doit contenir, à sa racine, exactement ce
qu'attend `src/build.py` pour un "lang dir" (voir son docstring) :

```
Slovingo-<code>/
├── md/            fiches SMD sources (le seul format à éditer à la main)
├── img/           images référencées par les fiches (optionnel)
├── exercises/     exceptions d'exercices persistantes (si utilisé)
├── lang.json      config de la langue (target_lang, native_lang, site...)
├── .gitignore     ignore dist/ et json/ (voir plus bas)
└── .github/
    └── workflows/
        └── build-release.yml   (voir "Mettre en place une nouvelle instance")
```

**Ne jamais committer `dist/` ni `json/`** : ce sont des dossiers générés à
chaque build (voir `src/build.py`). Les garder dans le repo pose un
problème concret : `build.py` ne régénère **pas** un fichier
`json/*.exercises.json` qui existe déjà (persistance volontaire pour les
exceptions faites main). Un `json/` committé et périmé se ferait donc
silencieusement garder tel quel au lieu d'être régénéré à partir des `.md`
à jour.

## Le reusable workflow (dans ce repo)

Fichier : [`.github/workflows/build-release.yml`](../.github/workflows/build-release.yml)

Déclenché via `on: workflow_call`, avec trois inputs :

| Input          | Obligatoire | Défaut   | Rôle                                                          |
|----------------|:-----------:|----------|----------------------------------------------------------------|
| `lang_code`    | oui         | —        | Nom du dossier langue (`sk-fr`, `fr-sk`, `bzh-fr`, `de-fr`...) |
| `slovingo_ref` | non         | `main`   | Branche/tag/commit de ce repo à utiliser pour le build         |
| `release_tag`  | non         | (auto)   | Tag de la release ; vide = date du jour (`YYYY.MM.DD`)          |

Ce qu'il fait :

1. Checkout du repo **appelant** (le contenu) dans `content/`.
2. Checkout de **ce repo** (`lstux/Slovingo`) sur `slovingo_ref`, dans `engine/`.
3. Copie le contenu dans `engine/langs/<lang_code>/` (exclut `.git`,
   `.github`, `LICENSE`, `README*`).
4. Build : `python3 src/build.py --lang-dir langs/<lang_code> --static-dir static`
   — aucune dépendance externe, pas de `pip install` nécessaire.
5. Package `dist/` en tarball.
6. Publie une Release GitHub sur le repo appelant, taguée `release_tag` (ou
   la date du jour).

**Contexte d'exécution important** : quand un repo de contenu appelle ce
workflow via `uses: lstux/Slovingo/.github/workflows/build-release.yml@main`,
le job s'exécute dans le contexte du repo **appelant** — c'est donc lui qui
reçoit la Release (le `GITHUB_TOKEN` du job est scopé au repo appelant, pas
à `Slovingo`).

## Mettre en place une nouvelle instance (ex: `Slovingo-de-fr`)

1. **Créer le repo** `lstux/Slovingo-de-fr` sur GitHub (vide, sans README
   auto-généré pour éviter tout conflit).

2. **Ajouter le contenu** à la racine : `md/`, `img/`, `exercises/` (si
   besoin), `lang.json`.

3. **Ajouter `.gitignore`** :
   ```gitignore
   json/
   dist/
   __pycache__/
   ```

4. **Ajouter `.github/workflows/build-release.yml`** :
   ```yaml
   name: Build & Release

   on:
     workflow_dispatch:
       inputs:
         slovingo_ref:
           description: "Branche/tag/commit du repo Slovingo (code) à utiliser pour le build"
           required: false
           default: "main"
         release_tag:
           description: "Tag de la release à créer (ex: 2026.09.27). Laisser vide = date auto."
           required: false
           default: ""

   permissions:
     contents: write

   jobs:
     build:
       uses: lstux/Slovingo/.github/workflows/build-release.yml@main
       with:
         lang_code: de-fr   # <-- seule ligne à adapter par instance
         slovingo_ref: ${{ inputs.slovingo_ref }}
         release_tag: ${{ inputs.release_tag }}
       permissions:
         contents: write
   ```

   Seule la valeur de `lang_code` change d'un repo de contenu à l'autre —
   tout le reste de ce fichier est identique pour chaque instance.

5. **Lancer un build** : onglet **Actions → Build & Release → Run workflow**
   sur le repo de contenu. Laisser `slovingo_ref` à `main` sauf besoin
   spécifique de figer une version du moteur ; laisser `release_tag` vide
   pour un tag automatique par date.

## Pourquoi `slovingo_ref` par défaut à `main` (et pas un tag figé)

Choix assumé pour la simplicité : chaque build utilise la dernière version
du moteur, sauf si on renseigne explicitement un tag/commit dans
`slovingo_ref` au moment du déclenchement. Une ancienne release reste donc
reproductible seulement si on retrouve/rejoue avec le `slovingo_ref` utilisé
à l'époque (visible dans le corps de la Release GitHub, qui l'indique
toujours).

## Pourquoi des repos gratuits GitHub-hosted suffisent

Tous les repos concernés sont publics (licence GPL v3) : les runners
GitHub-hébergés (`runs-on: ubuntu-latest`) y sont gratuits et illimités.
Pas besoin de runner self-hosted, sauf besoin matériel particulier (aucun
ici : le build est du Python pur, quelques secondes).

## Dépannage rapide

- **Le job échoue à l'étape "Build"** : vérifier que `lang.json` est bien
  un JSON valide et que `md/` contient au moins un fichier `.md`.
- **La release contient un contenu périmé** : vérifier qu'aucun `json/` ou
  `dist/` n'a été committé par erreur dans le repo de contenu (voir
  `.gitignore` ci-dessus) — sinon `build.py` peut garder des exercices
  périmés au lieu de les régénérer.
- **"Resource not accessible by integration" sur la création de la
  release** : vérifier que le fichier appelant a bien
  `permissions: contents: write` à la fois au niveau du workflow et au
  niveau du job `build`.
