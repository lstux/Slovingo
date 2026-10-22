# Zajka Quest 🦊 (ODROID-GO)

Une aventure en vue de dessus dans l'univers de [Zajka Friends](https://github.com/lstux/Slovingo-sk-fr-friends),
à la Zelda Game Boy ou Pokémon : le renard suit le récit du cours de Košice à Bratislava, explore
les lieux et parle aux personnages. C'est le pendant calme de [Zajka Run](../zajka-run/) : on peut
finir le jeu sans comprendre le slovaque, mais le comprendre rend le voyage plus riche.

> **État : squelette.** Écran titre et carte du chapitre 1 seulement. Viennent ensuite le
> convertisseur du cours, la carte de Košice, les dialogues et la sauvegarde.

## Décisions

| sujet | choix |
|---|---|
| contenu du cours | en flash pour le prototype, sur la carte SD ensuite (même format de données) |
| sauvegarde | NVS |
| langue de l'interface | slovaque, comme Zajka Run |
| récit | les épisodes dans l'ordre ; le prototype rejoue l'épisode 1 |
| bonus de compréhension | répliques secrètes et objets |
| code commun avec Run | copié pour l'instant, factorisé plus tard dans `esp32/zajka-common/` |
| personnages | 16 × 24 en vue de dessus, portraits 48 × 48 dans les dialogues |

## Commandes (prévues)

| bouton | action |
|---|---|
| croix | se déplacer |
| A | parler, valider |
| B | annuler |
| SELECT | traduction française de la réplique |
| START | menu |
| VOLUME | volume (5 niveaux) |
| MENU (tenir 1 s) | retour au lanceur du firmware d'origine |

## Compiler

```sh
pio run                 # .pio/build/odroid-go/firmware.bin et ZajkaQuest.fw
pio run -t upload       # ou flasher en USB (remplace le firmware d'origine)
```

Installation : copier `ZajkaQuest.fw` dans `/odroid/firmware/` sur la carte SD, puis allumer en
tenant **B**. Le `.fw` est aussi dans les *Artifacts* du workflow « Zajka Quest (ODROID-GO) ».

## Organisation

```
src/main_odroid.cpp   matériel : écran (LovyanGFX, DMA par bandes), boutons, son (DAC), lanceur
src/game/             le jeu, sans Arduino ; gfx, audio et input sont repris de Zajka Run
src/game/gen/         données générées (images, polices) : ne pas éditer
art/                  images sources (PNG) ; les sprites 32 × 48 sont ceux de Run, provisoires
tools/                générateurs (Python 3 + Pillow, numpy) ; build_assets.py régénère src/game/gen/
sim/                  simulateur PC : même code de jeu, captures PPM et son WAV
```

`tools/make_sprites.py` et `tools/import_ai.py` sont encore ceux de Run (planches de profil
32 × 48) ; ils seront étendus aux planches 4 directions, aux tuiles et aux portraits.

## Tester sur PC

```sh
python3 tools/build_assets.py      # après un changement dans art/
make -C sim
build/sim --script sim/demo.txt --out /tmp/zq --expect titre,chapitre,titre
```

Un script liste des plages d'images et des boutons (`100 102 S` = START des images 100 à 102)
et `shot 200` pour une capture. `--expect` vérifie que le jeu passe par ces états, dans l'ordre.
