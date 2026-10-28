// Niveaux de Zajka Run : port de esp32/zajka-run/src/game/level.{h,cpp}.
(function (root) {
  'use strict';

  const TILE = 16, MAX_LEVEL_W = 400, MAX_LEVEL_H = 20;

  // Tuiles de la planche
  const T_EMPTY = 0, T_GROUND_TOP = 1, T_GROUND = 2, T_GROUND_TOP_L = 3, T_GROUND_TOP_R = 4,
    T_GROUND_L = 5, T_GROUND_R = 6, T_BLOCK = 7, T_BONUS = 8, T_BONUS_USED = 9,
    T_LEDGE_L = 10, T_LEDGE = 11, T_LEDGE_R = 12, T_SPIKES = 13, T_LADDER = 14,
    T_WATER_TOP = 15,    // surface (animee avec T_WATER_TOP2)
    T_WATER = 16, T_WATER_TOP2 = 17,
    T_CRUMBLE = 18,      // sol craquele / eboulis : s'effondre quand on reste dessus
    T_CRUMBLE_HIT = 19,  // le meme qui se fissure (affichage seulement)
    T_SECRET = 20,       // faux mur 'x' : dessine comme le sol voisin, mais on le traverse
    T_HIDDEN = 21;       // bloc cache 'u' : invisible, se revele quand on le cogne par en dessous

  const K_AIR = 0, K_SOLID = 1, K_ONEWAY = 2, K_HAZARD = 3, K_LADDER = 4, K_WATER = 5;

  function kind(t) {
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

  const isGround = (c) => c === '#' || c === 'x' || c === 'O';   // 'x', 'O' : faux murs, dessines comme du sol
  const isLedge = (c) => c === '=';

  class Level {
    constructor() {
      this.def = null;
      this.w = 0; this.h = 0;
      this.tiles = new Uint8Array(MAX_LEVEL_W * MAX_LEVEL_H);
      this.marks = new Uint8Array(MAX_LEVEL_W * MAX_LEVEL_H);    // butees invisibles '|'
      this.look = new Uint8Array(MAX_LEVEL_W * MAX_LEVEL_H);     // faux murs : la tuile de sol qu'ils imitent
      this.region = new Uint8Array(MAX_LEVEL_W * MAX_LEVEL_H);   // faux murs : numero du passage secret (1..15)
      this.nsecrets = 0; this.nhidden = 0;
      this.spawns = [];      // { kind, tx, ty }
      this.nspawns = 0;
      this.startX = 32; this.startY = 160;
    }

    load(d) {
      this.def = d;
      const w = this.w = d.w > MAX_LEVEL_W ? MAX_LEVEL_W : d.w;
      const h = this.h = d.h > MAX_LEVEL_H ? MAX_LEVEL_H : d.h;
      this.tiles.fill(0); this.marks.fill(0); this.look.fill(0); this.region.fill(0);
      this.spawns = [];
      this.nspawns = 0;
      this.nsecrets = this.nhidden = 0;
      const tiles = this.tiles;
      const ch = (x, y) => {
        if (y < 0 || y >= h) return ' ';
        if (x < 0 || x >= w) return '#';
        return d.rows[y].charAt(x);   // les lignes sont completees par des espaces a l'export
      };
      const addSpawn = (kind, x, y) => {
        if (this.nspawns < 160) { this.spawns.push({ kind, tx: x, ty: y }); this.nspawns++; }
      };
      for (let y = 0; y < h; y++) {
        for (let x = 0; x < w; x++) {
          const c = ch(x, y);
          let t = T_EMPTY;
          if (isGround(c)) {
            const top = !isGround(ch(x, y - 1));
            const l = !isGround(ch(x - 1, y)), r = !isGround(ch(x + 1, y));
            if (top) t = l ? T_GROUND_TOP_L : (r ? T_GROUND_TOP_R : T_GROUND_TOP);
            else t = l ? T_GROUND_L : (r ? T_GROUND_R : T_GROUND);
            if (c === 'x' || c === 'O') {   // faux mur : on garde l'image du sol, la case se traverse
              this.look[y * w + x] = t;
              t = T_SECRET;
              if (c === 'O') addSpawn('o', x, y);
            }
          } else if (c === 'u') {
            t = T_HIDDEN;
            this.nhidden++;
          } else if (c === 'B') {
            t = T_BLOCK;
          } else if (c === '?') {
            t = T_BONUS;
          } else if (isLedge(c)) {
            const l = !isLedge(ch(x - 1, y)), r = !isLedge(ch(x + 1, y));
            t = l ? T_LEDGE_L : (r ? T_LEDGE_R : T_LEDGE);
          } else if (c === '^') {
            t = T_SPIKES;
          } else if (c === 'X') {
            t = T_CRUMBLE;
          } else if (c === 'H') {
            t = T_LADDER;
          } else if (c === '~') {
            t = ch(x, y - 1) === '~' ? T_WATER : T_WATER_TOP;
          } else if (c === '|') {
            this.marks[y * w + x] = 1;
          } else if (c === 'S') {
            this.startX = x * TILE;
            this.startY = (y + 1) * TILE;   // pieds en bas de la case
          } else if (c !== ' ') {
            addSpawn(c, x, y);
          }
          tiles[y * w + x] = t;
        }
      }
      // les faux murs voisins forment un seul passage secret (parcours en largeur)
      const region = this.region;
      for (let y = 0; y < h; y++) {
        for (let x = 0; x < w; x++) {
          if (tiles[y * w + x] !== T_SECRET || region[y * w + x]) continue;
          const id = this.nsecrets < 15 ? ++this.nsecrets : 15;
          const queue = [y * w + x];
          region[y * w + x] = id;
          for (let head = 0; head < queue.length;) {
            const i = queue[head++], cx = i % w, cy = (i / w) | 0;
            const dx = [1, -1, 0, 0], dy = [0, 0, 1, -1];
            for (let k = 0; k < 4; k++) {
              const nx = cx + dx[k], ny = cy + dy[k];
              if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;
              const j = ny * w + nx;
              if (tiles[j] === T_SECRET && !region[j]) {
                region[j] = id;
                queue.push(j);
              }
            }
          }
        }
      }
    }

    at(tx, ty) {
      if (tx < 0 || tx >= this.w) return T_BLOCK;   // murs invisibles sur les bords
      if (ty < 0) return T_EMPTY;
      if (ty >= this.h) return T_EMPTY;             // le vide en bas : on tombe
      return this.tiles[ty * this.w + tx];
    }
    set(tx, ty, t) {
      if (tx >= 0 && tx < this.w && ty >= 0 && ty < this.h) this.tiles[ty * this.w + tx] = t;
    }
    regionAt(tx, ty) {
      if (tx < 0 || tx >= this.w || ty < 0 || ty >= this.h) return 0;
      return this.region[ty * this.w + tx];
    }
    lookAt(tx, ty) { return this.look[ty * this.w + tx]; }
    mark(tx, ty) {
      if (tx < 0 || tx >= this.w || ty < 0 || ty >= this.h) return true;
      return this.marks[ty * this.w + tx] !== 0;
    }
    kindAt(tx, ty) { return kind(this.at(tx, ty)); }
    // haut d'une echelle : on peut s'y tenir debout
    ladderTop(tx, ty) { return this.at(tx, ty) === T_LADDER && this.at(tx, ty - 1) !== T_LADDER; }
    pixelW() { return this.w * TILE; }
    pixelH() { return this.h * TILE; }
  }

  const ns = root.ZajkaRun = root.ZajkaRun || {};
  Object.assign(ns, {
    TILE, MAX_LEVEL_W, MAX_LEVEL_H, Level, kind,
    T_EMPTY, T_GROUND_TOP, T_GROUND, T_GROUND_TOP_L, T_GROUND_TOP_R, T_GROUND_L, T_GROUND_R, T_BLOCK, T_BONUS,
    T_BONUS_USED, T_LEDGE_L, T_LEDGE, T_LEDGE_R, T_SPIKES, T_LADDER, T_WATER_TOP, T_WATER, T_WATER_TOP2,
    T_CRUMBLE, T_CRUMBLE_HIT, T_SECRET, T_HIDDEN,
    K_AIR, K_SOLID, K_ONEWAY, K_HAZARD, K_LADDER, K_WATER,
  });
})(globalThis);
