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

## État du code

Écrit d'après le dépôt officiel LilyGo (brochage, écran, tactile, SD), **jamais compilé avec PlatformIO
ni essayé sur la carte**. Seule vérification faite : compilation de contrôle du code de l'app et de
`src/compat/` sur un ordinateur, avec LVGL 7.11 et ArduinoJson 6.21 réels et des en-têtes Arduino
factices. Les fichiers qui parlent directement au matériel (`board.cpp`, `touch.cpp`, `lvgl_port.cpp`,
`main.cpp`, `src/compat/hardware/wifictl.cpp`, `src/compat/utils/uri_load/uri_load.cpp`) n'ont donc
pas été compilés. Points à surveiller au premier essai :

- le téléchargement HTTPS ne vérifie pas le certificat du serveur (`setInsecure()`), les cours étant publics ;
- la tâche de téléchargement de l'app a 12 Ko de pile : à surveiller pendant la poignée de main TLS ;
- la calibration du tactile fait ses propres calculs (pas ceux de la bibliothèque LilyGo) ;
- le comportement exact de l'installation depuis le Launcher.
