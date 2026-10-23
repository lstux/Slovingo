#include "level.h"

#include <string.h>

TileKind Level::kind(uint8_t t) {
  switch (t) {
    case T_EMPTY: return K_AIR;
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

static bool isGround(char c) { return c == '#'; }
static bool isLedge(char c) { return c == '='; }

void Level::load(const LevelDef* d) {
  def = d;
  w = d->w > MAX_LEVEL_W ? MAX_LEVEL_W : d->w;
  h = d->h > MAX_LEVEL_H ? MAX_LEVEL_H : d->h;
  memset(tiles, 0, sizeof(tiles));
  memset(marks, 0, sizeof(marks));
  nspawns = 0;
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
      } else if (c == 'B') {
        t = T_BLOCK;
      } else if (c == '?') {
        t = T_BONUS;
      } else if (isLedge(c)) {
        bool l = !isLedge(ch(x - 1, y)), r = !isLedge(ch(x + 1, y));
        t = l ? T_LEDGE_L : (r ? T_LEDGE_R : T_LEDGE);
      } else if (c == '^') {
        t = T_SPIKES;
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
}
