#include "world.h"

#include "gen/assets.h"

namespace {
constexpr int WALK_SPEED = 1;  // pixels par image : un pas de case en 16 images
constexpr int RUN_SPEED = 2;   // B tenu
constexpr int TURN_DELAY = 6;  // une pression breve tourne le renard sans le faire avancer
const int DX[4] = {0, 0, -1, 1}, DY[4] = {1, -1, 0, 0};

// planche de marche de chaque personnage (16 x 24, lignes bas/haut/gauche/droite)
const Sheet* sheetFor(Speaker s) {
  switch (s) {
    case SPK_FOX: return &walk_fox;
    case SPK_ANDREA: return &walk_andrea;
    default: return &walk_andrea;  // en attendant les autres planches
  }
}

Dir heldDir(const Input& in) {
  if (in.down(BTN_UP)) return DIR_UP;
  if (in.down(BTN_DOWN)) return DIR_DOWN;
  if (in.down(BTN_LEFT)) return DIR_LEFT;
  if (in.down(BTN_RIGHT)) return DIR_RIGHT;
  return (Dir)-1;
}
}  // namespace

void World::load(int mapIndex, int tx, int ty, Dir dir) {
  mapIndex_ = mapIndex;
  map_ = ALL_MAPS[mapIndex];
  p_ = Walker();
  p_.tx = tx;
  p_.ty = ty;
  p_.px = tx * TILE;
  p_.py = ty * TILE;
  p_.dir = dir;
  p_.sheet = sheetFor(SPK_FOX);
  p_.who = SPK_FOX;
  nnpc_ = 0;
  for (int i = 0; i < map_->nMarks && nnpc_ < MAX_NPC; i++) {
    const MapMark& m = map_->marks[i];
    if (m.type != M_NPC) continue;
    Walker& n = npc_[nnpc_++];
    n = Walker();
    n.tx = m.x;
    n.ty = m.y;
    n.px = m.x * TILE;
    n.py = m.y * TILE;
    n.dir = m.dir;
    n.who = m.npc;
    n.sheet = sheetFor(m.npc);
  }
  turn_ = bumpWait_ = 0;
  waitRelease_ = true;  // sinon, la croix encore tenue ramenerait le renard dans la porte
  warp_ = nullptr;
  talk_ = -1;
  snapCamera();
}

void World::loadStart(int mapIndex) {
  const MapDef& m = *ALL_MAPS[mapIndex];
  for (int i = 0; i < m.nMarks; i++)
    if (m.marks[i].type == M_START) {
      load(mapIndex, m.marks[i].x, m.marks[i].y, m.marks[i].dir);
      return;
    }
  load(mapIndex, m.w / 2, m.h / 2, DIR_DOWN);
}

bool World::walkable(int tx, int ty) const {
  if (tx < 0 || ty < 0 || tx >= map_->w || ty >= map_->h) return false;
  if (map_->tiles->kinds[map_->base[ty * map_->w + tx]] == TK_SOLID) return false;
  return npcAt(tx, ty) < 0;
}

int World::npcAt(int tx, int ty) const {
  for (int i = 0; i < nnpc_; i++)
    if (npc_[i].tx == tx && npc_[i].ty == ty) return i;
  return -1;
}

void World::moveWalker(Walker& w, int speed) {
  if (w.left <= 0) return;
  int s = speed < w.left ? speed : w.left;
  w.px += DX[w.dir] * s;
  w.py += DY[w.dir] * s;
  w.left -= s;
  if (w.left == 0) w.steps++;
}

WorldEvent World::step(const Input& in) {
  warp_ = nullptr;
  talk_ = -1;
  if (bumpWait_ > 0) bumpWait_--;
  if (p_.left > 0) {
    moveWalker(p_, in.down(BTN_B) ? RUN_SPEED : WALK_SPEED);
    if (p_.left == 0) {
      // arrive sur une case : une porte ?
      for (int i = 0; i < map_->nMarks; i++) {
        const MapMark& m = map_->marks[i];
        if (m.type == M_WARP && m.x == p_.tx && m.y == p_.ty) {
          warp_ = &m;
          updateCamera();
          return WE_WARP;
        }
      }
    }
  }
  WorldEvent ev = WE_NONE;
  if (p_.left == 0) {
    Dir d = heldDir(in);
    if (waitRelease_) {
      if (d >= 0) d = (Dir)-1;
      else waitRelease_ = false;
    }
    if (d < 0) {
      turn_ = 0;
    } else if (d != p_.dir) {
      p_.dir = d;
      turn_ = TURN_DELAY;
    } else if (turn_ > 0) {
      turn_--;
    } else {
      int nx = p_.tx + DX[d], ny = p_.ty + DY[d];
      if (walkable(nx, ny)) {
        p_.tx = nx;
        p_.ty = ny;
        p_.left = TILE;
        moveWalker(p_, in.down(BTN_B) ? RUN_SPEED : WALK_SPEED);
      } else if (bumpWait_ == 0) {
        bumpWait_ = 20;
        ev = WE_BUMP;
      }
    }
    if (p_.left == 0 && in.hit(BTN_A)) {
      int n = npcAt(p_.tx + DX[p_.dir], p_.ty + DY[p_.dir]);
      if (n >= 0) {
        talk_ = n;
        // le PNJ se tourne vers le renard
        static const Dir opposite[4] = {DIR_UP, DIR_DOWN, DIR_RIGHT, DIR_LEFT};
        npc_[n].dir = opposite[p_.dir];
        ev = WE_TALK;
      }
    }
  }
  updateCamera();
  return ev;
}

// ---------------------------------------------------------------- camera et rendu
void World::snapCamera() { updateCamera(); }

void World::updateCamera() {
  int mw = map_->w * TILE, mh = map_->h * TILE;
  // carte plus petite que l'ecran : centree ; sinon le renard au centre, sans sortir de la carte
  if (mw <= SCREEN_W) camX_ = (mw - SCREEN_W) / 2;
  else {
    camX_ = p_.px + TILE / 2 - SCREEN_W / 2;
    if (camX_ < 0) camX_ = 0;
    if (camX_ > mw - SCREEN_W) camX_ = mw - SCREEN_W;
  }
  if (mh <= SCREEN_H) camY_ = (mh - SCREEN_H) / 2;
  else {
    camY_ = p_.py + TILE / 2 - SCREEN_H / 2;
    if (camY_ < 0) camY_ = 0;
    if (camY_ > mh - SCREEN_H) camY_ = mh - SCREEN_H;
  }
}

void World::drawTiles(Band& b, const uint8_t* layer, bool skipEmpty) {
  const Image8& img = *map_->tiles->img;
  int cols = img.w / TILE;
  int ty0 = (camY_ + b.y0) >> 4, ty1 = (camY_ + b.y0 + b.h - 1) >> 4;
  int tx0 = camX_ >> 4, tx1 = (camX_ + SCREEN_W - 1) >> 4;
  if (camY_ + b.y0 < 0) ty0 = (camY_ + b.y0 - 15) / 16;  // division vers -infini
  if (camX_ < 0) tx0 = (camX_ - 15) / 16;
  for (int ty = ty0; ty <= ty1; ty++) {
    for (int tx = tx0; tx <= tx1; tx++) {
      int t = 0;
      if (tx >= 0 && ty >= 0 && tx < map_->w && ty < map_->h) t = layer[ty * map_->w + tx];
      if (t == 0 && skipEmpty) continue;
      int dx = tx * TILE - camX_, dy = ty * TILE - camY_;
      if (t == 0) b.fillRect(dx, dy, TILE, TILE, 0x0000);
      else b.blit(img, (t % cols) * TILE, (t / cols) * TILE, TILE, TILE, dx, dy);
    }
  }
}

void World::drawWalker(Band& b, const Walker& w) {
  int frame = 0;  // 0 debout, 1 et 2 : un pied puis l'autre
  if (w.left > TILE / 2) frame = 1 + (w.steps & 1);
  b.frame(*w.sheet, w.dir * w.sheet->cols + frame, w.px - camX_, w.py - (CHAR_H - TILE) - camY_);
}

void World::render(Band& b) {
  drawTiles(b, map_->base, false);
  // personnages du haut vers le bas : celui de devant cache celui de derriere
  const Walker* order[MAX_NPC + 1];
  int n = 0;
  order[n++] = &p_;
  for (int i = 0; i < nnpc_; i++) order[n++] = &npc_[i];
  for (int i = 1; i < n; i++)
    for (int j = i; j > 0 && order[j]->py < order[j - 1]->py; j--) {
      const Walker* t = order[j];
      order[j] = order[j - 1];
      order[j - 1] = t;
    }
  for (int i = 0; i < n; i++) drawWalker(b, *order[i]);
  if (map_->over) drawTiles(b, map_->over, true);
}
