# Zajka Run 🦊 (ODROID-GO)

Un jeu de plateforme dans l'univers de [Zajka Friends](https://github.com/lstux/Slovingo-sk-fr-friends) :
le renard traverse la Slovaquie de Košice à Bratislava. Ici, pas de leçon : c'est un pur jeu
d'action à la Mario / Sonic, avec le slovaque juste en décor (panneaux, « Stlač START »,
« Ahoj! To si ty? » d'Andrea à l'arrivée).

Niveau 1 : **Košice, rue Hlavná**. Pavés, maisons bourgeoises, théâtre à coupole verte,
Dóm svätej Alžbety, tram Tatra à prendre comme ascenseur, pigeons, guêpes et
**medovníky** (cœurs en pain d'épices) à ramasser. 50 medovníky = une vie.

## Commandes

| bouton | action |
|---|---|
| croix | se déplacer |
| A | sauter (tenir pour sauter plus haut, rebondir plus haut sur un ennemi ou un ressort) |
| B | courir (indispensable pour le grand trou) |
| START | pause |
| SELECT (en pause) | retour à l'écran titre |
| VOLUME | volume (5 niveaux) |
| MENU (tenir 1 s) | retour au lanceur du firmware d'origine |

## Installer sur la console (firmware d'origine)

1. Récupérer `ZajkaRun.fw` : dans les *Artifacts* du workflow GitHub Actions
   « Zajka Run (ODROID-GO) », ou en compilant (ci-dessous).
2. Le copier dans `/odroid/firmware/` sur la carte SD.
3. Éteindre, puis allumer en tenant **B** : choisir *Zajka Run*, valider avec START.

## Compiler

```sh
pio run                 # .pio/build/odroid-go/firmware.bin et ZajkaRun.fw
pio run -t upload       # ou flasher en USB (remplace le firmware d'origine)
```

Le `.fw` est fabriqué après le build par `scripts/make_fw.py` → `tools/mkfw.py`
(portage Python de l'outil `mkfw` d'origine).

## Organisation

```
src/main_odroid.cpp   matériel : écran (LovyanGFX, DMA par bandes), boutons, son (DAC), lanceur
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
  `python3 tools/build_assets.py`. Un nouveau fichier `levels/1-2_xxx.txt` devient le niveau suivant.
- **Les graphismes** : voir `art/PROMPTS.md` (prompts Gemini / ChatGPT, puis
  `tools/import_ai.py` pour les importer). Les images actuelles sont provisoires :
  décors dessinés par `tools/make_placeholder_art.py`, personnages rendus depuis les SVG
  de MathPulse par `tools/make_sprites.py`.
- **La physique** : constantes en tête de `src/game/game.cpp` (vitesse, gravité, saut...).

## Tester sur PC

```sh
make -C sim
build/sim --bot --every 120 --out /tmp/zr          # le robot joue, une capture toutes les 2 s
build/sim --bot --start-x 2950 --out /tmp/zr       # démarre près de l'arrivée
build/sim --script mon_script.txt --wav --out /tmp/zr
```

Un script liste des plages d'images et des boutons (`100 102 S` = START des images 100 à 102,
`260 330 RB` = droite + B), et `shot 318` pour une capture.

## Crédits

Personnages : renard et lapine de Slovingo / MathPulse. Musique « Hlavná » : air original
(ré majeur, quarte augmentée à la manière des chants slovaques, rythme de polka).
Polices : DejaVu Sans (licence Bitstream Vera) et Poppins (SIL OFL), converties en bitmaps.
