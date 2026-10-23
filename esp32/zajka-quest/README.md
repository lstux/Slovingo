# Zajka Quest 🦊 (ODROID-GO)

Une aventure en vue de dessus dans l'univers de [Zajka Friends](https://github.com/lstux/Slovingo-sk-fr-friends),
à la Zelda Game Boy ou Pokémon : le renard suit le récit du cours de Košice à Bratislava, explore
les lieux et parle aux personnages. C'est le pendant calme de [Zajka Run](../zajka-run/) : on peut
finir le jeu sans comprendre le slovaque, mais le comprendre rend le voyage plus riche.

> **État : le chapitre 1 (Košice) se joue en entier**, des retrouvailles dans le tram à la nuit
> sous le pommier, soit les six fiches de la série Košice du cours. La partie est sauvegardée.
> Les graphismes sont provisoires.

## Le chapitre 1 : Košice

| lieu | ce qui s'y passe |
|---|---|
| rue Hlavná | on y commence ; deux passants disent des « répliques en plus » du cours ; on monte dans le tram à l'arrêt |
| le tram | le Narrateur plante le décor ; chaque fois qu'on parle à Andrea, l'épisode suivant : 1 « To si ty? », 2 « Ako sa máš? », 3 « Čo tu robíš? », 4 « Pamätáš sa na Tatry? », 5 « V električke » ; quatre voyageurs ont aussi leurs répliques. À la fin du 5, on descend avec Andrea |
| chez sa tante, le soir | le Narrateur, puis l'épisode 6 « Dobrú noc » avec Andrea ; on va dormir sous le pommier (A face au tronc) : fin du chapitre |

Le titre de chaque épisode s'affiche quand il commence. L'écran de fin compte les répliques
secrètes gagnées. L'histoire est écrite dans `src/game/story.cpp`.

## Sauvegarde

La partie s'enregistre toute seule dans la NVS de l'ESP32 (espace `zquest`), en passant une
porte et à la fin de chaque conversation : carte, case et orientation du renard, épisodes joués,
répliques secrètes gagnées par épisode. Une sauvegarde d'une version précédente du jeu est
ignorée. Un petit « Uložené » s'affiche en haut à droite.
À l'écran titre, s'il y a une partie : **Pokračovať** (continuer) ou **Nová hra** (nouvelle
partie, après confirmation « Vymazať uloženú hru? », A : oui, B : non).

## Le dialogue

Parler à quelqu'un : A, face à lui. Quand il n'y a plus d'épisode à jouer avec Andrea, elle
redit sa dernière réplique.

- **A** termine la phrase qui s'écrit, puis passe à la suivante (tenu : plus vite).
- **SELECT** passe du slovaque à la traduction française, puis à l'aide du cours (« + »).
- **Les répliques du renard** deviennent des choix quand un exercice du cours s'y prête :
  remettre les morceaux dans l'ordre (exercice `order`) ou choisir la bonne phrase parmi celles
  où un mot est remplacé par les mauvaises réponses d'un `qcm` (« To si ty? » / « To som ty? » /
  « To je ty? »). Une erreur ne coûte rien : on rechoisit.
- **Les répliques secrètes** : un choix réussi du premier coup, sans avoir ouvert la traduction,
  en gagne une, que le personnage dit à la fin de la conversation. Elles sont propres au jeu,
  dans `story/bonus.txt`, **à faire relire par un Slovaque**.

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
| croix | se déplacer (une pression brève tourne le renard sans avancer) |
| A | parler, valider |
| B | courir (tenu) ; annuler dans les menus |
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
src/game/world.*      le monde : cartes en tuiles, déplacement case par case, PNJ, portes, caméra
src/game/dialog.*     la boîte de dialogue : texte lettre par lettre, SK/FR/aide, choix de réplique
src/game/save.*       le bloc de sauvegarde (la console l'écrit en NVS, le simulateur en mémoire)
src/game/story.cpp    l'histoire du chapitre 1 : qui joue quel épisode, où, et dans quel ordre
src/game/course.h     structures du contenu du cours (répliques, mots, exercices)
src/game/gen/         données générées (images, polices, cours) : ne pas éditer
maps/*.txt            cartes en texte (légende en tête de chaque fichier)
story/bonus.txt       répliques secrètes (bonus de compréhension), propres au jeu
art/tiles/            planche de tuiles 16 × 16 (ordre : TILESET dans tools/build_assets.py)
art/walk/             planches de marche 16 × 24 (lignes bas, haut, gauche, droite)
art/sprites/          personnages 32 × 48 de l'écran titre (ceux de Run)
tools/                générateurs (Python 3 + Pillow, numpy) ; build_assets.py régénère src/game/gen/
sim/                  simulateur PC : même code de jeu, captures PPM et son WAV
```

Tuiles et planches de marche sont dessinées par `tools/make_placeholder_art.py`, en attendant
de vraies images. `tools/make_sprites.py` et `tools/import_ai.py` sont encore ceux de Run
(planches de profil 32 × 48) ; ils seront étendus aux tuiles, aux planches 4 directions et aux
portraits.

## Les cartes

Une carte est un fichier texte : un en-tête, `---`, puis une lettre par tuile de 16 px.

```
name: V električke          nom affiché
ground: -                   tuile sous les repères et sous le feuillage
music: -1                   musique (-1 : silence)
1: warp kosice_hlavna 23 10 up on e    porte : carte, case et orientation d'arrivée ; tuile « e » dessous
2: npc andrea down on s                personnage, orientation ; assis sur un siège « s »
---
WWWWWWWWWWWWWWWWWWWW
Wss-ss-ss-s2-ss-ss-W
...
wwwwwwwww1wwwwwwwwww
```

Les lettres et leur genre (on marche dessus, solide, passe devant les personnages) sont dans
`TILESET`, en tête de `tools/build_assets.py`. Un chiffre renvoie à la ligne « chiffre: » de
l'en-tête : `start` (départ), `warp` (porte) ou `npc` (personnage ; `npc human down say 2 0-1`
lui fait dire les « répliques en plus » 0 et 1 de l'épisode 2). `night: 1` affiche la carte de
nuit. `build_assets.py` vérifie
que chaque porte arrive sur une case libre. Après une porte, le renard attend que la croix soit
relâchée avant de repartir.

## Le contenu du cours

Les répliques, le vocabulaire et les exercices viennent du dépôt
[Slovingo-sk-fr-friends](https://github.com/lstux/Slovingo-sk-fr-friends), converti par
`tools/build_course.py` en `src/game/gen/course_data.inc` (commité, comme les images) :

```sh
python3 src/langs.py sk-fr-friends            # à la racine du dépôt : le cours dans langs/
python3 tools/build_course.py --dump          # voir ce qui est lu
python3 tools/build_course.py                 # régénérer après un changement du cours
```

Les fiches converties sont listées dans `EPISODES` en tête du script (les six fiches de la série
Košice). Le script garde le dialogue (`! 🦊 …`, `> traduction`, `> mot = sens`,
`+ aide`), les « répliques en plus », les nouveaux mots et les exercices `qcm`, `listen` et
`order`. `[USER_NAME]` devient le nom choisi par le joueur (par défaut celui de `lang.json`).

## Tester sur PC

```sh
python3 tools/build_assets.py      # après un changement dans art/
make -C sim
build/sim --course                 # résumé du cours embarqué, glyphes manquants dans la police
build/sim --script sim/chapter.txt --auto --save /tmp/zq.sav --out /tmp/zq \
    --expect carte:kosice_tram,parle:Andrea,bonus:2,carte:kosice_teta,bonus:9,fin   # le chapitre
build/sim --script sim/resume.txt --auto --save /tmp/zq.sav --expect reprise,bonus:9,fin   # reprise
build/sim --script sim/chapter.txt --auto-peek ...   # ouvre la traduction à chaque choix : pas de bonus
build/sim --script sim/chapter.txt --auto-wrong ...  # se trompe une fois à chaque choix : pas de bonus
ZQ_TRACE=1 build/sim --script mon_script.txt      # affiche chaque case du renard
```

Un script liste des plages d'images et des boutons (`100 102 S` = START des images 100 à 102)
et `shot 200` pour une capture. `--expect` vérifie que le jeu passe par ces états (`titre`,
`chapitre`, `monde`, `fondu`, `dialogue`, `fin`), ces cartes (`carte:…`), ces conversations
(`parle:…`, `dialogue:fin`) et ce nombre de répliques secrètes (`bonus:…`), dans l'ordre. Avec
`--auto`, un robot mène les conversations et l'horloge du script s'arrête pendant ce temps.
