# Page d'accueil (launcher) et site multi-cours

## Pourquoi

Quand plusieurs cours sont publiés sur la même origine GitHub Pages
(`lstux.github.io/Slovingo-sk-fr-kids/`, `lstux.github.io/Slovingo-de-fr-kids/`…),
Chrome sur Android peut considérer qu'une app est « déjà installée » pour tous les
cours dès qu'un seul est installé, et refuser d'en installer un autre.

Le site assemblé ici n'a **qu'une seule app à installer** : la page d'accueil
`/Slovingo/`. Son `scope` contient tous les cours, qui sont publiés dans des
sous-dossiers : `/Slovingo/<cours>/`.

Les repos de cours continuent de publier leur propre site, indépendamment : ce
site-ci vient en plus.

## Fonctionnement

- `launcher/index.html` affiche la liste des cours (`courses.json`, généré au
  build). Le dernier cours choisi est retenu dans le `localStorage`
  (clé `slovingo:last-course`) et la page d'accueil y redirige directement.
- `/Slovingo/?choose` force l'affichage de la liste (lien « Changer de cours »
  depuis un cours : `../?choose`).
- `launcher/service-worker.js` ne gère que la coquille de la page d'accueil et
  laisse passer tout le reste : les cours gardent leur propre service worker.
- Le manifest de la page d'accueil a un `id` explicite (`slovingo-launcher`) et
  des `start_url` / `scope` relatifs (`./`), donc indépendants du nom du repo.

## Lien « Changer de cours »

Dans les réglages d'un cours (tout en bas), un bouton « Changer de cours » mène à
`../?choose`, donc à la liste des cours de la page d'accueil. Il n'apparaît que si
le cours est publié dans ce site : `build_site.py` ajoute `site.launcher_url: "../"`
au `lang.json` du cours au moment du build. Un cours publié seul n'a pas cette clé,
donc pas de lien.

Le texte vient de `ui.change_course`. Si le cours ne le définit pas, `build_site.py`
met un texte par défaut dans la langue du cours (fr, sk, en, de) ; un `lang.json`
peut toujours le surcharger. Un build isolé affiche un avertissement
`check_ui_keys` de plus pour cette clé : sans effet visible, puisque le lien
n'y est pas affiché.

## Liste des cours en JSON (« API »)

`/Slovingo/courses.json` sert à la fois à la page d'accueil et à d'autres clients
(montre, widget…) :

```json
{
  "version": 1,
  "courses": [
    {
      "id": "sk-fr-kids",
      "path": "sk-fr-kids/",
      "api": "sk-fr-kids/api/index.json",
      "title": "Zajka - apprends le Slovaque",
      "short_name": "Zajka",
      "description": "…",
      "flag": "🇸🇰",
      "companions": ["🦊", "🐰"],
      "theme_color": "#0b4ea2"
    }
  ]
}
```

Les chemins sont relatifs à `courses.json`. `api` pointe vers l'API légère du
cours (`src/export_api.py` : `index.json`, `sheets/<id>.json`, `exercises/<id>.json`).
Les illustrations locales des fiches (`@ img/fichier.jpg | Légende`) y sont aussi exportées, réduites en
JPEG baseline (224×150 max) dans `img/<id>-<n>.jpg`, et référencées par des blocs `{"t":"img",…}` des
fiches. C'est facultatif : sans Pillow, avec `--no-images`, ou pour une image absente ou encore distante
(`http…`, voir `fetch_images.py`), l'image est omise avec un avertissement et l'export continue.

## Ajouter un cours

1. Ajouter son code dans `launcher/course-ids.json` (le repo doit s'appeler
   `lstux/Slovingo-<code>`, branche `main`).
2. Relancer le workflow « GitHub Pages (launcher + cours) ».

Un cours introuvable ou qui échoue à se construire est ignoré (avertissement dans
le résumé du workflow) : les autres sont publiés quand même. Le build n'échoue que
si aucun cours n'a pu être construit. Pour retirer un cours du site, l'enlever de
`launcher/course-ids.json`.

## Build

```
python3 src/build_site.py --content-root content --launcher-dir launcher \
    --static-dir static --base-path /Slovingo/ --out _site
```

`content/` contient un checkout par cours, nommé d'après son code
(`content/sk-fr-kids/`…). Chaque cours est construit comme d'habitude avec
`src/build.py`, avec seulement `site.url_path` réécrit en `/Slovingo/<cours>/`.
`storage_prefix` ne change pas : la progression est la même quelle que soit
l'adresse, tant que l'origine est la même.

## Caches des service workers

Toutes les apps d'une même origine partagent le stockage de cache. Le service
worker d'un cours ne supprime donc que ses propres anciennes versions
(`<storage_prefix>-vN`), jamais les caches des autres cours ni de la page
d'accueil.

## Déploiement

Le workflow `.github/workflows/pages.yml` publie depuis `main` (push, chaque nuit
vers 3 h UTC, ou à la main). Il faut que la source de GitHub Pages soit réglée
sur « GitHub Actions » dans les paramètres du repo.
