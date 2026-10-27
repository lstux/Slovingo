# Graphismes de Zajka Run : prompts pour Gemini / ChatGPT

Chaque image du jeu peut être remplacée par une image générée, sans toucher au code. Les prompts ci-dessous sont **complets** : un bloc = un copier-coller.

## État

| fichier | état | source |
| --- | --- | --- |
| `kosice/far.png` fond lointain | fait (Gemini) | `sources/kosice_far_gemini.jpg` |
| `kosice/mid.png` rue Hlavná, Dóm | fait (Gemini) | `sources/kosice_mid_gemini.jpg` |
| `kosice/tiles.png` tuiles | fait (Gemini, cases réaffectées) | `sources/kosice_tiles_gemini.jpg` |
| `objects/medovnik` | fait (Gemini) | `sources/medovnik_gemini.jpg` |
| `objects/holub` pigeon 48×32 | fait (Gemini, import avec `--mag-cut 140 --tint-cut 100` pour garder les pattes roses) | `sources/holub_big_gemini.jpg` |
| `objects/osa` guêpe 32×32 | fait (Gemini) | `sources/osa_big_gemini.jpg` |
| `sprites/fox` renard (cases 48×48) | fait (Gemini) | `sources/fox_<anim>_gemini.jpg` |
| `sprites/rabbit` Andrea | fait en 48×48 (Gemini) ; vol plané importé (`sources/andrea_glide.jpg`) | `sources/rabbit_<anim>_gemini.jpg` |
| `objects/pruzina`, `lampa`, `zastavka`, `elektricka`, `lavicka`, `fx` | fait (Gemini) | `sources/<nom>_gemini.jpg` |
| `objects/hud` icônes 8×8 | garder (trop petit pour une IA) | dessiné à la main |
| `sucho/far`, `sucho/mid`, `sucho/tiles` (niveau 1-2) | fait (Gemini) | `sources/sucho_*_gemini.jpg` |
| `objects/tryska`, `zahradnik` | fait (Gemini) | `sources/tryska_gemini.jpg`, `sources/zahradnik_gemini.jpg` |
| `raj/far`, `raj/mid`, `raj/tiles` (niveau 1-3) | fait (Gemini ; mid en 2 bandes, `--strips 2`) | `sources/raj_*_gemini.jpg` |
| `objects/platforma`, `flasa`, `vevericka`, `orol`, `rozcestnik`, `les` | fait (Gemini ; `rozcestnik` importé avec `--grid 2x2 --pick 1,2 --uniform` : Gemini a rendu 4 cases) | `sources/{platforma,flasa,vevericka_big,orol,rozcestnik,les}_gemini.jpg` |
| `tatry/far`, `tatry/mid`, `tatry/tiles` (niveau 1-4) | fait (Gemini ; mid en 2 images empilées, `--strips 2` ; tuiles : cases réaffectées, eau profonde et herbe recalées après import) | `sources/tatry_*_gemini.jpg`, `tatry_mid_a/b.jpg` |
| `objects/kamen`, `chata`, `chatar`, `hory` | fait (Gemini ; `chata` importé avec `--keep-rules`, sans `--strip-ground`) | `sources/{kamen,chata,chatar,hory}_gemini.jpg` |
| `sprites/fox` ligne `climb` (échelle) | fait (Gemini, de dos) | `sources/fox_climb_gemini.jpg` |

## Marche à suivre

1. Coller le prompt dans Gemini. Pour les personnages, **joindre l'image de référence** indiquée.
2. Enregistrer l'image dans `art/sources/` (pour pouvoir la réimporter plus tard).
3. L'importer avec la commande donnée sous le prompt : l'outil détoure le magenta, découpe les images d'animation, réduit à la bonne taille et à une palette courte.
4. `python3 tools/build_assets.py`, puis vérifier dans le simulateur : `make -C sim && build/sim --bot --every 120 --out /tmp/zr`.

Si l'IA ajoute un trait de sol sous un objet : `--strip-ground`. Si les images n'ont pas la même taille exprès (ressort comprimé, petit nuage) : `--uniform`. Si elle mélange l'ordre des tuiles : `--cells` (voir la réimportation des tuiles en bas de page).

## Niveau 1-2 : Sucho, la sécheresse à Košice (à faire)

La série Sucho du cours : canicule, la fontaine chantante devant le théâtre est à sec. Buses de la fontaine qui propulsent (jet), sol desséché qui s'effondre, étang boueux, et le jardinier (pán Záhradník, un blaireau) à l'arrivée. Images provisoires : `tools/make_placeholder_levels.py` (ne remplace jamais une image IA importée).

### Fond lointain (`sucho/far.png`)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 48 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Wide side-scrolling game background, aspect ratio 2:1. A scorching summer heatwave over the city of Košice in eastern Slovakia: pale washed-out blue sky turning hazy yellow near the horizon, a big white-yellow sun, no clouds. In the far distance, dry yellow-brown hills burnt by the drought, and pale rows of panel housing blocks (sídliská) half lost in the heat haze. Everything sits in the lower half of the image; the upper half is sky. Atmospheric perspective: distant elements are pale and low-contrast.
```

```
python3 tools/import_ai.py far art/sources/sucho_far_gemini.jpg --theme sucho --mirror
```

### Le parc du théâtre et la gare (`sucho/mid.png`)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 64 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A side-scrolling game background layer made of TWO horizontal strips stacked one above the other, filling the whole image, separated by a thick flat solid magenta #FF00FF band. Each strip is a very wide panorama (about 3.5:1), seen straight from the side, with the ground line at the same height in both strips. The two strips are two consecutive halves of ONE continuous scene: the right end of the TOP strip continues seamlessly into the left end of the BOTTOM strip, and the right end of the BOTTOM strip joins seamlessly with the left end of the TOP strip (the same yellow trees at both seams), so the whole can repeat.
The scene is the centre of Košice during a drought. TOP strip, left to right: a park with trees whose leaves have turned yellow from the heat, then the cream-coloured neo-baroque State Theatre of Košice with its green copper dome and arched windows, and in front of it the singing fountain (Spievajúca fontána), a wide stone basin that is completely EMPTY and dry, with its metal nozzles sticking out. BOTTOM strip, left to right: the dry park with benches and a few yellow trees, then the Košice railway station building with a clock, then yellowed trees again at the right end. Dry yellowed grass along the bottom edge of each strip; the bottom edge of each strip is the ground. Everything above the roofs and tree tops is flat solid magenta #FF00FF.
```

```
python3 tools/import_ai.py mid art/sources/sucho_mid_gemini.jpg --theme sucho --strips 2 --clear-sky
```

Gemini ignore le format 4:1 (il rend environ 2,4:1, soit un fond de 349 px de large). Ce prompt demande donc **deux bandes superposées** : je les découpe et je les mets bout à bout (couche d'environ 900-1150 px de large). `--clear-sky` retire le ciel bleu que Gemini peint parfois derrière les bâtiments.

### Tuiles (`sucho/tiles.png`)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 64 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Game tileset sheet, exactly 8 columns by 4 rows of square tiles, each tile 16x16 pixels, upscaled x8 (1024x512 image), perfectly aligned on the grid, flat magenta #FF00FF where a tile is empty, no grid lines. Theme: parched park ground in a drought. Tiles in reading order: 0 empty; 1 dry ground seen from the side: a thin strip of yellowed dry grass on top of cracked light-brown earth; 2 plain cracked dry earth; 3 same as 1 with a left edge; 4 same as 1 with a right edge; 5 same as 2 with a left edge; 6 same as 2 with a right edge; 7 a grey stone paving block; 8 a wooden crate decorated with a red gingerbread heart with white icing; 9 the same crate, empty and faded; 10, 11, 12 a thin stone ledge (left end, middle, right end); 13 sharp metal spikes; 14 empty; 15 the surface of a muddy pond, greyish green with small pale ripples on top; 16 deep muddy water, darker green; 17 the same surface as 15 with the ripples shifted (second animation frame); 18 a slab of dry earth with deep dark cracks, looking fragile; 19 the same slab breaking apart, cracks wider, small pieces falling; 20 to 31 empty.
```

```
python3 tools/import_ai.py tiles art/sources/sucho_tiles_gemini.jpg --theme sucho
```

### Buse de la fontaine (`objects/tryska_16x64`, 4 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 24 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A fountain nozzle for a side-view platform game, from the singing fountain of Košice: a short grey metal nozzle at the bottom, seen exactly from the side. 4 frames in a single horizontal row, evenly spaced with wide gaps, same scale, nozzle on the same baseline: 1 nozzle off, no water; 2 nozzle sputtering a few drops just above it; 3 a powerful vertical jet of light blue and white water rising four times the nozzle width high, with a foamy top; 4 the same full jet with the foam and the white streaks shifted (animation). Each frame is 1 wide by 4 tall and must read clearly at 16x64 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

```
python3 tools/import_ai.py object tryska art/sources/tryska_gemini.jpg --frames 4 --uniform
```

### Pán Záhradník, le jardinier (`objects/zahradnik_32x48`, 2 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Pán Záhradník, the old gardener of the park: an anthropomorphic badger standing upright on two legs, grey fur with the black and white striped face of a badger, wearing a green work dungarees, a straw hat, holding an empty metal watering can, looking worried by the drought. Side view, FACING LEFT. 2 frames in a single horizontal row, evenly spaced with wide gaps, same scale, feet on the same baseline: standing with the watering can, then raising one arm to greet. Each frame is 2 wide by 3 tall and must read clearly at 32x48 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

```
python3 tools/import_ai.py object zahradnik art/sources/zahradnik_gemini.jpg --frames 2 --uniform
```

## Niveau 1-3 : Slovenský raj (à faire)

La gorge de Suchá Belá (série Raj du cours) : échelles, passerelles au-dessus du torrent, écureuils, bouteille d'eau, et pán Orol à l'arrivée. Les images actuelles sont provisoires (`tools/make_placeholder_raj.py`, qui ne remplace jamais une image IA importée).

### Fond lointain (`raj/far.png`)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 48 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Wide side-scrolling game background, aspect ratio 2:1. Clear blue summer sky with a few small white clouds, lighter near the horizon. In the far distance, the jagged blue-grey peaks of the High Tatras (Vysoké Tatry) with small patches of snow on the summits. In front of them, rolling hills covered with dark spruce forest, typical of the Slovak Paradise national park (Slovenský raj) in eastern Slovakia. Everything sits in the lower half of the image; the upper half is sky. Atmospheric perspective: distant elements are pale and low-contrast. No buildings.
```

```
python3 tools/import_ai.py far art/sources/raj_far_gemini.jpg --theme raj --mirror
```

### La gorge de Suchá Belá (`raj/mid.png`)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 64 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A side-scrolling game background layer made of TWO horizontal strips stacked one above the other, filling the whole image, separated by a thick flat solid magenta #FF00FF band. Each strip is a very wide panorama (about 3.5:1), seen straight from the side, with the forest floor at the same height in both strips. The two strips are two consecutive halves of ONE continuous scene: the right end of the TOP strip continues seamlessly into the left end of the BOTTOM strip, and the right end of the BOTTOM strip joins seamlessly with the left end of the TOP strip (the same dark spruce trees at both seams), so the whole can repeat.
The scene is a limestone gorge in the Slovak Paradise national park (the Suchá Belá gorge). TOP strip, left to right: dark green spruce forest, then pale grey-cream limestone cliffs starting to rise, with moss on the ledges and a first thin white waterfall falling down the rock face. BOTTOM strip, left to right: tall limestone cliffs and rock towers with vertical cracks, a second thin white waterfall, dark green spruce trees growing on top of the cliffs, then spruce forest again at the right end. A dense line of spruce forest along the bottom edge of each strip; the bottom edge of each strip is the forest floor. Everything above the cliff tops and tree tops is flat solid magenta #FF00FF.
```

```
python3 tools/import_ai.py mid art/sources/raj_mid_gemini.jpg --theme raj --strips 2
```

### Tuiles (`raj/tiles.png`)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 64 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Game tileset sheet, exactly 8 columns by 4 rows of square tiles, each tile 16x16 pixels, upscaled x8 (1024x512 image), perfectly aligned on the grid, flat magenta #FF00FF where a tile is empty, no grid lines. Theme: a limestone gorge in a Slovak spruce forest. Tiles in reading order: 0 empty; 1 forest floor seen from the side: a thin strip of grass and spruce needles on top of pale grey limestone rock; 2 plain limestone rock; 3 same as 1 with a left edge; 4 same as 1 with a right edge; 5 same as 2 with a left edge; 6 same as 2 with a right edge; 7 a cracked limestone block; 8 a wooden crate decorated with a red gingerbread heart with white icing; 9 the same crate, empty and faded; 10, 11, 12 a metal grating footbridge seen from the side with a thin chain railing (left end, middle, right end); 13 sharp pointed rocks; 14 a metal ladder segment with two side rails and three rungs; 15 the surface of a mountain stream, light blue with small white ripples on top; 16 deep stream water, darker blue; 17 the same surface as 15 with the ripples shifted (second animation frame); 18 to 31 empty.
```

```
python3 tools/import_ai.py tiles art/sources/raj_tiles_gemini.jpg --theme raj
# si Gemini melange l'ordre des cases : --cells "1=L,C 2=L,C ..." (voir Tuiles de Kosice plus bas)
```

### Passerelle mobile (`objects/platforma_48x16`, 1 image)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 24 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A moving metal footbridge for a side-view platform game, like the metal walkways bolted over the water in the gorges of the Slovak Paradise: a flat steel grating plank seen exactly from the side, dark grey with a lighter top edge and two short supports underneath. A single image, centred, proportions exactly 3 wide by 1 tall. It must read clearly at 48x16 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

```
python3 tools/import_ai.py object platforma art/sources/platforma_gemini.jpg
```

### Bouteille d'eau (`objects/flasa_16x16`, 1 image)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 16 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A collectible item for a platform game: a small light blue plastic water bottle with a dark blue cap and a white label, with a tiny shine. A single image, centred, upright. It must read clearly at 16x16 pixels. Flat solid magenta #FF00FF background everywhere, no shadow.
```

```
python3 tools/import_ai.py object flasa art/sources/flasa_gemini.jpg
```

### Écureuil (`objects/vevericka_16x16`, 3 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 24 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A cute enemy for a side-view platform game: a Eurasian red squirrel with a big bushy red tail and ear tufts, seen from the side, facing right. 3 frames in a single horizontal row, evenly spaced with wide gaps, same scale: sitting upright, then jumping (stretched out in mid-air), then squashed flat with dizzy cross eyes. Each frame must read clearly at 16x16 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

```
python3 tools/import_ai.py object vevericka art/sources/vevericka_gemini.jpg --frames 3 --uniform
```

### Pán Orol, le garde du parc (`objects/orol_32x48`, 2 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Pán Orol, the friendly park ranger of the Slovak Paradise: an anthropomorphic golden eagle with brown plumage, a golden nape, a yellow beak, wearing a green forest ranger hat with a small badge, perched on a pale limestone rock. Side view, FACING LEFT. 2 frames in a single horizontal row, evenly spaced with wide gaps, same scale, rock on the same baseline: standing calmly, then raising one wing to greet. Each frame is 2 wide by 3 tall (eagle and rock together) and must read clearly at 32x48 pixels. Flat solid magenta #FF00FF background everywhere, no shadow.
```

```
python3 tools/import_ai.py object orol art/sources/orol_gemini.jpg --frames 2 --uniform
```

### Rozcestník, point de passage (`objects/rozcestnik_16x48`, 2 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 24 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A Slovak hiking trail signpost (rozcestník) for a side-view platform game: a wooden post with two yellow arrow-shaped direction signs pointing left and right (no letters, no numbers), and the red-white-red trail marking painted on the post. Seen exactly from the side, tall and thin (proportions 1 wide by 3 tall). 2 frames in a single horizontal row, evenly spaced with wide gaps, same scale: faded colours (not reached yet), then bright colours with a small red flag on top (checkpoint reached). Each frame must read clearly at 16x48 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

```
python3 tools/import_ai.py object rozcestnik art/sources/rozcestnik_gemini.jpg --frames 2 --uniform
```

### Décor de forêt (`objects/les_32x16`, 2 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Two forest floor decoration props for a side-view platform game, seen exactly from the side, in a single horizontal row with a wide gap between them: first a green fern bush, second a cut tree stump with a small red mushroom with white dots next to it. Both the same height and on the same baseline, each twice as wide as tall. Each must read clearly at 32x16 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

```
python3 tools/import_ai.py object les art/sources/les_gemini.jpg --frames 2
```

### Le renard sur l'échelle (joindre `art/characters/fox-reference.png`, 2 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. He is an ANTHROPOMORPHIC fox teenager who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Orange fur, dark grey ear tips, cream muzzle, grey cheeks, big bushy orange tail with a cream tip, wearing a royal blue hoodie, yellow backpack straps, navy blue trousers and white sneakers. Keep the SAME body proportions as the reference: short legs, big head, about 2.5 heads tall.
Sprite sheet for a side-view platform game: the character seen from BEHIND, climbing a ladder: a 2-frame CLIMBING animation (left arm and right leg up, then right arm and left leg up), back of the hoodie and backpack visible, tail hanging down. Do not draw the ladder. Both frames in a single horizontal row, evenly spaced with wide gaps, same size. The whole pose fits in a square and must read clearly at 48x48 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

```
python3 tools/import_ai.py sprite fox climb art/sources/fox_climb_gemini.jpg --frames 2 --head-px 17.5 --anchor-x 27
```


## Niveau 1-4 : Vysoké Tatry, l'orage (à faire)

La série Tatry du cours : montée vers le refuge, la pluie et l'orage arrivent au fil du niveau (le jeu assombrit l'image et dessine la pluie lui-même, les images restent de jour). Chaînes à grimper, éboulis qui cèdent, rafales de vent, pierres qui tombent, lac glacé, et le gardien du refuge (chatár, un ours) à l'arrivée.

### Fond lointain (`tatry/far.png`)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 48 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Wide side-scrolling game background, aspect ratio 2:1. The High Tatras (Vysoké Tatry) in Slovakia before a storm: a grey-blue sky with heavy dark storm clouds building up on one side, light still breaking through on the other. In the distance, the jagged granite peaks of the High Tatras (like Kriváň and Gerlachovský štít), grey rock with small patches of old snow. Everything sits in the lower half of the image; the upper half is sky. Atmospheric perspective: distant elements are pale and low-contrast. No buildings.
```

```
python3 tools/import_ai.py far art/sources/tatry_far_gemini.jpg --theme tatry --mirror
```

### Pentes de granit et pleso (`tatry/mid.png`)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 64 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Side-scrolling game background layer, very wide panorama (aspect ratio about 4:1), seen straight from the side. A high mountain valley in the High Tatras above the tree line: steep grey granite slopes and scree fields with big fallen boulders, patches of dwarf mountain pine (kosodrevina) along the bottom, and on the right a small dark blue glacial lake (pleso) with a pale shoreline. The bottom edge of the image is the ground. Everything above the mountain slopes is flat solid magenta #FF00FF.
```

```
# deux images (suite l'une de l'autre) empilées dans tatry_mid_gemini.jpg :
python3 tools/import_ai.py mid art/sources/tatry_mid_gemini.jpg --theme tatry --strips 2 --clear-sky
```

### Tuiles (`tatry/tiles.png`)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 64 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Game tileset sheet, exactly 8 columns by 4 rows of square tiles, each tile 16x16 pixels, upscaled x8 (1024x512 image), perfectly aligned on the grid, flat magenta #FF00FF where a tile is empty, no grid lines. Theme: a granite mountain trail in the High Tatras. Tiles in reading order: 0 empty; 1 mountain ground seen from the side: a thin strip of short alpine grass on top of grey granite; 2 plain grey granite with pink speckles; 3 same as 1 with a left edge; 4 same as 1 with a right edge; 5 same as 2 with a left edge; 6 same as 2 with a right edge; 7 a big granite boulder block; 8 a wooden crate decorated with a red gingerbread heart with white icing; 9 the same crate, empty and faded; 10, 11, 12 wooden trail steps / a thin wooden plank (left end, middle, right end); 13 sharp pointed rocks; 14 a vertical steel chain fixed to the rock, like the safety chains on Tatra trails, one chain segment that tiles vertically; 15 the surface of a cold mountain lake, dark blue with small white ripples on top; 16 deep lake water, darker blue; 17 the same surface as 15 with the ripples shifted (second animation frame); 18 loose scree, a pile of small grey stones that looks unstable; 19 the same scree sliding apart with dark gaps; 20 to 31 empty.
```

```
python3 tools/import_ai.py tiles art/sources/tatry_tiles_gemini.jpg --theme tatry \
  --cells "1=0,1 2=0,2 3=0,1,L 4=0,1,R 5=0,2,L 6=0,2,R 7=0,7 8=1,0 9=1,1 10=1,2 11=1,3 12=1,2 13=1,4 14=1,5 15=2,0 17=2,1 18=2,4 19=2,5"
# puis : l'herbe (tuiles 1, 3, 4) est remontée en haut de la case, et l'eau profonde (16) refaite à partir du bas de la tuile 15
```

### Pierre qui tombe (`objects/kamen_16x16`, 2 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 16 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A falling rock for a side-view platform game: a round-ish grey granite stone with a lighter edge on top and a few cracks. 2 frames in a single horizontal row, evenly spaced with wide gaps, same size: the stone, then the same stone rotated a quarter turn (it tumbles). Each frame must read clearly at 16x16 pixels. Flat solid magenta #FF00FF background everywhere, no shadow.
```

```
python3 tools/import_ai.py object kamen art/sources/kamen_gemini.jpg --frames 2 --uniform
```

### Le refuge (`objects/chata_96x64`, 1 image)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A mountain hut (horská chata) in the High Tatras for a side-view platform game, seen exactly from the side: a ground floor of grey stone blocks, an upper floor of dark wooden planks, a steep dark brown roof, a stone chimney, a warm yellow lit window and a wooden door. A single image, proportions exactly 3 wide by 2 tall. It must read clearly at 96x64 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

```
python3 tools/import_ai.py object chata art/sources/chata_gemini.jpg --keep-rules
```

### Chatár, le gardien du refuge (`objects/chatar_32x48`, 2 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
The chatár, keeper of the mountain hut: an old anthropomorphic brown bear standing upright on two legs, with a white beard, wearing a red Nordic wool sweater with a white pattern, a navy blue knitted beanie and sturdy hiking boots. Side view, FACING LEFT. 2 frames in a single horizontal row, evenly spaced with wide gaps, same scale, feet on the same baseline: standing calmly, then raising one paw to greet. Each frame is 2 wide by 3 tall and must read clearly at 32x48 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

```
python3 tools/import_ai.py object chatar art/sources/chatar_gemini.jpg --frames 2 --uniform
```

### Décor de montagne (`objects/hory_32x16`, 2 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 24 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Two mountain decoration props for a side-view platform game, seen exactly from the side, in a single horizontal row with a wide gap between them: first a low bush of dwarf mountain pine (kosodrevina), dark green; second a small cairn of stacked grey stones (mužík) marking the trail. Both the same height and on the same baseline, each twice as wide as tall. Each must read clearly at 32x16 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

```
python3 tools/import_ai.py object hory art/sources/hory_gemini.jpg --frames 2
```

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

Andrea est jouable dans les niveaux suivants : ses cases font 48×48, comme le renard. Le moteur lit la taille dans le nom du fichier de la planche (`rabbit_48x48.png`). Lignes à faire : idle (4), win (2), run (6, comme ANIM_LEN dans game.cpp), jump (1), fall (1), hurt (1), climb (2), glide (2, planeur à oreilles en parachute ; sans cette ligne le moteur garde la pose de chute).

**Avant la première importation** : créer `art/sprites/rabbit_48x48.png` (3 lignes de 48 px de haut, 6 cases de 48 px de large, fond transparent), puis supprimer `art/sprites/rabbit_32x48.png` pour qu'il n'en reste qu'une. Les images déjà importées en 32×48 ne seront plus utilisées.

### Andrea : idle (4 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. She is Andrea, an ANTHROPOMORPHIC rabbit teenage girl from Košice who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Keep the SAME body proportions as the reference: short legs, big head, about 2.5 heads tall. Grey fur, long upright ears (pink inside), white blaze widening into a white muzzle, dark eyes, small Y-shaped nose, grey whiskers, two front teeth. Summer outfit only, NOT a spacesuit, NOT a jumpsuit, no helmet: a mint green t-shirt, blue denim shorts, red sneakers with white soles, a small yellow bag on a shoulder strap.
Sprite sheet for a side-view platform game, the character in side view facing right. All frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. Each frame is a 48x48 square and must read clearly at 48x48 pixels (the character fills the square, feet on the bottom edge). Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
a 4-frame IDLE animation (waiting at a tram stop, looking around, one ear twitching).
```

`python3 tools/import_ai.py sprite rabbit idle art/sources/andrea_idle.png --frames 4`

### Andrea : win (2 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. She is Andrea, an ANTHROPOMORPHIC rabbit teenage girl from Košice who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Keep the SAME body proportions as the reference: short legs, big head, about 2.5 heads tall. Grey fur, long upright ears (pink inside), white blaze widening into a white muzzle, dark eyes, small Y-shaped nose, grey whiskers, two front teeth. Summer outfit only, NOT a spacesuit, NOT a jumpsuit, no helmet: a mint green t-shirt, blue denim shorts, red sneakers with white soles, a small yellow bag on a shoulder strap.
Sprite sheet for a side-view platform game, the character in side view facing right. All frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. Each frame is a 48x48 square and must read clearly at 48x48 pixels (the character fills the square, feet on the bottom edge). Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
a 2-frame VICTORY animation (waving one arm high, big smile).
```

`python3 tools/import_ai.py sprite rabbit win art/sources/andrea_win.png --frames 2`

### Andrea : run (6 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. She is Andrea, an ANTHROPOMORPHIC rabbit teenage girl from Košice who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Keep the SAME body proportions as the reference: short legs, big head, about 2.5 heads tall. Grey fur, long upright ears (pink inside), white blaze widening into a white muzzle, dark eyes, small Y-shaped nose, grey whiskers, two front teeth. Summer outfit only, NOT a spacesuit, NOT a jumpsuit, no helmet: a mint green t-shirt, blue denim shorts, red sneakers with white soles, a small yellow bag on a shoulder strap.
Sprite sheet for a side-view platform game, the character in side view facing right. All frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. Each frame is a 48x48 square and must read clearly at 48x48 pixels (the character fills the square, feet on the bottom edge). Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
a 6-frame RUNNING cycle (arms swinging, legs alternating, ears bouncing).
```

`python3 tools/import_ai.py sprite rabbit run art/sources/andrea_run.png --frames 6`

### Andrea : jump (1 image)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. She is Andrea, an ANTHROPOMORPHIC rabbit teenage girl from Košice who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Keep the SAME body proportions as the reference: short legs, big head, about 2.5 heads tall. Grey fur, long upright ears (pink inside), white blaze widening into a white muzzle, dark eyes, small Y-shaped nose, grey whiskers, two front teeth. Summer outfit only, NOT a spacesuit, NOT a jumpsuit, no helmet: a mint green t-shirt, blue denim shorts, red sneakers with white soles, a small yellow bag on a shoulder strap.
Sprite sheet for a side-view platform game, the character in side view facing right. All frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. Each frame is a 48x48 square and must read clearly at 48x48 pixels (the character fills the square, feet on the bottom edge). Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
a single JUMP frame (rising, one knee up, both arms raised, compact pose with knees bent). Only one frame.
```

`python3 tools/import_ai.py sprite rabbit jump art/sources/andrea_jump.png --frames 1`

### Andrea : fall (1 image)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. She is Andrea, an ANTHROPOMORPHIC rabbit teenage girl from Košice who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Keep the SAME body proportions as the reference: short legs, big head, about 2.5 heads tall. Grey fur, long upright ears (pink inside), white blaze widening into a white muzzle, dark eyes, small Y-shaped nose, grey whiskers, two front teeth. Summer outfit only, NOT a spacesuit, NOT a jumpsuit, no helmet: a mint green t-shirt, blue denim shorts, red sneakers with white soles, a small yellow bag on a shoulder strap.
Sprite sheet for a side-view platform game, the character in side view facing right. All frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. Each frame is a 48x48 square and must read clearly at 48x48 pixels (the character fills the square, feet on the bottom edge). Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
a single FALL frame seen from the side, body UPRIGHT and VERTICAL, dropping straight down through the air: arms raised above the head, legs together and straight, feet pointing down, ears blown upward. The figure is tall and narrow, NOT lying down and NOT horizontal. Only one frame.
```

`python3 tools/import_ai.py sprite rabbit fall art/sources/andrea_fall.png --frames 1`

### Andrea : glide (2 images, planeur)

Pour le vol plané (A tenu en tombant) : les oreilles légèrement écartées et qui battent (version retenue : oreilles de taille normale, la 2e image avec une oreille rabattue). Joindre `art/characters/andrea-reference.png`.

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. She is Andrea, an ANTHROPOMORPHIC rabbit teenage girl from Košice who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Keep the SAME body proportions as the reference: short legs, big head, about 2.5 heads tall. Grey fur, long upright ears (pink inside), white blaze widening into a white muzzle, dark eyes, small Y-shaped nose, grey whiskers, two front teeth. Summer outfit only, NOT a spacesuit, NOT a jumpsuit, no helmet: a mint green t-shirt, blue denim shorts, red sneakers with white soles, a small yellow bag on a shoulder strap.
Sprite sheet for a side-view platform game, the character in side view facing right. All frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. Each frame is a 48x48 square and must read clearly at 48x48 pixels (the character fills the square, feet on the bottom edge). Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
a 2-frame GLIDE animation: Andrea floating gently down through the air with her two long ears opened wide and held up above her head like a parachute canopy, the ears spread apart in a V shape and slightly curved, arms spread out to the sides for balance, legs relaxed and dangling, body UPRIGHT and VERTICAL (NOT lying down, NOT horizontal). Frame 1: ears fully open and high. Frame 2: ears slightly lower and flapping. Do not draw any real parachute, only her ears.
```

`python3 tools/import_ai.py sprite rabbit glide art/sources/andrea_glide.jpg --frames 2 --grid 1x2 --strip-ground`

### Andrea : hurt (1 image)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. She is Andrea, an ANTHROPOMORPHIC rabbit teenage girl from Košice who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Keep the SAME body proportions as the reference: short legs, big head, about 2.5 heads tall. Grey fur, long upright ears (pink inside), white blaze widening into a white muzzle, dark eyes, small Y-shaped nose, grey whiskers, two front teeth. Summer outfit only, NOT a spacesuit, NOT a jumpsuit, no helmet: a mint green t-shirt, blue denim shorts, red sneakers with white soles, a small yellow bag on a shoulder strap.
Sprite sheet for a side-view platform game, the character in side view facing right. All frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. Each frame is a 48x48 square and must read clearly at 48x48 pixels (the character fills the square, feet on the bottom edge). Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
a single HURT frame (surprised, eyes squeezed shut, body leaning back, one arm up). Only one frame.
```

`python3 tools/import_ai.py sprite rabbit hurt art/sources/andrea_hurt.png --frames 1`

### Andrea : climb (2 images)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 40 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours, proportions and face. She is Andrea, an ANTHROPOMORPHIC rabbit teenage girl from Košice who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Keep the SAME body proportions as the reference: short legs, big head, about 2.5 heads tall. Grey fur, long upright ears (pink inside), white blaze widening into a white muzzle, dark eyes, small Y-shaped nose, grey whiskers, two front teeth. Summer outfit only, NOT a spacesuit, NOT a jumpsuit, no helmet: a mint green t-shirt, blue denim shorts, red sneakers with white soles, a small yellow bag on a shoulder strap.
Sprite sheet for a side-view platform game, the character in side view facing right. All frames in a single horizontal row, evenly spaced with wide gaps, same size, feet on the same baseline. Each frame is a 48x48 square and must read clearly at 48x48 pixels (the character fills the square, feet on the bottom edge). Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
a 2-frame CLIMBING animation seen from BEHIND, climbing a ladder (left arm and right leg up, then right arm and left leg up), back of the t-shirt and backpack straps visible. Do not draw the ladder.
```

`python3 tools/import_ai.py sprite rabbit climb art/sources/andrea_climb.png --frames 2`

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

### Ennemis plus grands (pigeon, guêpe, écureuil)

Les ennemis de 16-24 px sont trop petits à côté des personnages de 48×48. Nouvelles tailles :
pigeon **48×32** (3 images), guêpe **32×32** (2 images), écureuil **32×32** (3 images).
Images reçues et importées ; les zones de contact et les décalages sont réglés dans `game.cpp`. À l'origine, je règle aussi les zones de contact et les décalages dans `game.cpp`
(les ennemis deviennent plus faciles à toucher).

**Pigeon** (`objects/holub_48x32`)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A cute enemy for a side-view platform game: a plump grey city pigeon with an iridescent green-purple neck, orange eye and big bright pink feet, seen exactly from the side, facing right. 3 frames in a single horizontal row, evenly spaced with wide gaps: walking frame 1, walking frame 2 (other foot forward, head bobbing), then a third frame where the pigeon is squashed flat with dizzy cross eyes. Same scale, feet on the same baseline. The pigeon fills its frame. Each frame is 3 wide by 2 tall (proportions 3:2) and must read clearly at 48x32 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

```
git rm art/objects/holub_24x16.png
python3 -c "from PIL import Image; Image.new('RGBA',(144,32)).save('art/objects/holub_48x32.png')"
python3 tools/import_ai.py object holub art/sources/holub_big_gemini.jpg --frames 3 --mag-cut 140 --tint-cut 100
```

**Guêpe** (`objects/osa_32x32`)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 24 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A flying enemy for a side-view platform game: a cartoon wasp with yellow and black stripes, a black head with a big white eye highlight and a slightly angry look, a visible stinger, seen from the side and facing right. 2 frames of wing flapping in a single horizontal row, evenly spaced with wide gaps: wings up, wings down. Same scale. The wasp fills its frame. Each frame is a square and must read clearly at 32x32 pixels. Flat solid magenta #FF00FF background everywhere, no shadow.
```

```
git rm art/objects/osa_16x16.png
python3 -c "from PIL import Image; Image.new('RGBA',(64,32)).save('art/objects/osa_32x32.png')"
python3 tools/import_ai.py object osa art/sources/osa_big_gemini.jpg --frames 2
```

**Écureuil** (`objects/vevericka_32x32`)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 24 colours). No text, no labels, no grid, no frames or boxes around the images, no border, no watermark.
A cute enemy for a side-view platform game: a Eurasian red squirrel with a big bushy red tail and ear tufts, seen from the side, facing right. 3 frames in a single horizontal row, evenly spaced with wide gaps, same scale: sitting upright, then jumping (stretched out in mid-air), then squashed flat with dizzy cross eyes. The squirrel fills its frame. Each frame is a square and must read clearly at 32x32 pixels. Flat solid magenta #FF00FF background everywhere, no ground line, no shadow.
```

```
git rm art/objects/vevericka_16x16.png
python3 -c "from PIL import Image; Image.new('RGBA',(96,32)).save('art/objects/vevericka_32x32.png')"
python3 tools/import_ai.py object vevericka art/sources/vevericka_big_gemini.jpg --frames 3 --uniform
```

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

Le saut a été refait en pose compacte (genoux repliés) : précisez toujours « same body proportions as the reference, about 2.5 heads tall », sinon Gemini allonge les jambes.

### Tuiles

Gemini n'a pas respecté l'ordre de la grille : on réaffecte les cases (ligne, colonne de son image) aux tuiles du jeu. `L` / `R` ajoutent un bord gauche / droit, `f` retourne la case.

```
python3 tools/import_ai.py tiles art/sources/kosice_tiles_gemini.jpg \
  --cells "1=0,1 2=0,2 3=0,1,L 4=0,1,R 5=0,5,f 6=0,5 7=0,7 8=1,0 9=1,1 10=2,2 11=1,6 12=2,5 13=3,0"
```

Tuiles du jeu : 1 dessus du sol (pavés), 2 sol plein, 3/4 dessus bord gauche/droit, 5/6 plein bord gauche/droit, 7 bloc de pierre, 8 caisse bonus, 9 caisse vide, 10/11/12 corniche gauche/milieu/droite, 13 pointes.

## Niveaux suivants

Chaque série du cours peut devenir un monde avec ses propres `far.png` / `mid.png` / `tiles.png` : Liptov (crue), Banská Štiavnica (mine), Bratislava (château, Danube).
