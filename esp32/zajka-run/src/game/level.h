#pragma once
#include <stdint.h>

#include "gfx.h"

constexpr int TILE = 16;
constexpr int MAX_LEVEL_W = 400;
constexpr int MAX_LEVEL_H = 20;

// Definition d'un niveau telle qu'ecrite dans levels/*.txt (voir la legende en tete du fichier).
struct LevelDef {
  const char* name;
  const char* subtitle;
  const char* hint;       // phrase en slovaque affichee a l'arrivee
  const Theme* theme;
  int music;
  const Sheet* npc;       // personnage a l'arrivee avec Andrea (nullptr : arret de tram seul)
  int weather;            // 0 beau temps, 1 orage qui monte au fil du niveau (pluie, nuit, eclairs)
  const char* next;       // prochaine etape annoncee a la fin du dernier niveau
  int w, h;
  const char* const* rows;
};

// Tuiles de la planche (tools/make_placeholder_art.py documente le dessin de chacune).
enum TileId : uint8_t {
  T_EMPTY = 0,
  T_GROUND_TOP = 1,
  T_GROUND = 2,
  T_GROUND_TOP_L = 3,
  T_GROUND_TOP_R = 4,
  T_GROUND_L = 5,
  T_GROUND_R = 6,
  T_BLOCK = 7,
  T_BONUS = 8,
  T_BONUS_USED = 9,
  T_LEDGE_L = 10,
  T_LEDGE = 11,
  T_LEDGE_R = 12,
  T_SPIKES = 13,
  T_LADDER = 14,
  T_WATER_TOP = 15,   // surface (animee avec T_WATER_TOP2)
  T_WATER = 16,
  T_WATER_TOP2 = 17,
  T_CRUMBLE = 18,     // sol craquele / eboulis : s'effondre quand on reste dessus
  T_CRUMBLE_HIT = 19, // le meme qui se fissure (affichage seulement)
  T_SECRET = 20,      // faux mur 'x' : dessine comme le sol voisin, mais on le traverse (passage secret)
  T_HIDDEN = 21,      // bloc cache 'u' : invisible, se revele quand on le cogne par en dessous
};

enum TileKind : uint8_t { K_AIR, K_SOLID, K_ONEWAY, K_HAZARD, K_LADDER, K_WATER };

// Objets places dans la carte (convertis en entites au chargement).
struct Spawn {
  char kind;
  int16_t tx, ty;
};

struct Level {
  const LevelDef* def = nullptr;
  int w = 0, h = 0;
  uint8_t tiles[MAX_LEVEL_W * MAX_LEVEL_H];
  uint8_t marks[MAX_LEVEL_W * MAX_LEVEL_H];  // butees invisibles '|'
  uint8_t look[MAX_LEVEL_W * MAX_LEVEL_H];   // faux murs : la tuile de sol qu'ils imitent
  uint8_t region[MAX_LEVEL_W * MAX_LEVEL_H]; // faux murs : numero du passage secret (1..15), 0 = aucun
  int nsecrets = 0, nhidden = 0;             // passages secrets et blocs caches du niveau
  Spawn spawns[160];
  int nspawns = 0;
  int startX = 32, startY = 160;

  void load(const LevelDef* d);
  uint8_t at(int tx, int ty) const {
    if (tx < 0 || tx >= w) return T_BLOCK;      // murs invisibles sur les bords
    if (ty < 0) return T_EMPTY;
    if (ty >= h) return T_EMPTY;                // le vide en bas : on tombe
    return tiles[ty * w + tx];
  }
  void set(int tx, int ty, uint8_t t) {
    if (tx >= 0 && tx < w && ty >= 0 && ty < h) tiles[ty * w + tx] = t;
  }
  int regionAt(int tx, int ty) const {
    if (tx < 0 || tx >= w || ty < 0 || ty >= h) return 0;
    return region[ty * w + tx];
  }
  uint8_t lookAt(int tx, int ty) const { return look[ty * w + tx]; }
  bool mark(int tx, int ty) const {
    if (tx < 0 || tx >= w || ty < 0 || ty >= h) return true;
    return marks[ty * w + tx] != 0;
  }
  static TileKind kind(uint8_t t);
  TileKind kindAt(int tx, int ty) const { return kind(at(tx, ty)); }
  // haut d'une echelle : on peut s'y tenir debout
  bool ladderTop(int tx, int ty) const { return at(tx, ty) == T_LADDER && at(tx, ty - 1) != T_LADDER; }
  int pixelW() const { return w * TILE; }
  int pixelH() const { return h * TILE; }
};
