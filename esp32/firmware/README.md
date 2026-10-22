# Firmware Slovingo (LilyGo T-HMI)

Slovingo sur la LilyGo T-HMI : ESP32-S3R8 (16 Mo de flash, 8 Mo de PSRAM), écran 2,8" 320×240,
tactile résistif (XPT2046), carte microSD.

L'interface est **celle de la montre** : le code de `../twatch-app/slovingo` (LVGL 7.11) est compilé tel quel,
sans copie. `src/compat/` remplace les quelques fonctions du firmware de la montre qu'il utilise
(tuile, boutons de pied de page, clavier, Wi-Fi, téléchargement, réglages JSON). Ce qui change :

| Sur la montre | Sur la T-HMI |
|---|---|
| SPIFFS | carte SD (`SPIFFS` est un autre nom de `SD_MMC`, voir `include/SPIFFS.h`) |
| `wificfg.json` dans la SPIFFS | `/wificfg.json` sur la SD, même format (`networklist` : `ssid` / `psk`) |
| `slovingo.json` dans la SPIFFS | `/slovingo.json` sur la SD, mêmes clés (voir `../twatch-app/conf/`) |
| retour haptique | aucun (pas de moteur) |
| flèche de retour de l'écran d'accueil | éteint l'appareil (voir ci-dessous) |

## Interface grand écran

`-D SLV_BIG_SCREEN=1` (dans `platformio.ini`, absent du build de la montre) active dans le code de l'app :
marges et boutons plus grands aux coins arrondis et dégradés, question du quiz dans une carte avec barre
de progression, barre de progression dans la lecture, pages de lecture plus longues (texte 440 caractères,
dialogue 5 répliques, tableau 5 lignes, voir `SLV_RD_*` en tête de `slovingo_app_main.cpp`) et un
glissement de 220 ms quand une page ou un écran change (swipe, flèches, boutons, question suivante).
Elle ajoute aussi l'exercice **« relier »** (type `match` du cours) : deux colonnes, on touche un mot puis sa
traduction ; la paire juste devient verte, une erreur clignote en rouge, et le point est pour une série sans erreur.
Retirer le flag redonne exactement l'interface de la montre (qui ne joue pas les `match`).

**Illustrations** : si l'API du cours contient des blocs `img` (générés par `src/export_api.py` à partir des images
locales `img/` du cours, voir `docs/`), la T-HMI les télécharge avec la fiche dans le cache de la carte SD, les décode
(JPEG baseline, bibliothèque TJpg_Decoder) et les affiche sur une page de lecture avec leur légende. Sans Wi-Fi
elle utilise le cache ; une image absente ou illisible est simplement omise. La montre ignore ces blocs.

## Compiler

```bash
pip install platformio
cd esp32/firmware
pio run                      # le fichier est .pio/build/t-hmi/firmware.bin
pio run -t upload            # flasher en USB
pio device monitor           # logs, 115200 bauds
```

Le build prend le code de l'app dans `../twatch-app/slovingo` (`scripts/add_app.py`) : il faut donc
compiler depuis un clone complet du dépôt.

## Lancer depuis le Launcher

Copier `firmware.bin` sur la carte SD (FAT32, SDHC), puis l'installer depuis le menu SD du
[Launcher](https://github.com/bmorcelli/Launcher) (voir sa documentation pour l'installation et
l'écran de sélection).

## Préparer la carte SD

Racine de la carte, mêmes fichiers que sur la montre :

```
/wificfg.json     "networklist": [ { "ssid": "...", "psk": "..." } ]   (et "tz" : fuseau POSIX, défaut Europe/Paris)
/slovingo.json    "url", "chosen", "questions", "name"                   (facultatif : l'app le crée)
```

L'app y écrit aussi le cache des cours (`/slv*.json`), la progression et la série de jours, et la
calibration du tactile (`/touch_cal.json`). Sans carte SD elle fonctionne, mais sans cache,
sans progression ni réglages enregistrés.

## Au premier démarrage

- **Calibration du tactile** : quatre croix s'affichent, touche-les (au stylet, c'est plus précis).
  Le résultat est enregistré dans `/touch_cal.json`. Pour recommencer : maintenir le bouton OK
  (GPIO 21) pendant la mise sous tension.
- **Image à l'envers** : passer `BOARD_ROTATION` de 1 à 3 dans `src/board.h`, recompiler, puis refaire
  la calibration (bouton OK maintenu au démarrage), l'ancienne ne vaut plus.
- **Éteindre** : la flèche de retour de l'écran d'accueil coupe l'alimentation sur batterie ; sur USB
  la carte se met en sommeil profond et le bouton OK la réveille.

## Diagnostic : écran noir, moniteur muet

Deux outils, à utiliser dans cet ordre.

**1. Le programme de diagnostic seul** (sans LVGL ni app) : écran, port série, SD, batterie, boutons.

```bash
pio run -e t-hmi-diag -t upload
pio device monitor -e t-hmi-diag
```

Il allume le rétroéclairage, remplit l'écran en rouge, vert puis bleu, puis affiche l'état de la SD, de
la batterie et du tactile (`IRQ=0` quand on touche l'écran, `OK=0` / `BOOT=0` quand on appuie sur un bouton).

- Les trois couleurs s'affichent : la carte, le flash et la config de l'écran sont bons, le problème est dans l'app.
- Rétroéclairage allumé mais écran blanc ou brouillé : le programme tourne, l'écran n'est pas piloté
  (broches, `TFT_eSPI`, `memory_type`).
- Carte entièrement éteinte et rien sur le port série : le programme ne démarre pas (mauvais mode de flash,
  mauvaise adresse, carte restée en mode téléchargement : appuyer sur RST ou débrancher/rebrancher l'USB).

**2. Le firmware normal, avec le diagnostic de démarrage** (`-D SLV_DIAG=1` dans `platformio.ini`, mettre `0`
pour le retirer). Au démarrage il écrit sur le port série une bannière (puce, flash, PSRAM, raison du dernier
reset) puis chaque étape : `board_init`, `sd_mount`, `display_init`, `touch_init`, `lvgl_port_init`, `app_setup`,
`app_activate`, `setup_done`, puis une ligne toutes les 5 s dans `loop`. Le dernier jalon franchi est gardé en
mémoire RTC, qui survit à un plantage : après un reset anormal, la bannière **et l'écran** (5 s, seulement dans ce cas ; le test rouge-vert-bleu reste dans `t-hmi-diag`) disent
à quelle étape le programme s'est arrêté.

Le port USB de la carte disparaît et réapparaît à chaque redémarrage : si le moniteur reste vide, le relancer
après avoir appuyé sur RST. Le firmware attend jusqu'à 2,5 s qu'il soit ouvert avant d'écrire.

## État du code

Écrit d'après le dépôt officiel LilyGo (brochage, écran, tactile, SD). Vérifications faites sur un
ordinateur : le code de l'app et `src/compat/` compilent avec le vrai LVGL 7.11 et le vrai ArduinoJson 6.21 ;
les fichiers qui parlent au matériel (`board.cpp`, `touch.cpp`, `lvgl_port.cpp`, `main.cpp`, `diag*.cpp`) passent un
contrôle de syntaxe, mais contre des en-têtes Arduino factices écrits par moi, ce qui ne prouve rien sur l'API
réelle. `wifictl.cpp` et `uri_load.cpp` (API Wi-Fi d'Arduino-ESP32) n'ont même pas passé ce contrôle. Le build
PlatformIO a ensuite été corrigé et compile chez l'utilisateur ; au premier essai sur la carte,
l'écran restait noir (voir Diagnostic ci-dessus). Points à surveiller :

- le téléchargement HTTPS ne vérifie pas le certificat du serveur (`setInsecure()`), les cours étant publics ;
- la tâche de téléchargement de l'app a 12 Ko de pile : à surveiller pendant la poignée de main TLS ;
- la calibration du tactile fait ses propres calculs (pas ceux de la bibliothèque LilyGo) ;
- le comportement exact de l'installation depuis le Launcher.
