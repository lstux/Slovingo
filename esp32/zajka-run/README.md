# Zajka Run 🦊 (ODROID-GO)

Un jeu de plateforme dans l'univers de [Zajka Friends](https://github.com/lstux/Slovingo-sk-fr-friends) :
le renard traverse la Slovaquie de Košice à Bratislava, et Andrea peut prendre sa place. Ici, pas de leçon : c'est un pur jeu
d'action à la Mario / Sonic, avec le slovaque juste en décor (panneaux, « Stlač START »,
« Ahoj! To si ty? » d'Andrea à l'arrivée).

Niveau 1 : **Košice, rue Hlavná**. Pavés, maisons bourgeoises, théâtre à coupole verte,
Dóm svätej Alžbety, tram Tatra à prendre comme ascenseur, pigeons, guêpes et
**medovníky** (cœurs en pain d'épices) à ramasser. 50 medovníky = une vie.

Les niveaux suivent l'ordre du cours.

Niveau 2 : **Sucho, la sécheresse à Košice**. Canicule, la fontaine chantante devant le théâtre
est à sec : ses **buses** crachotent puis propulsent le renard (A maintenu pour monter plus haut),
le **sol desséché** s'effondre après un instant, étang boueux à éviter, et pán Záhradník,
le jardinier, à l'arrivée.

Niveau 3 : **Slovenský raj, la gorge de Suchá Belá** (série Raj du cours). Forêt sèche,
ruisseau, puis la gorge : **échelles** (croix haut/bas, A pour sauter), passerelles
au-dessus du torrent (l'eau fait perdre une vie), passerelles mobiles et ascenseur,
écureuils, **bouteilles d'eau** qui rendent un cœur, et pán Orol, le garde du parc, à l'arrivée.

Niveau 4 : **Vysoké Tatry, l'orage**. Montée vers le refuge : le ciel s'assombrit, la pluie
puis l'orage arrivent (éclairs et tonnerre), **chaînes** à grimper, **éboulis** qui cèdent,
**rafales de vent** qui repoussent le renard, pierres qui tombent, lac glacé, et le chatár
du refuge à l'arrivée.

**Andrea** (la lapine) est jouable aussi : sur l'écran titre, **SELECT** passe du renard à
Andrea. Elle a les mêmes mouvements que le renard (course, saut, échelle, choc).

La progression est sauvegardée (niveaux débloqués, meilleur temps et medovníky par niveau) ;
sur l'écran titre, gauche/droite choisit le niveau. Le choix du personnage n'est pas encore sauvegardé.

## Commandes

| bouton | action |
|---|---|
| croix | se déplacer, monter/descendre une échelle |
| A | sauter (tenir pour sauter plus haut, rebondir plus haut sur un ennemi ou un ressort) |
| B | courir (indispensable pour le grand trou) |
| START | pause |
| SELECT (écran titre) | changer de personnage : renard ou Andrea |
| SELECT (en pause) | retour à l'écran titre |
| VOLUME | volume (5 niveaux) |
| MENU (tenir 1 s) | retour au lanceur du firmware d'origine |

## Installer sur la console (firmware d'origine)

1. Récupérer `ZajkaRun.fw` : dans les *Artifacts* du workflow GitHub Actions
   « Zajka Run (ODROID-GO) » (artefact `ZajkaRun-odroid-go`), ou en compilant (ci-dessous).
2. Le copier dans `/odroid/firmware/` sur la carte SD.
3. Éteindre, puis allumer en tenant **B** : choisir *Zajka Run*, valider avec START.

## Compiler

```sh
pio run                 # env odroid-go-idf (par défaut) : .pio/build/odroid-go-idf/ZajkaRun.fw
pio run -t upload       # flasher en USB (remplace le firmware d'origine, voir plus bas)
```

Deux environnements PlatformIO :

- **`odroid-go-idf`** (par défaut) : ESP-IDF 4.4, sans somme MD5 dans la table de partitions.
  C'est celui qui démarre depuis le `.fw` installé par le lanceur d'origine.
- **`odroid-go`** : Arduino. Il fonctionne en flashage USB, mais plante au démarrage depuis le
  `.fw` (« No MD5 found in partition table ») : la bibliothèque Arduino précompilée exige la somme MD5.

Le `.fw` est fabriqué après le build par `scripts/make_fw.py` → `tools/mkfw.py`
(portage Python de l'outil `mkfw` d'origine).

Le flashage USB remplace la table de partitions du lanceur d'origine : pour le retrouver,
réécrire une sauvegarde complète de la flash (`esptool.py write_flash 0x0 sauvegarde.bin`).
Faire cette sauvegarde avant tout `pio run -t upload`.

## Organisation

```
src/main_odroid_idf.cpp   matériel (ESP-IDF) : écran (LovyanGFX, DMA par bandes), boutons, son (DAC)
src/main_odroid.cpp   matériel (Arduino, flashage USB seulement)
src/game/             le jeu, sans Arduino : physique, ennemis, tram, états, rendu, synthé
src/game/gen/         données générées (images, polices, niveaux) : ne pas éditer
levels/*.txt          niveaux en texte (légende en tête de fichier)
art/                  images sources (PNG) ; art/PROMPTS.md pour les remplacer par des images IA
tools/                générateurs et convertisseurs (Python 3 + Pillow, numpy, matplotlib)
sim/                  simulateur PC : même code de jeu, captures PPM et son WAV
```

Le rendu compose l'écran par bandes de 320 × 40 dans deux petits tampons, envoyés par DMA
pendant qu'on calcule la bande suivante : pas besoin d'un tampon d'écran complet. La logique
tourne à 60 Hz fixes, l'affichage autour de 30 images/s.

## Modifier

- **Un niveau** : éditer `levels/1-1_kosice.txt` (caractères expliqués en tête), puis
  `python3 tools/build_assets.py`. Les fichiers sont joués dans l'ordre de leur nom (`1-5_xxx.txt` serait le suivant).
- **Les graphismes** : voir `art/PROMPTS.md` (prompts Gemini / ChatGPT, état de chaque
  image, puis `tools/import_ai.py` pour les importer ; les images IA d'origine sont gardées
  dans `art/sources/`). Tous les graphismes viennent maintenant de Gemini (sauf les icônes du HUD).
  Les générateurs provisoires (`tools/make_placeholder_art.py`, `tools/make_sprites.py`) restent
  pour de nouveaux niveaux ; `make_placeholder_art.py` écrase les images importées.
  Les images provisoires de Sucho, Raj et Tatry viennent de `make_placeholder_levels.py` et
  `make_placeholder_raj.py`, qui ne touchent jamais une image IA importée.
- **Les graphismes** : Andrea a une planche de cases 48×48 (`art/sprites/rabbit_48x48.png`),
  même format que le renard : 7 lignes (idle, run, jump, fall, hurt, win, climb).
- **La physique** : constantes en tête de `src/game/game.cpp` (vitesse, gravité, saut...).

## Tester sur PC

```sh
make -C sim
build/sim --bot --every 120 --out /tmp/zr          # le robot joue, une capture toutes les 2 s
build/sim --andrea --bot --out /tmp/zr             # le robot joue avec Andrea
build/sim --bot --start-x 2950 --out /tmp/zr       # démarre près de l'arrivée
build/sim --level 2 --start-x 1300 --start-y 272 --script s.txt --out /tmp/zr   # niveau 2, position donnée
build/sim --save /tmp/zr.sav ...                    # sauvegarde dans un fichier
build/sim --script mon_script.txt --wav --out /tmp/zr
```

Un script liste des plages d'images et des boutons (`100 102 S` = START des images 100 à 102,
`260 330 RB` = droite + B), et `shot 318` pour une capture.

## Crédits

Personnages : renard et lapine de Slovingo / MathPulse. Musique « Hlavná » : air original
(ré majeur, quarte augmentée à la manière des chants slovaques, rythme de polka).
Polices : DejaVu Sans (licence Bitstream Vera) et Poppins (SIL OFL), converties en bitmaps.
