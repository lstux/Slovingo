# Zajka — versions web (HTML / JavaScript)

Les deux jeux de la console ESP32 / Odroid-Go, jouables dans un navigateur :

| jeu | page |
|---|---|
| **Zajka Run** (plateformes) | `zajka-run/index.html` |
| **Zajka Quest** (aventure et cours de slovaque, chapitres 1 et 2) | `zajka-quest/index.html` |

Fichiers statiques, sans serveur ni compilation : ouvrir `index.html` (le lanceur), ou publier le
dossier tel quel (GitHub Pages...). Les scripts sont classiques (pas de modules), ça marche aussi
en `file://`.

## Fidélité

Le code de jeu est un **portage ligne à ligne** du C++ du firmware (`esp32/zajka-run`,
`esp32/zajka-quest`) : même logique à 60 Hz, mêmes images, polices, niveaux, cartes, textes du cours,
même synthé audio (22 050 Hz, entiers), même rendu 320 × 240 en RGB565 par bandes de 40 lignes.
Zajka Run reproduit même l'arithmétique `float` 32 bits du firmware (`Math.fround`).

Les images, niveaux, cartes et textes ne sont **pas recopiés à la main** : ils sont lus dans les
fichiers que le firmware embarque (`src/game/gen/*.inc`). Quand les jeux ESP32 évoluent, on
régénère les données et la version web suit.

## Suivre les jeux ESP32

Après avoir régénéré les assets côté ESP32 (`tools/build_assets.py`, `tools/build_course.py`) :

```sh
python3 web-games/tools/export_run.py          # images, polices, niveaux de Zajka Run
python3 web-games/tools/export_quest.py        # images, cartes, cours et scènes de Zajka Quest
python3 web-games/tools/export_audio.py run    # musique et effets (parse audio.cpp)
python3 web-games/tools/export_audio.py quest
```

Si la **logique** change (`game.cpp`, `world.cpp`, `dialog.cpp`, `story.cpp`, `chapterN.cpp`...), il
faut reporter le changement dans `zajka-*/js/` (même découpage de fichiers et mêmes noms que le C++,
pour retrouver facilement l'endroit). Un nouveau chapitre de Quest demande en plus ses fonctions
`chN...` dans `zajka-quest/js/game.js`.

## Vérifier que la version web reste identique

Les tests rejouent les mêmes scénarios dans le simulateur C++ du firmware et dans la version web
(Node, sans navigateur), puis comparent le journal des événements, chaque capture d'écran (PPM)
et le son (WAV) octet par octet :

```sh
web-games/tests/compare_run.sh      # Zajka Run : 4 niveaux, robot, Andrea, trajectoires image par image
web-games/tests/compare_quest.sh    # Zajka Quest : chapitres 1 et 2, choix, menus, sauvegarde, reprise
node web-games/tests/browser_page.js zajka-quest /tmp/captures   # la vraie page dans Chromium (Playwright)
```

Il faut `g++`, `node` (≥ 18) et, pour le dernier, `playwright`. Résultat attendu : « 0 lignes
différentes, 0 captures différentes, son ok ». Les simulateurs `tests/sim_run.js` et
`tests/sim_quest.js` ont la même ligne de commande que ceux du firmware (`--script`, `--bot`,
`--auto`, `--shots`, `--wav`, `--save`...).

## Commandes

| | clavier | manette | écran tactile |
|---|---|---|---|
| croix | flèches, WASD | croix, stick | croix |
| A (sauter, valider) | Espace, K | A, Y | A |
| B (courir, annuler) | Maj, J, X | B, X | B |
| Start | Entrée, P | Start | START |
| Select | Tab, C, Retour arrière | Select | SELECT |
| volume | V | L, R | VOL |

Les boutons tactiles n'apparaissent que sur un écran tactile. Les touches suivent la position
physique (clavier AZERTY compris).

## Différences avec la console

- **Sauvegarde** : `localStorage` du navigateur (la console utilise la NVS). Zajka Quest garde le
  même bloc de 68 octets que le firmware (même somme de contrôle).
- **Volume** : 2 au premier lancement (la console démarre muette) ; il est mémorisé. Le son démarre
  au premier appui ou toucher (règle des navigateurs).
- **MENU** (retour au lanceur de la console) : remplacé par le lien « Jeux ».
- Pas de carte SD : le cours est embarqué, comme dans le firmware actuel.
