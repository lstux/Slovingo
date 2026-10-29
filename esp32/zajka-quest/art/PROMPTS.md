# Graphismes de Zajka Quest : prompts pour Gemini / ChatGPT

Chaque image du jeu peut être remplacée par une image générée, sans toucher au code. Les prompts
ci-dessous sont **complets** : un bloc = un copier-coller. Même méthode que pour Zajka Run
(`../zajka-run/art/PROMPTS.md`), mais en **vue de dessus**, à la Pokémon ou Zelda Game Boy.

## État

| fichier | état | source |
| --- | --- | --- |
| `tiles/kosice.png` rue Hlavná | provisoire (dessiné par script) | prompt « Rue » |
| `tiles/kosice.png` objets de la rue | provisoire | prompt « Objets » |
| `tiles/kosice.png` tram | provisoire | prompt « Tram » |
| `tiles/kosice.png` maison de la tante | provisoire | prompt « Maison et jardin » |
| `tiles/kosice.png` kiosque, composteur, panneau | provisoire | prompt « Ticket et panneaux » |
| `tiles/kosice.png` parc à sec, fontaine vide, gare | provisoire | prompt « Sucho » |
| `items/*.png` objets du sac | provisoire | prompt « Objets du sac » |
| `walk/mouse_16x24.png`, `walk/chick_16x24.png` | provisoire | prompt « Animaux du parc » |
| `walk/fox_16x24.png` renard qui marche | provisoire | prompt « Renard qui marche » |
| `walk/andrea_16x24.png` Andrea qui marche | provisoire | prompt « Andrea qui marche » |
| `walk/human_16x24.png` passants, voyageurs | provisoire | prompt « Passant » |
| `portraits/fox_48x48.png`, `andrea_48x48.png` | provisoire (têtes SVG de Slovingo) | prompts « Portraits » |
| `sprites/fox_48x48.png`, `rabbit_32x48.png` (écran titre) | fait (Gemini, repris de Zajka Run) | `../zajka-run/art/sources/` |

## Marche à suivre

1. Coller le prompt dans Gemini. Pour le renard et Andrea, **joindre l'image de référence** indiquée.
2. Enregistrer l'image dans `art/sources/` (pour pouvoir la réimporter plus tard), sous le nom
   donné dans la commande.
3. L'importer avec la commande donnée sous le prompt. L'outil découpe la grille, détoure le
   magenta, réduit à 16 × 16 (tuiles), 16 × 24 (marche) ou 48 × 48 (portraits) et à une palette
   courte.
4. `python3 tools/build_assets.py`, puis regarder dans le simulateur :
   `make -C sim && build/sim --script sim/chapter.txt --auto --every 120 --out /tmp/zq`
   (une capture toutes les 2 s, en PPM).

Pour revenir aux images provisoires : `python3 tools/make_placeholder_art.py` (redessine tout).

**Les tuiles se rangent par nom** (ceux de `TILESET`, en tête de `tools/build_assets.py`) :
`--names` donne le nom de chaque case de l'image, en lisant de gauche à droite puis de haut en
bas. `nom@fond` : un objet sur fond magenta, posé sur la tuile `fond` déjà importée (importer
donc les textures de la rue **avant** les objets). `-` : case ignorée. Si Gemini ne respecte pas
l'ordre demandé, il suffit de changer l'ordre des noms dans la commande.

Une tuile fait 16 × 16 pixels : les détails fins disparaissent. Demander des formes simples et
contrastées. Si le résultat est trop détaillé, ajouter « chunky pixels, each tile drawn at 16x16
then upscaled with nearest neighbour ».

## Les tuiles

### Rue (9 textures)

```
16-bit pixel art for a top-down RPG in the style of Pokémon Gold and Zelda: A Link to the Past (Super Nintendo), seen from above at a slight 3/4 angle. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours), warm summer daylight. No text, no labels, no numbers, no watermark.
A tileset: a grid of 3 rows x 3 columns of SQUARE tiles separated by thin black lines. Each tile is a seamless texture that repeats in every direction and must read clearly at 16x16 pixels. In reading order:
1. grey granite cobblestones of a historic pedestrian street in Košice, Slovakia;
2. the same cobblestones with a round cast-iron manhole cover in the middle;
3. light beige stone paving slabs (a sidewalk);
4. short green grass;
5. a small flower bed: grass with red, yellow and pink flowers inside a low wooden border;
6. tram rails: two steel rails running horizontally across grey cobblestones;
7. red clay roof tiles seen from above, with a cream cornice along the bottom edge;
8. a pastel yellow baroque house facade seen from the front, with one white-framed window with blue glass;
9. the same yellow facade with a brown wooden shop door.
```

`python3 tools/import_ai.py tileset art/sources/rue.png --grid 3x3 --names "paves paves2 trottoir herbe fleurs rails toit facade porte"`

### Objets de la rue (7 objets, sur magenta)

```
16-bit pixel art for a top-down RPG in the style of Pokémon Gold and Zelda: A Link to the Past (Super Nintendo), seen from above at a slight 3/4 angle. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours), warm summer daylight. No text, no labels, no numbers, no watermark.
Seven small objects, each one centred in its own square cell of a grid of 2 rows x 4 columns, on a flat solid magenta #FF00FF background everywhere: no ground, no shadow, no grid lines. Each object fills most of its cell and must read clearly at 16x16 pixels. In reading order:
1. a green tram stop shelter with a small yellow sign;
2. a wooden park bench;
3. a black cast-iron street lamp;
4. the trunk of a lime tree with the base of its roots, without leaves;
5. the round leafy crown of a lime tree seen from above, without trunk;
6. a square stone fountain basin with blue water and a small jet in the middle;
7. the leafy crown of an apple tree with small red apples, seen from above, without trunk;
8. leave this last cell empty (only magenta).
```

`python3 tools/import_ai.py tileset art/sources/objets.png --grid 2x4 --names "abri@trottoir banc@paves lampadaire@paves tronc@paves feuillage fontaine@paves feuillage_pommier -"`

Le feuillage passe devant les personnages : il reste transparent autour des feuilles.

### Tram (7 tuiles)

```
16-bit pixel art for a top-down RPG in the style of Pokémon Gold and Zelda: A Link to the Past (Super Nintendo), seen from above at a slight 3/4 angle. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours), warm summer daylight. No text, no labels, no numbers, no watermark.
A tileset for a red-and-cream Tatra tram of Košice: a grid of 2 rows x 4 columns of SQUARE tiles separated by thin black lines, each one must read clearly at 16x16 pixels. In reading order:
1. the side of the tram seen from the street: red body, cream stripe along the top, a light blue window (this tile repeats horizontally);
2. the tram door seen from the street: red body with a cream double door with two small windows;
3. the tram floor inside: grey ribbed rubber (seamless);
4. a blue fabric tram seat seen from above, on the grey floor;
5. the inside wall of the tram with a large light blue window in a cream frame (repeats horizontally);
6. the red lower body of the tram seen from inside (repeats horizontally);
7. the grey tram floor with a yellow vertical handrail pole in the middle;
8. leave this last cell empty (plain grey).
```

`python3 tools/import_ai.py tileset art/sources/tram.png --grid 2x4 --names "tram porte_tram plancher siege paroi caisse barre -"`

### Maison et jardin de la tante (7 tuiles)

La carte est affichée de nuit par le jeu (couleurs assombries) : dessiner en plein jour.

```
16-bit pixel art for a top-down RPG in the style of Pokémon Gold and Zelda: A Link to the Past (Super Nintendo), seen from above at a slight 3/4 angle. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours), warm daylight. No text, no labels, no numbers, no watermark.
A tileset for a family house with a garden on the outskirts of Košice, Slovakia: a grid of 2 rows x 4 columns of SQUARE tiles separated by thin black lines, each one must read clearly at 16x16 pixels. In reading order:
1. a whitewashed house wall seen from the front with one window with brown wooden shutters and warm yellow light inside (repeats horizontally);
2. the brown wooden front door of the house with a grey stone step;
3. a wooden picket fence on grass (repeats horizontally);
4. an open wooden garden gate on a gravel path (the path runs vertically);
5. a gravel garden path running vertically through short grass;
6. the trunk of an apple tree with its roots, on short grass, without leaves;
7. a round dark green bush on short grass;
8. leave this last cell empty (plain grass).
```

`python3 tools/import_ai.py tileset art/sources/maison.png --grid 2x4 --names "maison porte_maison cloture portail allee pommier buisson -"`

### Ticket et panneaux (3 tuiles)

```
16-bit pixel art for a top-down RPG in the style of Pokémon Gold and Zelda: A Link to the Past (Super Nintendo), seen from above at a slight 3/4 angle. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours), warm summer daylight. No text, no labels, no numbers, no watermark.
Three small objects, each one centred in its own square cell of a grid of 1 row x 4 columns, on a flat solid magenta #FF00FF background everywhere: no ground, no shadow, no grid lines. Each object fills most of its cell and must read clearly at 16x16 pixels. In reading order:
1. a small green newspaper kiosk with a red awning and a counter with newspapers;
2. a yellow tram ticket validator box on a short grey pole, with a black slot;
3. a blue street sign on a grey post (blank, no letters);
4. leave this last cell empty (only magenta).
```

`python3 tools/import_ai.py tileset art/sources/ticket.png --grid 1x4 --names "trafika@paves composteur@plancher panneau@paves -"`

### Sucho : parc à sec, fontaine vide, gare (7 tuiles)

```
16-bit pixel art for a top-down RPG in the style of Pokémon Gold and Zelda: A Link to the Past (Super Nintendo), seen from above at a slight 3/4 angle. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours), hot dry summer light. No text, no labels, no numbers, no watermark.
A tileset for a town in Slovakia during a drought: a grid of 2 rows x 4 columns of SQUARE tiles separated by thin black lines, each one must read clearly at 16x16 pixels. Every tile fills its whole square, edge to edge. In reading order:
1. an empty square stone fountain basin, its bottom dry and cracked, on grey cobblestones;
2. shallow blue pond water (seamless);
3. dry cracked mud at the bottom of a dried-out pond (seamless);
4. a railway station ticket window: a wooden counter under a glass window;
5. light beige stone floor tiles of a station hall (seamless);
6. the side of a blue passenger train carriage with a light blue window and a yellow stripe (repeats horizontally);
7. the door of the same train carriage: grey double door with two small windows;
8. leave this last cell empty (plain grey).
```

`python3 tools/import_ai.py tileset art/sources/sucho.png --grid 2x4 --names "fontaine_vide eau boue guichet sol_gare train porte_train -"`

## Objets du sac (16 × 16)

Ils s'affichent dans le sac (menu START), dans le message « Do batohu » et, pour les gouttes d'eau,
posés par terre. Importer chaque case avec `import_ai.py tileset` n'a pas de sens ici : découper à
la main ou redessiner sur les icônes de `art/items/` (16 × 16, fond transparent).

```
16-bit pixel art icons for an RPG inventory, in the style of Pokémon Gold (Super Nintendo era). Crisp chunky pixels, hard edges, no anti-aliasing, thick dark outlines, limited palette (about 24 colours). No text, no labels, no numbers, no watermark.
Six chunky, exaggerated icons, each one centred in its own square cell of a grid of EXACTLY 2 rows x 3 columns (no more, no fewer), on a flat solid magenta #FF00FF background: no ground, no shadow, no grid lines. Each icon fills at least 80% of its cell and must read clearly at 16x16 pixels. In reading order:
1. a yellow tram ticket;
2. a small plastic bottle of water;
3. a single blue water drop;
4. a yellow-orange backpack;
5. a folded paper map with a red route;
6. two green train tickets.
```

## Les personnages qui marchent (16 × 24)

Une planche par personnage, à la RPG Maker : 4 lignes (de face, vers la gauche, vers la droite,
de dos), 3 colonnes (debout, pied gauche en avant, pied droit en avant). Le personnage est
minuscule à l'écran : demander des proportions « chibi » (grosse tête).

### Renard qui marche (joindre `art/characters/fox-reference.png`)

```
16-bit pixel art in the style of Pokémon Gold and Zelda: A Link to the Past (Super Nintendo). Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 24 colours). No text, no labels, no grid lines, no frames around the images, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours and face. He is an ANTHROPOMORPHIC fox teenager who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Orange fur, dark grey ear tips, cream muzzle, grey cheeks, big bushy orange tail with a cream tip, wearing a royal blue hoodie, yellow backpack straps, navy blue trousers and white sneakers.
Walking sprite sheet for a top-down RPG (like Pokémon or RPG Maker), drawn in chibi proportions (big head, about 2 heads tall): a grid of 4 rows x 3 columns, every character the same size and centred in its cell, feet on the same baseline. Row 1: walking towards the viewer (front view). Row 2: walking to the left (side view facing left). Row 3: walking to the right (side view facing right). Row 4: walking away from the viewer (back view, the tail and the yellow backpack visible). In each row: standing still, left foot forward, right foot forward. Each cell must read clearly at 16x24 pixels. Flat solid magenta #FF00FF background everywhere, no ground, no shadow.
```

`python3 tools/import_ai.py walk fox art/sources/fox_walk.png --grid 4x3 --rows down,left,right,up`

### Andrea qui marche (joindre `art/characters/andrea-reference.png`)

```
16-bit pixel art in the style of Pokémon Gold and Zelda: A Link to the Past (Super Nintendo). Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 24 colours). No text, no labels, no grid lines, no frames around the images, no watermark.
Keep EXACTLY the same character as the attached reference image: same clothes, colours and face. She is Andrea, an ANTHROPOMORPHIC rabbit teenage girl from Košice who stands and walks UPRIGHT ON TWO LEGS like a human, NOT a four-legged animal. Grey fur, long upright ears (pink inside), white blaze widening into a white muzzle, dark eyes, small Y-shaped nose, grey whiskers, two front teeth. Summer outfit: a mint green t-shirt, blue denim shorts, red sneakers with white soles, a small yellow bag on a shoulder strap.
Walking sprite sheet for a top-down RPG (like Pokémon or RPG Maker), drawn in chibi proportions (big head, about 2 heads tall, the long ears included in the height): a grid of 4 rows x 3 columns, every character the same size and centred in its cell, feet on the same baseline. Row 1: walking towards the viewer (front view). Row 2: walking to the left (side view facing left). Row 3: walking to the right (side view facing right). Row 4: walking away from the viewer (back view). In each row: standing still, left foot forward, right foot forward. Each cell must read clearly at 16x24 pixels. Flat solid magenta #FF00FF background everywhere, no ground, no shadow.
```

`python3 tools/import_ai.py walk andrea art/sources/andrea_walk.png --grid 4x3 --rows down,left,right,up`

### Passant (sans référence)

Les passants et les voyageurs du tram sont des adultes humains (le cours les désigne par leur
fonction et les vouvoie).

```
16-bit pixel art in the style of Pokémon Gold and Zelda: A Link to the Past (Super Nintendo). Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 24 colours). No text, no labels, no grid lines, no frames around the images, no watermark.
An ordinary adult passer-by in a Slovak town: short brown hair, a grey jacket over a white shirt, dark trousers, brown shoes, friendly face.
Walking sprite sheet for a top-down RPG (like Pokémon or RPG Maker), drawn in chibi proportions (big head, about 2 heads tall): a grid of 4 rows x 3 columns, every character the same size and centred in its cell, feet on the same baseline. Row 1: walking towards the viewer (front view). Row 2: walking to the left (side view facing left). Row 3: walking to the right (side view facing right). Row 4: walking away from the viewer (back view). In each row: standing still, left foot forward, right foot forward. Each cell must read clearly at 16x24 pixels. Flat solid magenta #FF00FF background everywhere, no ground, no shadow.
```

`python3 tools/import_ai.py walk human art/sources/human_walk.png --grid 4x3 --rows down,left,right,up`

Si le personnage paraît trop petit ou trop grand dans sa case : `--height-px 22` (hauteur debout
en pixels, 23 par défaut).

### Animaux du parc (sans référence)

La souris et le poussin ont soif au bord de l'étang (chapitre 2). Ils sont petits : un seul
dessin suffit, le jeu le réutilise dans les quatre sens.

```
16-bit pixel art in the style of Pokémon Gold (Super Nintendo era). Crisp chunky pixels, hard edges, no anti-aliasing, a dark outline, limited palette (about 16 colours). No text, no labels, no grid lines, no watermark.
Two tiny cute animals of a town park, side by side, each centred in its own square cell of a grid of EXACTLY 1 row x 2 columns (no more, no fewer), thick bold shapes, filling at least 80% of the height of its cell: a small grey mouse with pink ears sitting up, and a fluffy yellow chick. Both seen from the front, looking a little thirsty and sad. Each must read clearly at 16x12 pixels. No ground, no shadow. Flat solid magenta #FF00FF background everywhere, no ground, no shadow.
```

Pour l'instant, à découper à la main dans `art/walk/mouse_16x24.png` et `chick_16x24.png`
(dessin en bas de la case, 16 × 12).

## Portraits de la boîte de dialogue (48 × 48)

Ils s'affichent au-dessus de la boîte, à gauche, quand le personnage parle. Les portraits actuels
sont les têtes SVG de Slovingo ; un portrait en pixel art s'accordera mieux avec le reste.

### Portrait du renard (joindre `art/characters/fox-reference.png`)

```
16-bit pixel art in the style of the character portraits of Super Nintendo RPGs. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours). No text, no labels, no frame, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same face, fur colours and clothes. An ANTHROPOMORPHIC fox teenager: orange fur, dark grey ear tips, cream muzzle, grey cheeks, royal blue hoodie with yellow backpack straps.
A square portrait for a dialogue box: head and shoulders only, facing slightly to the right, friendly and a little cheeky expression, the ears fully inside the picture. It must read clearly at 48x48 pixels. Flat solid magenta #FF00FF background everywhere.
```

`python3 tools/import_ai.py portrait fox art/sources/fox_portrait.png`

### Portrait d'Andrea (joindre `art/characters/andrea-reference.png`)

```
16-bit pixel art in the style of the character portraits of Super Nintendo RPGs. Crisp chunky pixels, hard edges, no anti-aliasing, no blur, limited palette (about 32 colours). No text, no labels, no frame, no border, no watermark.
Keep EXACTLY the same character as the attached reference image: same face, fur colours and clothes. Andrea, an ANTHROPOMORPHIC rabbit teenage girl: grey fur, long upright ears (pink inside), white blaze widening into a white muzzle, dark eyes, small Y-shaped nose, grey whiskers, two front teeth, mint green t-shirt.
A square portrait for a dialogue box: head and shoulders only, facing slightly to the left, calm and slightly mysterious expression with a hint of a smile, the long ears fully inside the picture (they can be a little bent). It must read clearly at 48x48 pixels. Flat solid magenta #FF00FF background everywhere.
```

`python3 tools/import_ai.py portrait andrea art/sources/andrea_portrait.png`

## Chapitres suivants

Chaque série du cours apportera ses lieux (tuiles à ajouter à `TILESET`, avec un prompt du même
modèle) et ses personnages : Maťo (ours) et Katka (marmotte) dès le chapitre 5, Babka Zuzana
(brebis), Ján (écureuil), Marek (blaireau). Chacun aura sa planche de marche et son portrait,
avec les mêmes prompts en changeant la description.

## Notes d'import (premiers essais Gemini)

- Gemini rend souvent un magenta terne (ex. `#C2428D`) au lieu de `#FF00FF` : relever la couleur d'un coin et
  importer avec `--key C2428D --tol 115` (plus la tolérance est basse, plus les bruns/rouges de l'objet sont gardés).
- Les lignes « gauche » et « droite » du renard sortent parfois dans le même sens : recomposer la source
  (ligne droite = case 1 miroir + cases 2 et 3 qui regardent à droite, ligne gauche = son miroir), puis importer.
