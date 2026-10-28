// Monde en vue de dessus : port de esp32/zajka-quest/src/game/world.{h,cpp}.
// Cartes en tuiles de 16 px, deplacement case par case, collisions, PNJ, portes, camera.
(function (root) {
  'use strict';
  const C = root.ZajkaCommon;
  const { SCREEN_W, SCREEN_H, BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_A, BTN_B } = C;

  const TILE = 16, CHAR_H = 24, MAX_NPC = 16;
  const WALK_SPEED = 1, RUN_SPEED = 2, TURN_DELAY = 6;
  const DIR_DOWN = 0, DIR_UP = 1, DIR_LEFT = 2, DIR_RIGHT = 3;
  const TK_WALK = 0, TK_SOLID = 1, TK_OVER = 2;
  const M_START = 0, M_WARP = 1, M_NPC = 2, M_SIGN = 3, M_ITEM = 4, M_ZONE = 5;
  const WE_NONE = 0, WE_WARP = 1, WE_TALK = 2, WE_BUMP = 3, WE_INSPECT = 4, WE_SIGN = 5, WE_PICKUP = 6, WE_ZONE = 7;
  const DX = [0, 0, -1, 1], DY = [1, -1, 0, 0];
  const OPPOSITE = [DIR_UP, DIR_DOWN, DIR_RIGHT, DIR_LEFT];

  class Walker {
    constructor() {
      this.tx = 0; this.ty = 0; this.px = 0; this.py = 0; this.dir = DIR_DOWN; this.left = 0; this.steps = 0;
      this.sheet = null; this.who = 0; this.mark = null; this.hidden = false; this.follower = false;
    }
  }

  function heldDir(inp) {
    if (inp.down(BTN_UP)) return DIR_UP;
    if (inp.down(BTN_DOWN)) return DIR_DOWN;
    if (inp.down(BTN_LEFT)) return DIR_LEFT;
    if (inp.down(BTN_RIGHT)) return DIR_RIGHT;
    return -1;
  }

  function dirTo(fx, fy, tx, ty) {
    if (tx > fx) return DIR_RIGHT;
    if (tx < fx) return DIR_LEFT;
    if (ty < fy) return DIR_UP;
    return DIR_DOWN;
  }

  class World {
    // A : objets de rendu (assets.js), SPK : constantes des personnages
    constructor(A) {
      this.A = A;
      this.map_ = null; this.mapIndex_ = 0;
      this.p_ = new Walker();
      this.npc_ = []; this.nnpc_ = 0;
      this.follower_ = -1; this.followerWho_ = 0;
      this.camX_ = 0; this.camY_ = 0;
      this.turn_ = 0; this.bumpWait_ = 0; this.waitRelease_ = false;
      this.warp_ = null; this.event_ = null; this.picked_ = null;
      this.talk_ = -1; this.inspectX_ = -1; this.inspectY_ = -1;
      this.frame_ = 0;
    }

    sheetFor(who) {
      const K = this.A.K;
      switch (who) {
        case K.SPK_FOX: return this.A.walk_fox;
        case K.SPK_ANDREA: return this.A.walk_andrea;
        case K.SPK_MOUSE: return this.A.walk_mouse;
        case K.SPK_CHICK: return this.A.walk_chick;
        default: return this.A.walk_human;
      }
    }

    load(mapIndex, tx, ty, dir) {
      const K = this.A.K;
      this.mapIndex_ = mapIndex;
      this.map_ = this.A.maps[mapIndex];
      const p = this.p_ = new Walker();
      p.tx = tx; p.ty = ty; p.px = tx * TILE; p.py = ty * TILE; p.dir = dir;
      p.sheet = this.sheetFor(K.SPK_FOX); p.who = K.SPK_FOX;
      this.nnpc_ = 0;
      this.follower_ = -1;
      for (let i = 0; i < this.map_.nMarks && this.nnpc_ < MAX_NPC; i++) {
        const m = this.map_.marks[i];
        if (m.type !== M_NPC) continue;
        const n = new Walker();
        n.tx = m.x; n.ty = m.y; n.px = m.x * TILE; n.py = m.y * TILE; n.dir = m.dir; n.who = m.npc;
        n.sheet = this.sheetFor(m.npc); n.mark = m;
        this.npc_[this.nnpc_++] = n;
      }
      this.turn_ = this.bumpWait_ = 0;
      this.waitRelease_ = true;   // sinon, la croix encore tenue ramenerait le renard dans la porte
      this.warp_ = this.event_ = null;
      this.talk_ = -1;
      if (this.followerWho_ !== K.SPK_NONE) this.setFollower(this.followerWho_);
      this.snapCamera();
    }

    loadStart(mapIndex) {
      const m = this.A.maps[mapIndex];
      for (let i = 0; i < m.nMarks; i++) {
        if (m.marks[i].type === M_START) { this.load(mapIndex, m.marks[i].x, m.marks[i].y, m.marks[i].dir); return; }
      }
      this.load(mapIndex, (m.w / 2) | 0, (m.h / 2) | 0, DIR_DOWN);
    }

    setNpcHidden(tag, hidden) {
      for (let i = 0; i < this.nnpc_; i++) {
        if (this.npc_[i].mark && this.npc_[i].mark.tag === tag) this.npc_[i].hidden = hidden;
      }
    }

    // le personnage qui suit : derriere le renard (et on retire celui de la carte s'il y est)
    setFollower(who) {
      const K = this.A.K;
      this.followerWho_ = who;
      if (this.follower_ >= 0) {
        for (let i = this.follower_; i < this.nnpc_ - 1; i++) this.npc_[i] = this.npc_[i + 1];
        this.nnpc_--;
        this.follower_ = -1;
      }
      if (who === K.SPK_NONE || this.nnpc_ >= MAX_NPC) return;
      for (let i = 0; i < this.nnpc_; i++) if (this.npc_[i].who === who) this.npc_[i].hidden = true;
      const f = new Walker();
      f.who = who; f.sheet = this.sheetFor(who); f.follower = true;
      this.npc_[this.nnpc_] = f;
      this.follower_ = this.nnpc_++;
      this.placeFollower();
    }

    placeFollower() {
      if (this.follower_ < 0) return;
      const f = this.npc_[this.follower_], p = this.p_;
      const tries = [OPPOSITE[p.dir], DIR_DOWN, DIR_LEFT, DIR_RIGHT];
      let fx = p.tx, fy = p.ty;
      for (const d of tries) {
        const x = p.tx + DX[d], y = p.ty + DY[d];
        if (this.walkable(x, y)) { fx = x; fy = y; break; }
      }
      f.tx = fx; f.ty = fy; f.px = fx * TILE; f.py = fy * TILE; f.left = 0; f.dir = p.dir;
    }

    hasFollower() { return this.follower_ >= 0; }
    setPicked(bits) { this.picked_ = bits; }
    map() { return this.map_; }
    mapIndex() { return this.mapIndex_; }
    player() { return this.p_; }
    npc(i) { return this.npc_[i]; }
    npcCount() { return this.nnpc_; }
    pendingWarp() { return this.warp_; }
    event() { return this.event_; }
    talkNpc() { return this.talk_; }
    inspectX() { return this.inspectX_; }
    inspectY() { return this.inspectY_; }

    walkable(tx, ty) {
      const m = this.map_;
      if (tx < 0 || ty < 0 || tx >= m.w || ty >= m.h) return false;
      if (m.tiles.kinds[m.base[ty * m.w + tx]] === TK_SOLID) return false;
      return this.npcAt(tx, ty, false) < 0;   // on passe a travers celui qui nous suit
    }

    tileAt(tx, ty) {
      const m = this.map_;
      if (tx < 0 || ty < 0 || tx >= m.w || ty >= m.h) return 0;
      return m.base[ty * m.w + tx];
    }

    npcAt(tx, ty, withFollower) {
      for (let i = 0; i < this.nnpc_; i++) {
        const n = this.npc_[i];
        if (n.hidden || (n.follower && !withFollower)) continue;
        if (n.tx === tx && n.ty === ty) return i;
      }
      return -1;
    }

    picked(m) { return !!this.picked_ && (this.picked_[m.pickup >> 3] & (1 << (m.pickup & 7))) !== 0; }

    moveWalker(w, speed) {
      if (w.left <= 0) return;
      const s = speed < w.left ? speed : w.left;
      w.px += DX[w.dir] * s;
      w.py += DY[w.dir] * s;
      w.left -= s;
      if (w.left === 0) w.steps++;
    }

    step(inp) {
      const p = this.p_, map = this.map_;
      this.warp_ = this.event_ = null;
      this.talk_ = -1;
      this.inspectX_ = this.inspectY_ = -1;
      this.frame_ = (this.frame_ + 1) >>> 0;
      if (this.bumpWait_ > 0) this.bumpWait_--;
      const speed = inp.down(BTN_B) ? RUN_SPEED : WALK_SPEED;
      if (this.follower_ >= 0) this.moveWalker(this.npc_[this.follower_], speed);
      if (p.left > 0) {
        this.moveWalker(p, speed);
        if (p.left === 0) {
          // arrive sur une case : une porte, un objet, une zone ?
          for (let i = 0; i < map.nMarks; i++) {
            const m = map.marks[i];
            if (m.x !== p.tx || m.y !== p.ty) continue;
            if (m.type === M_WARP) { this.warp_ = m; this.updateCamera(); return WE_WARP; }
            if (m.type === M_ITEM && !this.picked(m)) { this.event_ = m; this.updateCamera(); return WE_PICKUP; }
            if (m.type === M_ZONE) { this.event_ = m; this.updateCamera(); return WE_ZONE; }
          }
        }
      }
      let ev = WE_NONE;
      if (p.left === 0) {
        let d = heldDir(inp);
        if (this.waitRelease_) {
          if (d >= 0) d = -1; else this.waitRelease_ = false;
        }
        if (d < 0) {
          this.turn_ = 0;
        } else if (d !== p.dir) {
          p.dir = d;
          this.turn_ = TURN_DELAY;
        } else if (this.turn_ > 0) {
          this.turn_--;
        } else {
          const nx = p.tx + DX[d], ny = p.ty + DY[d];
          if (this.walkable(nx, ny)) {
            const ox = p.tx, oy = p.ty;
            p.tx = nx; p.ty = ny; p.left = TILE;
            this.moveWalker(p, speed);
            // celui qui suit prend la case que le renard quitte
            if (this.follower_ >= 0) {
              const f = this.npc_[this.follower_];
              if (f.left > 0) {   // pas encore arrive : on le pose
                f.px = f.tx * TILE; f.py = f.ty * TILE; f.left = 0;
              }
              if (f.tx !== ox || f.ty !== oy) {
                const adx = f.tx - ox, ady = f.ty - oy;
                if (adx * adx + ady * ady === 1) {
                  f.dir = dirTo(f.tx, f.ty, ox, oy);
                  f.tx = ox; f.ty = oy; f.left = TILE;
                  this.moveWalker(f, speed);
                } else {   // trop loin : il nous rejoint
                  f.tx = ox; f.ty = oy; f.px = ox * TILE; f.py = oy * TILE; f.dir = p.dir;
                }
              }
            }
          } else if (this.bumpWait_ === 0) {
            this.bumpWait_ = 20;
            ev = WE_BUMP;
          }
        }
        if (p.left === 0 && inp.hit(BTN_A)) {
          const fx = p.tx + DX[p.dir], fy = p.ty + DY[p.dir];
          const n = this.npcAt(fx, fy, true);
          if (n >= 0) {
            this.talk_ = n;
            this.npc_[n].dir = OPPOSITE[p.dir];   // le PNJ se tourne vers le renard
            ev = WE_TALK;
          } else {
            this.inspectX_ = fx; this.inspectY_ = fy;
            ev = WE_INSPECT;
            for (let i = 0; i < map.nMarks; i++) {
              const m = map.marks[i];
              if (m.type === M_SIGN && m.x === fx && m.y === fy) { this.event_ = m; ev = WE_SIGN; }
            }
          }
        }
      }
      this.updateCamera();
      return ev;
    }

    // ---------------------------------------------------------------- camera et rendu
    snapCamera() { this.updateCamera(); }

    updateCamera() {
      const map = this.map_, p = this.p_;
      const mw = map.w * TILE, mh = map.h * TILE;
      if (mw <= SCREEN_W) this.camX_ = Math.trunc((mw - SCREEN_W) / 2);
      else {
        let c = p.px + (TILE >> 1) - (SCREEN_W >> 1);
        if (c < 0) c = 0;
        if (c > mw - SCREEN_W) c = mw - SCREEN_W;
        this.camX_ = c;
      }
      if (mh <= SCREEN_H) this.camY_ = Math.trunc((mh - SCREEN_H) / 2);
      else {
        let c = p.py + (TILE >> 1) - (SCREEN_H >> 1);
        if (c < 0) c = 0;
        if (c > mh - SCREEN_H) c = mh - SCREEN_H;
        this.camY_ = c;
      }
    }

    drawTiles(b, layer, skipEmpty) {
      const map = this.map_, img = map.tiles.img, cols = (img.w / TILE) | 0;
      const ty0 = (this.camY_ + b.y0) >> 4, ty1 = (this.camY_ + b.y0 + b.h - 1) >> 4;
      const tx0 = this.camX_ >> 4, tx1 = (this.camX_ + SCREEN_W - 1) >> 4;
      for (let ty = ty0; ty <= ty1; ty++) {
        for (let tx = tx0; tx <= tx1; tx++) {
          let t = 0;
          if (tx >= 0 && ty >= 0 && tx < map.w && ty < map.h) t = layer[ty * map.w + tx];
          if (t === 0 && skipEmpty) continue;
          const dx = tx * TILE - this.camX_, dy = ty * TILE - this.camY_;
          if (t === 0) b.fillRect(dx, dy, TILE, TILE, 0x0000);
          else b.blit(img, (t % cols) * TILE, ((t / cols) | 0) * TILE, TILE, TILE, dx, dy);
        }
      }
    }

    drawWalker(b, w) {
      if (w.hidden) return;
      let frame = 0;   // 0 debout, 1 et 2 : un pied puis l'autre
      if (w.left > (TILE >> 1)) frame = 1 + (w.steps & 1);
      b.frame(w.sheet, w.dir * w.sheet.cols + frame, w.px - this.camX_, w.py - (CHAR_H - TILE) - this.camY_);
    }

    render(b) {
      const map = this.map_;
      this.drawTiles(b, map.base, false);
      // objets a ramasser : poses sur leur case, ils sautillent un peu
      for (let i = 0; i < map.nMarks; i++) {
        const m = map.marks[i];
        if (m.type !== M_ITEM || this.picked(m) || m.item < 0) continue;
        const bob = (((this.frame_ / 20) | 0) & 1) ? -1 : 0;
        b.frame(this.A.item_icons, m.item, m.x * TILE - this.camX_, m.y * TILE - this.camY_ + bob);
      }
      // personnages du haut vers le bas : celui de devant cache celui de derriere
      const order = [this.p_];
      for (let i = 0; i < this.nnpc_; i++) order.push(this.npc_[i]);
      for (let i = 1; i < order.length; i++) {
        for (let j = i; j > 0 && order[j].py < order[j - 1].py; j--) {
          const t = order[j]; order[j] = order[j - 1]; order[j - 1] = t;
        }
      }
      for (const w of order) this.drawWalker(b, w);
      if (map.over) this.drawTiles(b, map.over, true);
      if (map.night) {
        // nuit : chaque couleur a moitie, avec un peu de bleu
        const count = SCREEN_W * b.h, buf = b.buf;
        for (let i = 0; i < count; i++) {
          const c = buf[i];
          let r = (c >> 11) & 31, g = (c >> 5) & 63, bl = c & 31;
          r = ((r * 2) / 5) | 0;
          g = ((g * 9) / 20) | 0;
          bl = ((bl * 3) / 5 | 0) + 4;
          if (bl > 31) bl = 31;
          buf[i] = (r << 11) | (g << 5) | bl;
        }
      }
    }
  }

  const Q = root.ZajkaQuest = root.ZajkaQuest || {};
  Object.assign(Q, {
    TILE, Walker, World, DIR_DOWN, DIR_UP, DIR_LEFT, DIR_RIGHT, TK_WALK, TK_SOLID, TK_OVER,
    M_START, M_WARP, M_NPC, M_SIGN, M_ITEM, M_ZONE,
    WE_NONE, WE_WARP, WE_TALK, WE_BUMP, WE_INSPECT, WE_SIGN, WE_PICKUP, WE_ZONE,
  });
})(globalThis);
