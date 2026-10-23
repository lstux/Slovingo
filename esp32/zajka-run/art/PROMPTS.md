# Graphismes de Zajka Run : prompts pour Gemini / ChatGPT

Chaque image du jeu peut être remplacée par une image générée, sans toucher au code. Les prompts ci-dessous sont **complets** : un bloc = un copier-coller.

## État

| fichier | état | source |
| --- | --- | --- |
| `kosice/far.png` fond lointain | fait (Gemini) | `sources/kosice_far_gemini.jpg` |
| `kosice/mid.png` rue Hlavná, Dóm | fait (Gemini) | `sources/kosice_mid_gemini.jpg` |
| `kosice/tiles.png` tuiles | fait (Gemini, cases réaffectées) | `sources/kosice_tiles_gemini.jpg` |
| `objects/medovnik` | fait (Gemini) | `sources/medovnik_gemini.jpg` |
| `objects/holub` pigeon | fait (Gemini) | `sources/holub_gemini.jpg` |
| `objects/osa` guêpe | fait (Gemini) | `sources/osa_gemini.jpg` |
| `sprites/fox` renard (cases 48×48) | fait (Gemini) | `sources/fox_<anim>_gemini.jpg` |
| `sprites/rabbit` Andrea | fait (Gemini : idle, win, run sur 4 images) | `sources/rabbit_<anim>_gemini.jpg` |
| `objects/pruzina`, `lampa`, `zastavka`, `elektricka`, `lavicka`, `fx` | fait (Gemini) | `sources/<nom>_gemini.jpg` |
| `objects/hud` icônes 8×8 | garder (trop petit pour une IA) | dessiné à la main |

## Marche à suivre

1. Coller le prompt dans Gemini. Pour les personnages, **joindre l'image de référence** indiquée.
2. Enregistrer l'image dans `art/sources/` (pour pouvoir la réimporter plus tard).
3. L'importer avec la commande donnée sous le prompt : l'outil détoure le magenta, découpe les images d'animation, réduit à la bonne taille et à une palette courte.
4. `python3 tools/build_assets.py`, puis vérifier dans le simulateur : `make -C sim && build/sim --bot --every 120 --out /tmp/zr`.

Si l'IA ajoute un trait de sol sous un objet : `--strip-ground`. Si les images n'ont pas la même taille exprès (ressort comprimé, petit nuage) : `--uniform`. Si elle mélange l'ordre des tuiles : `--cells` (voir la réimportation des tuiles en bas de page).

## Le renard (joindre `art/characters/fox-reference.png`)

Une ligne d'animation par prompt : les résultats sont plus fidèles et ça ménage le quota.

### Renard : run (6 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. He is an ANTHROPOMORPHIC fox teenager who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Orange fur, dark grey ear tips, cream muzzle, grey cheeks, big bushy orange tail with a cream tip, wearing a royal blue hoodie, yellow backpack straps, navy blue trousers and white sneakers.
Sprite sheet for a side-view platform game, the character in side view facing right: a 6-frame RUNNING cycle (arms swinging, legs alternating, tail flowing behind), all frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. The character is about 1.5 times taller than wide and each frame must read clearly at 32x48 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow..
```

`python3 tools/import_ai.py sprite fox run art/sources/fox_run.png --frames 6`

### Renard : idle (4 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. He is an ANTHROPOMORPHIC fox teenager who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Orange fur, dark grey ear tips, cream muzzle, grey cheeks, big bushy orange tail with a cream tip, wearing a royal blue hoodie, yellow backpack straps, navy blue trousers and white sneakers.
Sprite sheet for a side-view platform game, the character in side view facing right: a 4-frame IDLE animation (standing, gentle breathing, tail swaying), all frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. The character is about 1.5 times taller than wide and each frame must read clearly at 32x48 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow..
```

`python3 tools/import_ai.py sprite fox idle art/sources/fox_idle.png --frames 4`

### Renard : jump (1 image)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. He is an ANTHROPOMORPHIC fox teenager who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Orange fur, dark grey ear tips, cream muzzle, grey cheeks, big bushy orange tail with a cream tip, wearing a royal blue hoodie, yellow backpack straps, navy blue trousers and white sneakers.
Sprite sheet for a side-view platform game, the character in side view facing right: a single JUMP frame (rising, one knee up, both arms raised). Only one frame, all frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. The character is about 1.5 times taller than wide and each frame must read clearly at 32x48 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow..
```

`python3 tools/import_ai.py sprite fox jump art/sources/fox_jump.png --frames 1`

### Renard : fall (1 image)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. He is an ANTHROPOMORPHIC fox teenager who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Orange fur, dark grey ear tips, cream muzzle, grey cheeks, big bushy orange tail with a cream tip, wearing a royal blue hoodie, yellow backpack straps, navy blue trousers and white sneakers.
Sprite sheet for a side-view platform game, the character in side view facing right: a single FALL frame (arms spread out for balance, legs slightly apart). Only one frame, all frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. The character is about 1.5 times taller than wide and each frame must read clearly at 32x48 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow..
```

`python3 tools/import_ai.py sprite fox fall art/sources/fox_fall.png --frames 1`

### Renard : hurt (1 image)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. He is an ANTHROPOMORPHIC fox teenager who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Orange fur, dark grey ear tips, cream muzzle, grey cheeks, big bushy orange tail with a cream tip, wearing a royal blue hoodie, yellow backpack straps, navy blue trousers and white sneakers.
Sprite sheet for a side-view platform game, the character in side view facing right: a single HURT frame (knocked backwards, surprised face, eyes closed). Only one frame, all frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. The character is about 1.5 times taller than wide and each frame must read clearly at 32x48 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow..
```

`python3 tools/import_ai.py sprite fox hurt art/sources/fox_hurt.png --frames 1`

### Renard : win (2 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. He is an ANTHROPOMORPHIC fox teenager who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Orange fur, dark grey ear tips, cream muzzle, grey cheeks, big bushy orange tail with a cream tip, wearing a royal blue hoodie, yellow backpack straps, navy blue trousers and white sneakers.
Sprite sheet for a side-view platform game, the character in side view facing right: a 2-frame VICTORY animation (waving one arm high, big smile), all frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. The character is about 1.5 times taller than wide and each frame must read clearly at 32x48 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow..
```

`python3 tools/import_ai.py sprite fox win art/sources/fox_win.png --frames 2`

## Andrea (joindre `art/characters/andrea-reference.png`)

Andrea n'est pas la pilote de MathPulse : la référence la montre en tenue d'été (`rabbit-full.svg`). Elle n'a besoin que de trois lignes : `idle` (à l'arrêt), `win` (arrivée), `run` (écran titre).

### Andrea : idle (4 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. She is Andrea, an ANTHROPOMORPHIC rabbit teenage girl from Košice who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Grey fur, long upright ears (pink inside), white blaze widening into a white muzzle, dark eyes, small Y-shaped nose, grey whiskers, two front teeth. Summer outfit: a mint green t-shirt, blue denim shorts, red sneakers with white soles, a small yellow bag on a shoulder strap.
Sprite sheet for a side-view platform game, the character in side view facing right: a 4-frame IDLE animation (waiting at a tram stop, looking around, one ear twitching), all frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. The character is about 1.5 times taller than wide and each frame must read clearly at 32x48 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow..
```

`python3 tools/import_ai.py sprite rabbit idle art/sources/andrea_idle.png --frames 4`

### Andrea : win (2 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. She is Andrea, an ANTHROPOMORPHIC rabbit teenage girl from Košice who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Grey fur, long upright ears (pink inside), white blaze widening into a white muzzle, dark eyes, small Y-shaped nose, grey whiskers, two front teeth. Summer outfit: a mint green t-shirt, blue denim shorts, red sneakers with white soles, a small yellow bag on a shoulder strap.
Sprite sheet for a side-view platform game, the character in side view facing right: a 2-frame VICTORY animation (waving one arm high, big smile), all frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. The character is about 1.5 times taller than wide and each frame must read clearly at 32x48 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow..
```

`python3 tools/import_ai.py sprite rabbit win art/sources/andrea_win.png --frames 2`

### Andrea : run (6 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. She is Andrea, an ANTHROPOMORPHIC rabbit teenage girl from Košice who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Grey fur, long upright ears (pink inside), white blaze widening into a white muzzle, dark eyes, small Y-shaped nose, grey whiskers, two front teeth. Summer outfit: a mint green t-shirt, blue denim shorts, red sneakers with white soles, a small yellow bag on a shoulder strap.
Sprite sheet for a side-view platform game, the character in side view facing right: a 6-frame RUNNING cycle (arms swinging, legs alternating, ears bouncing), all frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. The character is about 1.5 times taller than wide and each frame must read clearly at 32x48 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow..
```

`python3 tools/import_ai.py sprite rabbit run art/sources/andrea_run.png --frames 6`

## Déjà faits (pour régénérer si besoin)

### Ressort (`objects/pruzina_16x16`, 2 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 24 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A spring pad for a side-view platform game, seen exactly from the side: a red bouncy top plate on a silver metal coil spring standing on a dark grey base. 2 frames in a single horizontal row, evenly spaced with wide gaps: relaxed (spring extended), then compressed (spring squashed, plate low). Same base position and scale. Each frame must read clearly at 16x16 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

`python3 tools/import_ai.py object pruzina art/sources/pruzina_gemini.jpg --frames 2 --uniform`

### Lampadaire (point de passage) (`objects/lampa_16x48`, 2 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A historic cast-iron street lamp from an old Central European town centre, dark green-black painted iron, a slender fluted post on a wide base, a four-sided glass lantern on top with a small crown. Seen exactly from the side, tall and thin (proportions 1 wide by 3 tall). 2 frames in a single horizontal row, evenly spaced with wide gaps: unlit (grey glass), then lit (warm yellow glass with a few glow pixels around it). Same scale, base on the same baseline. Each frame must read clearly at 16x48 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

`python3 tools/import_ai.py object lampa art/sources/lampa_gemini.jpg --frames 2 --strip-ground --uniform`

### Arrêt de tram (`objects/zastavka_16x48`, 1 image)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 24 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A tram stop pole in a Slovak city, seen exactly from the side: a thin grey metal pole, at the top a round yellow sign with a dark blue rim showing a simple tram pictogram, and lower down a small white timetable panel with grey lines. No letters on the signs. Tall and thin (proportions 1 wide by 3 tall). A single image, centred. It must read clearly at 16x48 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

`python3 tools/import_ai.py object zastavka art/sources/zastavka_gemini.jpg`

### Tram (`objects/elektricka_96x48`, 1 image)

Si le toit n'est plus à la même hauteur dans l'image, ajuster `TRAM_ROOF` en tête de `src/game/game.cpp` (ligne du toit, sous le pantographe).

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A classic Czechoslovak Tatra T3 tram, seen exactly from the side (pure side elevation, no perspective), facing right: rounded front and rear ends, cream upper body, red lower band and skirt, a row of dark blue windows with small highlights, two sets of double doors, two bogies with dark wheels, a diamond pantograph raised on the roof. No numbers, no logos. Proportions exactly 2 wide by 1 tall, including the pantograph. A single image, centred. It must read clearly at 96x48 pixels. Flat solid magenta #FF00FF background everywhere, no rails, no ground line, no shadow.
```

`python3 tools/import_ai.py object elektricka art/sources/elektricka_gemini.jpg`

### Banc et bac à fleurs (`objects/lavicka_32x16`, 2 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Two street decoration props for a side-view platform game, seen exactly from the side, in a single horizontal row with a wide gap between them: first a wooden park bench with brown slats and black cast-iron legs; second a long wooden flower box filled with green leaves and small red, yellow and white flowers. Both the same height and on the same baseline, each twice as wide as tall. Each must read clearly at 32x16 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

`python3 tools/import_ai.py object lavicka art/sources/lavicka_gemini.jpg --frames 2`

### Effets (poussière, étincelle) (`objects/fx_16x16`, 5 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 12 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Small visual effects for a platform game, 5 frames in a single horizontal row, evenly spaced with wide gaps, all the same size: frames 1 to 3 are a small off-white dust puff growing and fading (small round cloud, bigger cloud, thin broken ring); frames 4 and 5 are a pale yellow four-pointed sparkle star, small then large. Each frame must read clearly at 16x16 pixels. Flat solid magenta #FF00FF background everywhere.
```

`python3 tools/import_ai.py object fx art/sources/fx_gemini.jpg --frames 5 --uniform`


### Fond lointain

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 48 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Wide side-scrolling game background, aspect ratio 2:1. Clear blue summer sky with a few puffy white clouds, fading lighter near the horizon. Far in the distance, low green wooded hills of eastern Slovakia, and pale blue-grey silhouettes of 1970s concrete housing blocks (Slovak "sídlisko") on the outskirts of Košice. Everything sits in the lower third of the image; the upper two thirds are sky. Atmospheric perspective: distant elements are pale and low-contrast.
```

`python3 tools/import_ai.py far art/sources/kosice_far_gemini.jpg --mirror`

### Rue Hlavná et le Dóm

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 64 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Side-scrolling game background layer, very wide panorama (aspect ratio about 4:1). The main pedestrian street of Košice, Slovakia ("Hlavná ulica"), seen straight from the side at street level, the bottom edge of the image is the pavement line. A row of pastel baroque and classicist burgher houses (yellow, salmon pink, mint green, cream, light blue) with red clay tile roofs, white window frames and arched gateways, linden trees in front. In the right half: St. Elisabeth Cathedral (Dóm svätej Alžbety), a large Gothic cathedral of grey stone with flying buttresses, tall pointed windows, a steep roof covered in glazed tiles forming a green, yellow and red diamond pattern, a tall north tower topped by a dark baroque spire with golden details. Next to it the slender Urban Tower (Urbanova veža). In the left half: the neo-baroque State Theatre with its green copper dome. Everything above the rooftops and spires is flat solid magenta #FF00FF.
```

`python3 tools/import_ai.py mid art/sources/kosice_mid_gemini.jpg`

### Medovník

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A collectible item for a platform game: a Slovak gingerbread heart (medovník), red-brown dough with white icing scallops along the edge and tiny green and yellow sugar dots. 4-frame coin-spin animation in a single horizontal row, evenly spaced with wide gaps: front view, three-quarter view, edge-on (thin sliver), three-quarter view from the other side. All frames the same size and centred on the same line. Each frame must read clearly at 16x16 pixels. Flat solid magenta #FF00FF background everywhere, no shadow.
```

`python3 tools/import_ai.py object medovnik art/sources/medovnik_gemini.jpg --frames 4`

### Pigeon

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A cute enemy for a side-view platform game: a plump grey city pigeon with an iridescent green-purple neck, orange eye and big bright pink feet, seen exactly from the side, facing right. 3 frames in a single horizontal row, evenly spaced with wide gaps: walking frame 1, walking frame 2 (other foot forward, head bobbing), then a third frame where the pigeon is squashed flat with dizzy cross eyes. Same scale, feet on the same baseline. Each frame must read clearly at 24x16 pixels (wider than tall). Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

`python3 tools/import_ai.py object holub art/sources/holub_gemini.jpg --frames 3 --strip-ground`

### Guêpe

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 24 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A small flying enemy for a side-view platform game: a cartoon wasp with yellow and black stripes, a black head with a white eye highlight, a small stinger, seen from the side and facing right. 2 frames of wing flapping in a single horizontal row, evenly spaced with wide gaps: wings up, wings down. Same scale. Each frame must read clearly at 16x16 pixels. Flat solid magenta #FF00FF background everywhere, no shadow.
```

`python3 tools/import_ai.py object osa art/sources/osa_gemini.jpg --frames 2`

### Réimporter les personnages

Gemini rend souvent les planches en grille et change les proportions d'une génération à
l'autre : on cale le renard sur la taille de sa tête (jusqu'au haut du sweat bleu), Andrea
sur sa hauteur debout, et la tête toujours au même x pour éviter les sautillements.

```
H=17.5; X=27
python3 tools/import_ai.py sprite fox idle art/sources/fox_idle_gemini.jpg --grid 2x2 --head-px $H --anchor-x $X
python3 tools/import_ai.py sprite fox run  art/sources/fox_run_gemini.jpg  --grid 3x2 --head-px $H --anchor-x $X
python3 tools/import_ai.py sprite fox jump art/sources/fox_jump_gemini.jpg --head-px $H --anchor-x $X
python3 tools/import_ai.py sprite fox fall art/sources/fox_fall_gemini.jpg --head-px $H --anchor-x $X
python3 tools/import_ai.py sprite fox hurt art/sources/fox_hurt_gemini.jpg --head-px $H --anchor-x $X
python3 tools/import_ai.py sprite fox win  art/sources/fox_win_gemini.jpg  --grid 3x2 --pick 1,2 --head-px $H --anchor-x $X
python3 tools/import_ai.py sprite rabbit idle art/sources/rabbit_idle_gemini.jpg --grid 1x4 --height-px 46 --anchor-x 16
python3 tools/import_ai.py sprite rabbit win  art/sources/rabbit_win_gemini.jpg  --grid 1x4 --pick 1,2 --height-px 46 --anchor-x 16
python3 tools/import_ai.py sprite rabbit run  art/sources/rabbit_run_gemini.jpg  --grid 1x4 --height-px 46 --anchor-x 16
```

Le saut du renard est plus petit que les autres poses : Gemini l'a dessiné avec de très
longues jambes, il a fallu le réduire pour qu'il tienne dans la case. Pour le refaire, demander
*a compact JUMP pose, knees tucked up, same proportions as the reference*.

### Tuiles

Gemini n'a pas respecté l'ordre de la grille : on réaffecte les cases (ligne, colonne de son image) aux tuiles du jeu. `L` / `R` ajoutent un bord gauche / droit, `f` retourne la case.

```
python3 tools/import_ai.py tiles art/sources/kosice_tiles_gemini.jpg \
  --cells "1=0,1 2=0,2 3=0,1,L 4=0,1,R 5=0,5,f 6=0,5 7=0,7 8=1,0 9=1,1 10=2,2 11=1,6 12=2,5 13=3,0"
```

Tuiles du jeu : 1 dessus du sol (pavés), 2 sol plein, 3/4 dessus bord gauche/droit, 5/6 plein bord gauche/droit, 7 bloc de pierre, 8 caisse bonus, 9 caisse vide, 10/11/12 corniche gauche/milieu/droite, 13 pointes.

## Niveaux suivants

Chaque série du cours peut devenir un monde avec ses propres `far.png` / `mid.png` / `tiles.png` : Slovenský raj (gorges, échelles), Vysoké Tatry (Štrbské Pleso, orage), Liptov (crue), Banská Štiavnica (mine), Bratislava (château, Danube).
