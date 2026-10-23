# Zajka Quest 🦊 (ODROID-GO)

Une aventure en vue de dessus dans l'univers de [Zajka Friends](https://github.com/lstux/Slovingo-sk-fr-friends),
à la Zelda Game Boy ou Pokémon : le renard suit le récit du cours de Košice à Bratislava, explore
les lieux et parle aux personnages. C'est le pendant calme de [Zajka Run](../zajka-run/) : on peut
finir le jeu sans comprendre le slovaque, mais le comprendre rend le voyage plus riche.

> **État : les chapitres 1 (Košice) et 2 (Sucho) se jouent en entier**, soit les douze fiches
> des deux premières séries du cours, avec des quêtes, des objets à trouver et des mots à lire
> dans le monde. La partie est sauvegardée. Les graphismes sont provisoires.

## Ce qu'on fait dans le jeu

- **Parler** (A face à quelqu'un) : les épisodes du cours se jouent avec Andrea, les passants
  disent les « répliques en plus » du cours, d'autres personnages ont un rôle dans l'histoire.
- **Lire les panneaux** (A face à un panneau, un abri, une façade...) : le mot va au **Slovník**,
  le carnet de mots, avec sa traduction.
- **Trouver et donner des objets** : ils vont dans le **Batoh**, le sac (ticket, eau, gouttes
  d'eau, carte...). On en ramasse par terre en marchant dessus, on en reçoit, on en donne.
- **Avancer dans l'ordre** : chaque chapitre est une suite de petites quêtes ; quand on veut
  sauter une étape, Andrea retient le renard. Le menu **START** montre l'objectif (**Cieľ**).
- **Petits jeux de langue** : les choix de réplique (bonne phrase, morceaux à remettre dans
  l'ordre), sans pénalité, avec des répliques secrètes à gagner.

## Le chapitre 1 : Košice

| lieu | ce qui s'y passe |
|---|---|
| rue Hlavná | des panneaux à lire ; acheter un ticket à la trafika (choisir la bonne phrase) ; le conducteur ne laisse monter qu'avec un ticket ; deux passants |
| le tram | le Narrateur plante le décor ; Andrea : épisodes 1 « To si ty? » et 2 « Ako sa máš? » ; puis un revízor monte : il faut avoir composté son ticket (le composteur jaune près de la porte) ; ensuite les épisodes 3 à 5. À la fin du 5, on descend avec Andrea |
| chez sa tante, le soir | le Narrateur, l'épisode 6 « Dobrú noc », puis dormir sous le pommier (A face au tronc) : fin du chapitre |

## Le chapitre 2 : Sucho

| lieu | ce qui s'y passe |
|---|---|
| le jardin, le matin | l'épisode 1 « Je horúco » ; Andrea donne un peu d'eau, puis elle suit le renard partout |
| la rue | l'épisode 2 « Veľa alebo veľmi? » en chemin ; un thermomètre, l'école, une goutte d'eau par terre |
| le parc | au bord de l'étang presque à sec, l'épisode 3 « Chýba mi voda » ; les animaux ont soif : trouver cinq gouttes d'eau et les leur donner ; puis l'épisode 4 « Čo budeme robiť? » avec Andrea (le sac à dos et la carte) |
| le centre | la fontaine vide et le jardinier : l'épisode 5 « Fontána » |
| la gare | l'épisode 6 « Na stanici » : aider Andrea à écrire un mot à sa tante (morceaux à remettre dans l'ordre), les billets au guichet, le train. Fin du chapitre |

Le titre de chaque épisode s'affiche quand il commence. L'écran de fin compte les répliques
secrètes et les mots trouvés. L'histoire est écrite dans `src/game/chapter1.cpp` et
`chapter2.cpp` (outils communs : `story.cpp`). Les répliques propres au jeu (vendeur,
revízor, objectifs, animaux...) sont dans `story/lines.txt`, les objets dans `story/items.txt`.

## Sauvegarde

La partie s'enregistre toute seule dans la NVS de l'ESP32 (espace `zquest`), en passant une
porte et à la fin de chaque conversation : chapitre, carte, case et orientation du renard,
épisodes joués, étapes des quêtes, objets du sac, objets déjà ramassés, mots trouvés,
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
  dans `story/bonus.txt`, **à faire relire par un Slovaque**, comme `story/lines.txt`.

## Décisions

| sujet | choix |
|---|---|
| contenu du cours | en flash pour le prototype, sur la carte SD ensuite (même format de données) |
| sauvegarde | NVS |
| langue de l'interface | slovaque, comme Zajka Run |
| récit | les épisodes dans l'ordre, entre lesquels s'intercalent des quêtes |
| bonus de compréhension | répliques secrètes et objets |
| code commun avec Run | copié pour l'instant, factorisé plus tard dans `esp32/zajka-common/` |
| personnages | 16 × 24 en vue de dessus, portraits 48 × 48 dans les dialogues |

## Commandes

| bouton | action |
|---|---|
| croix | se déplacer (une pression brève tourne le renard sans avancer) |
| A | parler, lire un panneau, valider |
| B | courir (tenu) ; fermer un menu |
| SELECT | traduction française (répliques, messages, objectif) |
| START | menu : Batoh (sac), Slovník (mots), Cieľ (objectif) ; gauche / droite pour changer |
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
src/game/story.cpp    outils communs de l'histoire (épisodes, objets, panneaux, passages imposés)
src/game/chapter*.cpp l'histoire de chaque chapitre : quêtes, qui joue quel épisode, où
src/game/course.h     structures du contenu du cours (répliques, mots, exercices)
src/game/gen/         données générées (images, polices, cours) : ne pas éditer
maps/*.txt            cartes en texte (légende en tête de chaque fichier)
story/bonus.txt       répliques secrètes (bonus de compréhension), propres au jeu
story/lines.txt       les autres répliques propres au jeu, et les objectifs du menu
story/items.txt       les objets du sac (slovaque, français) ; icônes dans art/items/
art/tiles/            planche de tuiles 16 × 16 (ordre : TILESET dans tools/build_assets.py)
art/walk/             planches de marche 16 × 24 (lignes bas, haut, gauche, droite)
art/portraits/        portraits 48 × 48 de la boîte de dialogue
art/sprites/          le renard et Andrea de l'écran titre (planches Gemini de Zajka Run)
art/PROMPTS.md        les prompts Gemini / ChatGPT de toutes les images, et comment les importer
tools/                générateurs (Python 3 + Pillow, numpy) ; build_assets.py régénère src/game/gen/
sim/                  simulateur PC : même code de jeu, captures PPM et son WAV
```

Tuiles, planches de marche et portraits sont pour l'instant dessinés par
`tools/make_placeholder_art.py` (les portraits viennent des têtes SVG de Slovingo). Pour les
remplacer par des images générées : `art/PROMPTS.md` donne un prompt complet par image et la
commande `tools/import_ai.py` qui l'importe (`tileset`, `walk`, `portrait`).

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
l'en-tête : `start` (départ), `warp` (porte), `npc` (personnage ; `npc human down say 2 0-1`
lui fait dire les « répliques en plus » 0 et 1 de l'épisode 2 ; `id vodic` lui donne un nom pour
l'histoire), `sign mot` (panneau : le mot va au Slovník, il doit être dans le vocabulaire du
cours), `item kvapka` (objet à ramasser) ou `zone nom` (case qui déclenche l'histoire). Les
repères s'écrivent avec un chiffre ou `! $ % & + ; < > ? ~ ' ( ) [ ] { } / @`. `night: 1` affiche
la carte de nuit. `build_assets.py` vérifie
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
    --expect objet:lístok,carte:kosice_tram,carte:kosice_teta,bonus:9,fin,chapitre   # chapitre 1
build/sim --script sim/resume.txt --auto --save /tmp/zq.sav --expect reprise,carte:sucho_zahrada
build/sim --chapter 2 --script sim/chapter2.txt --auto --expect objet:kvapkax5,bonus:7,fin  # chapitre 2
build/sim --script sim/chapter.txt --auto-peek ...   # ouvre la traduction à chaque choix : pas de bonus
build/sim --script sim/chapter.txt --auto-wrong ...  # se trompe une fois à chaque choix : pas de bonus
ZQ_TRACE=1 build/sim --script mon_script.txt      # affiche chaque case du renard
```

Un script liste des plages d'images et des boutons (`100 102 S` = START des images 100 à 102)
et `shot 200` pour une capture. `--expect` vérifie que le jeu passe par ces états (`titre`,
`chapitre`, `monde`, `fondu`, `dialogue`, `fin`), ces cartes (`carte:…`), ces conversations
(`parle:…`, `dialogue:fin`), ce nombre de répliques secrètes (`bonus:…`), de mots
(`mots:…`) et ces objets (`objet:kvapkax5`), dans l'ordre. Avec
`--auto`, un robot mène les conversations et l'horloge du script s'arrête pendant ce temps.
