#include "world.h"

#include "gen/assets.h"

namespace {
constexpr int WALK_SPEED = 1;  // pixels par image : un pas de case en 16 images
constexpr int RUN_SPEED = 2;   // B tenu
constexpr int TURN_DELAY = 6;  // une pression breve tourne le renard sans le faire avancer
const int DX[4] = {0, 0, -1, 1}, DY[4] = {1, -1, 0, 0};
const Dir OPPOSITE[4] = {DIR_UP, DIR_DOWN, DIR_RIGHT, DIR_LEFT};

// planche de marche de chaque personnage (16 x 24, lignes bas/haut/gauche/droite)
const Sheet* sheetFor(Speaker s) {
  switch (s) {
    case SPK_FOX: return &walk_fox;
    case SPK_ANDREA: return &walk_andrea;
    case SPK_MOUSE: return &walk_mouse;
    case SPK_CHICK: return &walk_chick;
    case SPK_BIRD: return &walk_bird;
    case SPK_OROL: return &walk_orol;
    default: return &walk_human;  // en attendant les autres planches
  }
}

Dir heldDir(const Input& in) {
  if (in.down(BTN_UP)) return DIR_UP;
  if (in.down(BTN_DOWN)) return DIR_DOWN;
  if (in.down(BTN_LEFT)) return DIR_LEFT;
  if (in.down(BTN_RIGHT)) return DIR_RIGHT;
  return (Dir)-1;
}

Dir dirTo(int fx, int fy, int tx, int ty) {
  if (tx > fx) return DIR_RIGHT;
  if (tx < fx) return DIR_LEFT;
  if (ty < fy) return DIR_UP;
  return DIR_DOWN;
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
  follower_ = -1;
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
    n.mark = &m;
  }
  nOvr_ = 0;
  stepped_ = false;
  turn_ = bumpWait_ = 0;
  waitRelease_ = true;  // sinon, la croix encore tenue ramenerait le renard dans la porte
  warp_ = event_ = nullptr;
  talk_ = -1;
  if (followerWho_ != SPK_NONE) setFollower(followerWho_);
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

void World::setNpcHidden(uint8_t tag, bool hidden) {
  for (int i = 0; i < nnpc_; i++)
    if (npc_[i].mark && npc_[i].mark->tag == tag) npc_[i].hidden = hidden;
}

// Le personnage qui suit : on l'ajoute derriere le renard (et on retire celui de la carte s'il y est).
void World::setFollower(Speaker who) {
  followerWho_ = who;
  if (follower_ >= 0) {
    // on l'enleve
    for (int i = follower_; i < nnpc_ - 1; i++) npc_[i] = npc_[i + 1];
    nnpc_--;
    follower_ = -1;
  }
  if (who == SPK_NONE || nnpc_ >= MAX_NPC) return;
  for (int i = 0; i < nnpc_; i++)
    if (npc_[i].who == who) npc_[i].hidden = true;  // le meme, place sur la carte : il nous suit deja
  Walker& f = npc_[nnpc_];
  f = Walker();
  f.who = who;
  f.sheet = sheetFor(who);
  f.follower = true;
  follower_ = nnpc_++;
  placeFollower();
}

void World::placeFollower() {
  if (follower_ < 0) return;
  Walker& f = npc_[follower_];
  // derriere le renard si c'est libre, sinon sur une case voisine libre, sinon sur lui
  Dir tries[4] = {OPPOSITE[p_.dir], DIR_DOWN, DIR_LEFT, DIR_RIGHT};
  int fx = p_.tx, fy = p_.ty;
  for (Dir d : tries) {
    int x = p_.tx + DX[d], y = p_.ty + DY[d];
    if (walkable(x, y)) {
      fx = x;
      fy = y;
      break;
    }
  }
  f.tx = fx;
  f.ty = fy;
  f.px = fx * TILE;
  f.py = fy * TILE;
  f.left = 0;
  f.dir = p_.dir;
}

bool World::walkable(int tx, int ty) const {
  if (tx < 0 || ty < 0 || tx >= map_->w || ty >= map_->h) return false;
  if (map_->tiles->kinds[tileAt(tx, ty)] == TK_SOLID) return false;
  return npcAt(tx, ty, false) < 0;  // on passe a travers celui qui nous suit
}

int World::tileAt(int tx, int ty) const {
  if (tx < 0 || ty < 0 || tx >= map_->w || ty >= map_->h) return 0;
  for (int i = nOvr_ - 1; i >= 0; i--)
    if (ovrX_[i] == tx && ovrY_[i] == ty) return ovrT_[i];
  return map_->base[ty * map_->w + tx];
}

void World::setTile(int tx, int ty, int tile) {
  if (tx < 0 || ty < 0 || tx >= map_->w || ty >= map_->h) return;
  for (int i = 0; i < nOvr_; i++)
    if (ovrX_[i] == tx && ovrY_[i] == ty) {
      ovrT_[i] = (uint8_t)tile;
      return;
    }
  if (nOvr_ >= MAX_OVR) return;
  ovrX_[nOvr_] = (uint8_t)tx;
  ovrY_[nOvr_] = (uint8_t)ty;
  ovrT_[nOvr_] = (uint8_t)tile;
  nOvr_++;
}

int World::npcAt(int tx, int ty, bool withFollower) const {
  for (int i = 0; i < nnpc_; i++) {
    if (npc_[i].hidden || (npc_[i].follower && !withFollower)) continue;
    if (npc_[i].tx == tx && npc_[i].ty == ty) return i;
  }
  return -1;
}

bool World::picked(const MapMark& m) const {
  return picked_ && (picked_[m.pickup >> 3] & (1 << (m.pickup & 7)));
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
  warp_ = event_ = nullptr;
  talk_ = -1;
  inspectX_ = inspectY_ = -1;
  stepped_ = false;
  frame_++;
  if (bumpWait_ > 0) bumpWait_--;
  int speed = in.down(BTN_B) ? RUN_SPEED : WALK_SPEED;
  if (follower_ >= 0) moveWalker(npc_[follower_], speed);
  if (p_.left > 0) {
    moveWalker(p_, speed);
    if (p_.left == 0) {
      stepped_ = true;
      // arrive sur une case : une porte, un objet, une zone ?
      for (int i = 0; i < map_->nMarks; i++) {
        const MapMark& m = map_->marks[i];
        if (m.x != p_.tx || m.y != p_.ty) continue;
        if (m.type == M_WARP) {
          warp_ = &m;
          updateCamera();
          return WE_WARP;
        }
        if (m.type == M_ITEM && !picked(m)) {
          event_ = &m;
          updateCamera();
          return WE_PICKUP;
        }
        if (m.type == M_ZONE) {
          event_ = &m;
          updateCamera();
          return WE_ZONE;
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
        int ox = p_.tx, oy = p_.ty;
        p_.tx = nx;
        p_.ty = ny;
        p_.left = TILE;
        moveWalker(p_, speed);
        // celui qui suit prend la case que le renard quitte
        if (follower_ >= 0) {
          Walker& f = npc_[follower_];
          if (f.left > 0) {  // pas encore arrive : on le pose
            f.px = f.tx * TILE;
            f.py = f.ty * TILE;
            f.left = 0;
          }
          if (f.tx != ox || f.ty != oy) {
            int adx = f.tx - ox, ady = f.ty - oy;
            if (adx * adx + ady * ady == 1) {
              f.dir = dirTo(f.tx, f.ty, ox, oy);
              f.tx = ox;
              f.ty = oy;
              f.left = TILE;
              moveWalker(f, speed);
            } else {  // trop loin (apres un detour impossible) : il nous rejoint
              f.tx = ox;
              f.ty = oy;
              f.px = ox * TILE;
              f.py = oy * TILE;
              f.dir = p_.dir;
            }
          }
        }
      } else if (bumpWait_ == 0) {
        bumpWait_ = 20;
        ev = WE_BUMP;
      }
    }
    if (p_.left == 0 && in.hit(BTN_A)) {
      int fx = p_.tx + DX[p_.dir], fy = p_.ty + DY[p_.dir];
      int n = npcAt(fx, fy, true);
      if (n >= 0) {
        talk_ = n;
        npc_[n].dir = OPPOSITE[p_.dir];  // le PNJ se tourne vers le renard
        ev = WE_TALK;
      } else {
        inspectX_ = fx;
        inspectY_ = fy;
        ev = WE_INSPECT;
        for (int i = 0; i < map_->nMarks; i++) {
          const MapMark& m = map_->marks[i];
          if (m.type == M_SIGN && m.x == fx && m.y == fy) {
            event_ = &m;
            ev = WE_SIGN;
          }
        }
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

void World::drawTiles(Band& b, const uint8_t* layer, bool skipEmpty, bool isBase) {
  const Image8& img = *map_->tiles->img;
  int cols = img.w / TILE;
  int ty0 = (camY_ + b.y0) >> 4, ty1 = (camY_ + b.y0 + b.h - 1) >> 4;
  int tx0 = camX_ >> 4, tx1 = (camX_ + SCREEN_W - 1) >> 4;
  for (int ty = ty0; ty <= ty1; ty++) {
    for (int tx = tx0; tx <= tx1; tx++) {
      int t = 0;
      if (tx >= 0 && ty >= 0 && tx < map_->w && ty < map_->h) t = isBase ? tileAt(tx, ty) : layer[ty * map_->w + tx];
      if (t == 0 && skipEmpty) continue;
      int dx = tx * TILE - camX_, dy = ty * TILE - camY_;
      if (t == 0) b.fillRect(dx, dy, TILE, TILE, 0x0000);
      else b.blit(img, (t % cols) * TILE, (t / cols) * TILE, TILE, TILE, dx, dy);
    }
  }
}

void World::drawWalker(Band& b, const Walker& w) {
  if (w.hidden) return;
  int frame = 0;  // 0 debout, 1 et 2 : un pied puis l'autre
  if (w.left > TILE / 2) frame = 1 + (w.steps & 1);
  b.frame(*w.sheet, w.dir * w.sheet->cols + frame, w.px - camX_, w.py - (CHAR_H - TILE) - camY_);
}

void World::render(Band& b) {
  drawTiles(b, map_->base, false, true);
  // objets a ramasser : posés sur leur case, ils sautillent un peu
  for (int i = 0; i < map_->nMarks; i++) {
    const MapMark& m = map_->marks[i];
    if (m.type != M_ITEM || picked(m) || m.item < 0) continue;
    int bob = ((frame_ / 20) & 1) ? -1 : 0;
    b.frame(item_icons, m.item, m.x * TILE - camX_, m.y * TILE - camY_ + bob);
  }
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
  if (map_->night) {
    // nuit : chaque couleur a moitie, avec un peu de bleu
    int count = SCREEN_W * b.h;
    for (int i = 0; i < count; i++) {
      uint16_t c = b.buf[i];
      uint16_t r = (c >> 11) & 31, g = (c >> 5) & 63, bl = c & 31;
      r = r * 2 / 5;
      g = g * 9 / 20;
      bl = bl * 3 / 5 + 4;
      if (bl > 31) bl = 31;
      b.buf[i] = (uint16_t)((r << 11) | (g << 5) | bl);
    }
  }
}
