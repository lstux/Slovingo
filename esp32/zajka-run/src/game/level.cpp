#include "level.h"

#include <string.h>

TileKind Level::kind(uint8_t t) {
  switch (t) {
    case T_EMPTY:
    case T_SECRET:
    case T_HIDDEN: return K_AIR;
    case T_LEDGE_L:
    case T_LEDGE:
    case T_LEDGE_R: return K_ONEWAY;
    case T_SPIKES: return K_HAZARD;
    case T_LADDER: return K_LADDER;
    case T_WATER:
    case T_WATER_TOP:
    case T_WATER_TOP2: return K_WATER;
    default: return K_SOLID;
  }
}

static bool isGround(char c) { return c == '#' || c == 'x' || c == 'O'; }   // 'x', 'O' : faux murs, dessines comme du sol
static bool isLedge(char c) { return c == '='; }

void Level::load(const LevelDef* d) {
  def = d;
  w = d->w > MAX_LEVEL_W ? MAX_LEVEL_W : d->w;
  h = d->h > MAX_LEVEL_H ? MAX_LEVEL_H : d->h;
  memset(tiles, 0, sizeof(tiles));
  memset(marks, 0, sizeof(marks));
  memset(look, 0, sizeof(look));
  memset(region, 0, sizeof(region));
  nspawns = 0;
  nsecrets = nhidden = 0;
  auto ch = [&](int x, int y) -> char {
    if (y < 0 || y >= h) return ' ';
    if (x < 0 || x >= w) return '#';
    const char* r = d->rows[y];
    // les lignes sont completees par des espaces a la generation
    return r[x];
  };
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      char c = ch(x, y);
      uint8_t t = T_EMPTY;
      if (isGround(c)) {
        bool top = !isGround(ch(x, y - 1));
        bool l = !isGround(ch(x - 1, y)), r = !isGround(ch(x + 1, y));
        if (top) t = l ? T_GROUND_TOP_L : (r ? T_GROUND_TOP_R : T_GROUND_TOP);
        else t = l ? T_GROUND_L : (r ? T_GROUND_R : T_GROUND);
        if (c == 'x' || c == 'O') {   // faux mur : on garde l'image du sol, la case se traverse
          look[y * w + x] = t;
          t = T_SECRET;
          if (c == 'O' && nspawns < (int)(sizeof(spawns) / sizeof(spawns[0]))) spawns[nspawns++] = Spawn{'o', (int16_t)x, (int16_t)y};
        }
      } else if (c == 'u') {
        t = T_HIDDEN;
        nhidden++;
      } else if (c == 'B') {
        t = T_BLOCK;
      } else if (c == '?') {
        t = T_BONUS;
      } else if (isLedge(c)) {
        bool l = !isLedge(ch(x - 1, y)), r = !isLedge(ch(x + 1, y));
        t = l ? T_LEDGE_L : (r ? T_LEDGE_R : T_LEDGE);
      } else if (c == '^') {
        t = T_SPIKES;
      } else if (c == 'X') {
        t = T_CRUMBLE;
      } else if (c == 'H') {
        t = T_LADDER;
      } else if (c == '~') {
        t = ch(x, y - 1) == '~' ? T_WATER : T_WATER_TOP;
      } else if (c == '|') {
        marks[y * w + x] = 1;
      } else if (c == 'S') {
        startX = x * TILE;
        startY = (y + 1) * TILE;  // pieds en bas de la case
      } else if (c != ' ' && nspawns < (int)(sizeof(spawns) / sizeof(spawns[0]))) {
        spawns[nspawns++] = Spawn{c, (int16_t)x, (int16_t)y};
      }
      tiles[y * w + x] = t;
    }
  }
  // les faux murs voisins forment un seul passage secret (parcours en largeur)
  static int16_t queue[MAX_LEVEL_W * MAX_LEVEL_H];
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      if (tiles[y * w + x] != T_SECRET || region[y * w + x]) continue;
      int id = nsecrets < 15 ? ++nsecrets : 15;
      int head = 0, tail = 0;
      queue[tail++] = (int16_t)(y * w + x);
      region[y * w + x] = (uint8_t)id;
      while (head < tail) {
        int i = queue[head++], cx = i % w, cy = i / w;
        const int dx[4] = {1, -1, 0, 0}, dy[4] = {0, 0, 1, -1};
        for (int k = 0; k < 4; k++) {
          int nx = cx + dx[k], ny = cy + dy[k];
          if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;
          int j = ny * w + nx;
          if (tiles[j] == T_SECRET && !region[j]) {
            region[j] = (uint8_t)id;
            queue[tail++] = (int16_t)j;
          }
        }
      }
    }
  }
}
