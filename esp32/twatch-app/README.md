# App Slovingo pour la T-Watch 2020 v2

Ce dossier ne contient que le code de l'app. Elle s'ajoute au firmware d'origine
[d03n3rfr1tz3/TTGO.T-Watch.2020](https://github.com/d03n3rfr1tz3/TTGO.T-Watch.2020)
(le dépôt dont `lstux/T-Watch-2020` est le fork).

```
twatch-app/
  slovingo/                  le code de l'app (-> src/app/slovingo du firmware)
  conf/slovingo.json.example modèle de réglages
  install.sh                 installe l'app dans un clone du firmware
```

Le descriptif de l'app (fonctions, format des données, progression) est dans [`slovingo/README.md`](slovingo/README.md).

## Compiler le firmware avec l'app

Environnement PlatformIO : `t-watch2020-v2` (celui par défaut du projet est `t-watch2020-v1` :
mets toujours `-e`).

### 1. Récupérer le firmware d'origine et y ajouter l'app

```bash
pip install platformio            # ou l'extension PlatformIO de VS Code
git clone https://github.com/d03n3rfr1tz3/TTGO.T-Watch.2020.git
cd TTGO.T-Watch.2020
../Slovingo/esp32/twatch-app/install.sh .     # adapte le chemin vers ce dossier
```

`install.sh` fait quatre choses, il peut être relancé sans risque :

1. copie `slovingo/` dans `src/app/slovingo` ;
2. copie `conf/slovingo.json.example` dans `conf/` ;
3. ajoute `"app/slovingo",` à la liste `NOT_ESSENTIAL` de `support/apps_off.py`
   (les cartes à petite flash ne prennent pas l'app) ;
4. ajoute `data/*.json` à `.gitignore`.

Aucun autre fichier du firmware n'est modifié : l'app est détectée toute seule au build.
À la main, c'est donc : copier le dossier `slovingo/` dans `src/app/`, et c'est tout.

### 2. Réglages

Les réglages sont des fichiers JSON à la racine de la mémoire de la montre (SPIFFS). Les modèles
sont dans `conf/`, on les copie dans `data/` sans `.example` :

```bash
mkdir -p data
cp conf/slovingo.json.example data/slovingo.json
cp conf/wificfg.json.example  data/wificfg.json
```

| Fichier | Ce qu'on y règle |
|---|---|
| `slovingo.json` | `url` : dossier `api/` du cours (défaut `https://lstux.github.io/Slovingo/sk-fr-kids/api/`), `questions` : questions par série, `name` : prénom affiché à la place de `[USER_NAME]` (défaut `Líška`) |
| `wificfg.json` | `networklist` : `ssid` et `psk` de tes réseaux, `hostname`, `ftpserver` / `ftpuser` / `ftppass`, etc. |

Le Wi-Fi et l'URL du cours peuvent aussi se régler sur la montre (réglages Wi-Fi, bouton réglages de l'app).
`data/*.json` contient tes mots de passe : ne le commite pas.

### 3. Build et upload

```bash
pio run -e t-watch2020-v2                    # build seulement
pio run -e t-watch2020-v2 -t upload          # flasher le firmware (montre en USB)
pio run -e t-watch2020-v2 -t uploadfs        # envoyer le contenu de data/ sur la montre
pio device monitor                           # logs, 115200 bauds
```

- Port non détecté : ajouter `--upload-port /dev/ttyUSB0` (ou `COM3` sous Windows).
- `uploadfs` réécrit **toute** la mémoire SPIFFS : réglages faits sur la montre et cache
  Slovingo sont effacés. Mets tous tes fichiers dans `data/` avant de le lancer.
- Sans reflasher : `ftpserver: true` dans `wificfg.json`, puis
  `curl -T data/slovingo.json ftp://<ftpuser>:<ftppass>@<ip-de-la-montre>/slovingo.json`.

### 4. Sauvegarde avant le premier flash (conseillé)

```bash
python ~/.platformio/packages/tool-esptoolpy/esptool.py --port <PORT> --baud 921600 read_flash 0 0x1000000 sauvegarde-twatch.bin
# restaurer : même commande avec write_flash 0 sauvegarde-twatch.bin
```

### 5. Vérifier

```bash
curl -s https://lstux.github.io/Slovingo/sk-fr-kids/api/index.json | head -c 200
```

Puis sur la montre : Wi-Fi activé, app **slovingo** (groupe Général). Un cours déjà ouvert
est gardé en cache et marche sans Wi-Fi.

## Régénérer polices et images

`slovingo/tools/gen_assets.py` (Pillow et DejaVu Sans requis) regénère les polices, l'icône,
les mascottes (renard, lapin) et les pictogrammes de thèmes dans `slovingo/font/` et `slovingo/images/`.
Les aperçus PNG qu'il produit ne sont pas versionnés ici.
