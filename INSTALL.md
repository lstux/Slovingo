# Installation

Moteur générique pour tes mémos de langue (SMD → HTML + exercices +
PWA). Une seule copie du code (`src/`), un dossier par cours
(`langs/<code>/`), aucune langue codée en dur.

## 0. Prérequis

- Python 3 (aucune dépendance externe à installer)
- `rsync` si tu comptes publier sur un serveur distant
- Un navigateur pour vérifier le résultat (`file://` suffit pour un
  coup d'œil rapide, un vrai serveur local pour tester les exercices —
  voir étape 3)

## 1. Vérifier que ça tourne (cours d'exemple fourni)

Un mini-cours d'anglais de démonstration est prêt dans
`langs/example/`, pour vérifier que tout fonctionne avant de toucher
à quoi que ce soit :

```sh
./publish.sh -l langs/example -g -n -d
```

- `-g` : génère les fiches HTML + exercices depuis les `.md`
- `-n` : ignore les fiches privées (dossier `md/private/`, vide ici)
- `-d` : dry-run, ne tente pas de rsync vers un serveur

Le résultat apparaît dans `langs/example/html/`. Ouvre
`langs/example/html/index.html` dans un navigateur : tu dois voir une
page "Hello English (exemple)" avec une fiche de vocabulaire. Une fois
que ça marche, tu peux supprimer `langs/example/` entièrement — il ne
sert qu'à ce test.

**Remarque sur les exercices** : `exercises.html` charge ses données
en `fetch()`, ce qui ne fonctionne pas en `file://` dans certains
navigateurs (Chrome, notamment). Pour tester l'écran d'exercices,
lance un petit serveur local depuis la racine du cours :

```sh
cd langs/example/html && python3 -m http.server 8000
# puis ouvre http://localhost:8000/
```

## 2. Mettre en place le slovaque (cours existant)

`langs/sk/lang.json` reprend déjà exactement ta config actuelle (TTS
`sk-SK`, titre "Ahoj Slovenčina", déploiement `lslinux.org/slovak/`,
etc.) — rien à changer ici a priori.

Ce qu'il te reste à faire :

1. Copie tes fiches `.md` existantes dans `langs/sk/md/` (et tes
   fiches privées dans `langs/sk/md/private/`)
2. Remplace `langs/sk/parcours.txt` par ton vrai sommaire (celui-ci
   n'est qu'un squelette vide, pour ne rien écraser par erreur)
3. Si tu as des photos d'illustration ou des icônes PWA personnalisées,
   place-les dans `langs/sk/img/` et `langs/sk/icons/` (facultatif —
   sans ça, les fiches retombent sur motif + dégradé, et la PWA sur les
   icônes par défaut)
4. Génère et publie :

```sh
./publish.sh -l langs/sk -g -b
```

(`-b` fait une sauvegarde de `langs/sk/html/` dans
`langs/sk/backups/` avant d'écraser — pas la peine avec `-d` en plus
puisque rien n'est déployé, mais utile en publication réelle)

Comme `langs/sk` est le dossier par défaut, une fois que tout est en
place tu peux aussi simplement lancer `./publish.sh -g` sans `-l`.

## 3. Mettre en place le breton (nouveau cours)

`langs/bzh/lang.json` est un point de départ, à ajuster :

| Champ | Valeur actuelle | À faire |
|---|---|---|
| `target_lang.tts_code` | `br-FR` (le vrai code technique du breton — reste `br`, indépendant du nom de dossier) | Vérifie sur tes appareils si une voix existe pour ce code (`Réglages > Accessibilité > Synthèse vocale` selon l'OS). Si aucune voix bretonne n'est disponible nulle part, le site reste utilisable, juste sans lecture audio. |
| `target_lang.flag` | 🏴 (générique) | Il n'existe pas d'emoji drapeau officiel pour la Bretagne — remplace par ce que tu préfères, y compris juste le texte `"BZH"` |
| `site.deploy_host` / `site.deploy_path` | `lslinux.org` / `/breton/` | À confirmer ou changer selon où tu comptes héberger |
| `series` / `exercise_theme_labels` | vides | À remplir au fur et à mesure que tu définis tes séries bretonnes, sur le modèle de `langs/sk/lang.json` |

Ensuite, même principe que le slovaque : fiches `.md` dans
`langs/bzh/md/`, sommaire dans `langs/bzh/parcours.txt`, puis :

```sh
./publish.sh -l langs/bzh -g -d
```

Retire `-d` quand tu es prêt à réellement publier (ça déclenche un
`rsync` vers `site.deploy_host`/`site.deploy_path` — assure-toi d'avoir
les accès SSH configurés au préalable).

## 4. Ajouter une nouvelle langue plus tard

Aucun code à toucher :

1. `cp -r langs/bzh langs/nouvelle-langue` (ou repars de `langs/sk`)
2. Édite `langs/nouvelle-langue/lang.json`
3. Remplace le contenu de `md/` et `parcours.txt`
4. `./publish.sh -l langs/nouvelle-langue -g`

## Rappel des options de `publish.sh`

```
./publish.sh [options]

  -l DIR : dossier de langue à publier (défaut : langs/sk)
  -g     : régénère les HTML/JSON depuis les .md
  -b     : sauvegarde langs/<code>/html/ avant publication
  -n     : ignore les fiches privées (md/private/)
  -d     : dry-run, n'exécute pas le rsync final
  -h     : aide
```

## Structure du dossier

```
publish.sh              moteur de publication (générique)
src/                     tout le code — jamais dupliqué, jamais édité par langue
  smd2html.py            SMD -> fiches HTML
  smd2exercises.py       SMD -> exercices JSON
  langconfig.py          chargement de lang.json (Python)
  lang_field.py          lecture de lang.json depuis le shell
  gen_*.py               génèrent lang-config.js / manifest.json / service-worker.js / exercises.html
  *.template.*           gabarits utilisés par les gen_*.py
  app.js, exercises.js,  logique front (TTS, exercices, progression...)
  progress.js, index.js
  style.css,             mise en page + thèmes visuels par série
  exercises.css
langs/
  sk/                    cours slovaque (existant)
    lang.json            configuration : TTS, titre, déploiement, catégories...
    md/                  tes fiches sources (+ md/private/ pour le contenu perso)
    parcours.txt         sommaire pédagogique ordonné
    html/                généré par publish.sh -g — ne pas éditer à la main
    backups/             généré par publish.sh -b
  bzh/                   cours breton (nouveau, à compléter)
  example/               mini-cours de démo — à supprimer une fois le test fait
```

## Licence

GPL v3 — voir `LICENSE`.
