# Graphismes de Zajka Run : prompts pour Gemini / ChatGPT

Les images actuelles sont **provisoires** : dessinées par `tools/make_placeholder_art.py`
(décors, objets) et `tools/make_sprites.py` (personnages, à partir des SVG de MathPulse).
Elles fixent les formats. Chaque fichier peut être remplacé indépendamment par une image
générée, sans toucher au code.

## Marche à suivre

1. Générer l'image avec un des prompts ci-dessous (en anglais : les modèles d'images
   les suivent mieux). Pour les personnages, **joindre l'image de référence**
   (`art/characters/fox-reference.png`, `rabbit-reference.png`).
2. L'importer : `python3 tools/import_ai.py <type> ... fichier.png` (détails dans
   `tools/import_ai.py`). L'outil détoure le fond magenta, réduit à la bonne taille,
   découpe les images d'animation et réduit la palette.
3. Régénérer les données : `python3 tools/build_assets.py`.
4. Vérifier dans le simulateur (`make -C sim`, puis `build/sim --bot --every 120 --out /tmp/zr`)
   avant de compiler pour la console.

Conseils valables pour tous les prompts :

- Demander un **fond magenta uni `#FF00FF`** pour tout ce qui doit être transparent
  (personnages, objets, plan intermédiaire). Les IA rendent rarement une vraie transparence.
- Demander les images d'animation **sur une seule ligne, bien espacées**, toutes à la même
  échelle, pieds au même niveau.
- Si le résultat est trop détaillé (anticrénelage, dégradés), insister sur
  *« chunky pixels, hard edges, no anti-aliasing, limited palette »*, ou demander
  *« pixel art at 32×48, upscaled ×8 with nearest neighbour »*.
- Garder le même style d'un fichier à l'autre : réutiliser le bloc **Style** tel quel.

### Style (à placer au début de chaque prompt)

```
16-bit pixel art in the style of a Super Nintendo platformer. Crisp chunky pixels,
hard edges, no anti-aliasing, no blur, limited palette (about 48 colours), warm summer
light, cheerful and clean. No text, no logo, no watermark, no border.
```

---

## Niveau 1 : Košice, Hlavná ulica

### `art/kosice/far.png` : fond lointain (480 × 240, boucle horizontale)

`python3 tools/import_ai.py far image.png`

```
[Style] Wide side-scrolling game background, aspect ratio 2:1, seamless left-right tiling.
Clear blue summer sky with a few puffy white clouds, fading lighter near the horizon.
Far in the distance, low green wooded hills of eastern Slovakia, and pale blue-grey
silhouettes of 1970s concrete housing blocks (Slovak "sídlisko") on the outskirts of Košice.
Everything sits in the lower third of the image; the upper two thirds are sky.
Atmospheric perspective: distant elements are pale and low-contrast.
```

### `art/kosice/mid.png` : rue Hlavná et le Dóm (640 × 176, transparent au-dessus des toits)

`python3 tools/import_ai.py mid image.png`

```
[Style] Side-scrolling game background layer, very wide panorama (aspect ratio about 3.6:1),
seamless left-right tiling. The main pedestrian street of Košice, Slovakia ("Hlavná ulica"),
seen straight from the side at street level, the bottom edge of the image is the pavement line.
A row of pastel baroque and classicist burgher houses (yellow, salmon pink, mint green, cream,
light blue) with red clay tile roofs, white window frames and arched gateways, linden trees
in front. In the right half: St. Elisabeth Cathedral (Dóm svätej Alžbety), a large Gothic
cathedral of grey stone with flying buttresses, tall pointed windows, a steep roof covered in
glazed tiles forming a green, yellow and red diamond pattern, a tall north tower topped by a
dark baroque spire with golden details, and a rose window on the west gable. Next to it the
slender Urban Tower (Urbanova veža). In the left half: the neo-baroque State Theatre with its
green copper dome. Everything above the rooftops and spires is flat solid magenta #FF00FF.
```

### `art/kosice/tiles.png` : tuiles du sol (128 × 64, grille de 8 × 4 tuiles de 16 px)

`python3 tools/import_ai.py tiles image.png`

C'est le fichier le plus délicat pour une IA (grille exacte). Si le résultat ne colle
pas, garder les tuiles provisoires : elles sont déjà dans le bon style. L'ordre attendu
(ligne par ligne, de gauche à droite) :

| n° | tuile |
|---|---|
| 0 | vide (magenta) |
| 1 | dessus du sol : pavés de granit gris-beige avec bordure de trottoir |
| 2 | sol plein : terre brune et cailloux |
| 3 / 4 | dessus du sol, bord gauche / bord droit (au bord d'un trou) |
| 5 / 6 | sol plein, bord gauche / bord droit |
| 7 | bloc de pierre taillée |
| 8 | caisse en bois ornée d'un cœur en pain d'épices (bloc bonus) |
| 9 | la même caisse, vide et terne (bonus déjà pris) |
| 10 / 11 / 12 | corniche de pierre claire avec balustres (gauche, milieu, droite) |
| 13 | pointes : barrière de chantier hérissée, bande jaune et noire en bas |
| 14 à 31 | libres (magenta) |

```
[Style] Game tileset sheet, exactly 8 columns by 4 rows of square tiles, each tile 16x16 pixels,
upscaled x8 (1024x512 image), perfectly aligned on the grid, flat magenta #FF00FF where a tile
is empty. Tiles in reading order: 0 empty; 1 grey-beige granite cobblestone pavement seen from
the side with a kerb on top; 2 brown earth with small stones; 3 same as 1 with a left edge;
4 same as 1 with a right edge; 5 same as 2 with a left edge; 6 same as 2 with a right edge;
7 dressed stone block; 8 wooden crate decorated with a red gingerbread heart with white icing;
9 the same crate, empty and faded; 10, 11, 12 light stone cornice with small balusters (left
end, middle, right end); 13 construction-site spikes on a yellow and black striped base;
14 to 31 empty.
```

---

## Personnages

Les planches font **32 × 48 par image**, une ligne par animation, dans cet ordre :
`idle` (4 images), `run` (6), `jump` (1), `fall` (1), `hurt` (1), `win` (2).
On peut en remplacer une ligne à la fois :

`python3 tools/import_ai.py sprite fox run image.png --frames 6`

Le renard regarde vers la droite (le jeu le retourne pour aller à gauche).

### Le renard (`art/sprites/fox_32x48.png`), joindre `fox-reference.png`

```
[Style] Sprite sheet for a side-view platform game. The character is EXACTLY the one in the
attached reference image: an anthropomorphic orange fox with dark grey ear tips, cream muzzle,
grey cheeks, a royal blue hoodie, yellow backpack straps, navy blue trousers and white sneakers,
a big bushy orange tail with a cream tip. Show him in 3/4 side view facing right,
in a 6-frame RUNNING cycle, all frames in a single horizontal row, evenly spaced with gaps,
same size, feet on the same baseline. Each frame reads clearly at 32x48 pixels.
Flat solid magenta #FF00FF background, no shadow on the ground.
```

Variantes de la dernière phrase pour les autres lignes :

- `idle` : *a 4-frame IDLE animation (gentle breathing, tail swaying)*
- `jump` : *a single JUMP frame (rising, one knee up, arms up)*
- `fall` : *a single FALL frame (arms out for balance)*
- `hurt` : *a single HURT frame (knocked back, surprised face)*
- `win` : *a 2-frame VICTORY animation (waving one arm high, big smile)*

### Andrea, la lapine (`art/sprites/rabbit_32x48.png`), joindre `rabbit-reference.png`

```
[Style] Sprite sheet for a side-view platform game. The character is EXACTLY the one in the
attached reference image: an anthropomorphic grey rabbit with long ears (pink inside), white
blaze and muzzle, two front teeth, a pale lavender jumpsuit with pink stripes, a gold star
badge, dark slate boots and white gloves. 3/4 side view facing right, a 4-frame IDLE
animation (waiting at a tram stop, looking around), all frames in one horizontal row, evenly
spaced, same size, feet on the same baseline. Flat solid magenta #FF00FF background.
```

Andrea n'utilise pour l'instant que `idle` (à l'arrêt) et `win` (à l'arrivée), et `run`
sur l'écran titre.

---

## Objets (`art/objects/`)

`python3 tools/import_ai.py object <nom> image.png --frames N`

| fichier | taille | images | prompt (après le bloc Style, fond magenta #FF00FF) |
|---|---|---|---|
| `medovnik_16x16` | 16 × 16 | 4 | *Slovak gingerbread heart (medovník), red-brown with white icing scallops and tiny coloured dots, rotating coin animation: front, three-quarter, edge-on, three-quarter* |
| `holub_24x16` | 24 × 16 | 3 | *a plump grey city pigeon with an iridescent green neck, walking to the right, 2 walking frames, then 1 frame squashed flat with dizzy eyes* |
| `osa_16x16` | 16 × 16 | 2 | *a cartoon wasp flying to the right, yellow and black stripes, 2 frames of wing flapping* |
| `pruzina_16x16` | 16 × 16 | 2 | *a red spring pad on a metal base, relaxed then compressed* |
| `lampa_16x48` | 16 × 48 | 2 | *a historic cast-iron street lamp from an old European town centre, unlit then lit with a warm glow* |
| `zastavka_16x48` | 16 × 48 | 1 | *a tram stop pole with a small sign showing a tram pictogram and a timetable panel* |
| `elektricka_96x48` | 96 × 48 | 1 | *a classic Czechoslovak Tatra T3 tram in a cream and red livery, seen exactly from the side, with its pantograph up, facing right* |
| `lavicka_32x16` | 32 × 16 | 2 | *a wooden park bench with cast-iron legs; and a wooden flower box with red and yellow flowers* |
| `fx_16x16` | 16 × 16 | 5 | *3 frames of a small dust puff growing and fading, then 2 frames of a yellow sparkle* |
| `hud_8x8` | 8 × 8 | 4 | (icônes minuscules : mieux vaut les garder à la main) |

---

## Niveaux suivants (idées pour la suite, mêmes formats)

Le récit du cours traverse la Slovaquie. Chaque série peut devenir un monde avec ses
propres `far.png` / `mid.png` / `tiles.png` :

- **Slovenský raj** : gorges, échelles et passerelles métalliques au-dessus de l'eau.
- **Vysoké Tatry** : Štrbské Pleso, sommets, orage (éclairs en arrière-plan).
- **Liptov (Povodeň)** : crue, l'eau qui monte, toits et barques.
- **Banská Štiavnica (Bana)** : ville minière, galeries, wagonnets.
- **Bratislava (Hrad)** : le château blanc aux quatre tours, le Danube, le pont SNP.
