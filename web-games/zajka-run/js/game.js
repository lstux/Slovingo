// Zajka Run : plateformer. Port de esp32/zajka-run/src/game/game.{h,cpp}.
// Logique a 60 Hz (step), rendu dans un tampon RGB565 (renderBand). On garde les memes noms de
// fonctions et de variables que le C++ pour que les deux versions restent faciles a comparer.
(function (root) {
  'use strict';

  const C = root.ZajkaCommon, R = root.ZajkaRun;
  const { SCREEN_W, SCREEN_H, textWidth,
    BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_A, BTN_B, BTN_START, BTN_SELECT, BTN_VOLUME } = C;
  const { TILE, T_EMPTY, T_BLOCK, T_BONUS, T_BONUS_USED, T_LADDER, T_WATER_TOP, T_WATER_TOP2, T_CRUMBLE,
    T_CRUMBLE_HIT, T_SECRET, T_HIDDEN, K_AIR, K_SOLID, K_ONEWAY, K_HAZARD, K_LADDER, K_WATER, Level, kind } = R;

  // Les nombres a virgule du firmware sont des float 32 bits : on arrondit chaque operation avec
  // f() = Math.fround, pour que la physique donne exactement les memes valeurs que sur la console.
  const f = Math.fround;

  // ---------------------------------------------------------------- reglages de la physique (par image a 60 Hz)
  const WALK_MAX = 2.0, RUN_MAX = f(3.4), ACCEL = f(0.10), RUN_ACCEL = f(0.13), AIR_ACCEL = f(0.08);
  const FRICTION = f(0.12), SKID = f(0.28), GRAVITY = f(0.32);
  const GRAVITY_HOLD = f(0.18);   // A tenu en montant : saut plus haut
  const JUMP_V = f(5.6), JUMP_SPEED_BONUS = f(0.18), MAX_FALL = 6.5;
  const STOMP_V = f(4.4), STOMP_V_HOLD = f(6.2);
  const SPRING_V = 8.0, SPRING_V_HOLD = f(9.4);
  const COYOTE = 6, JUMP_BUFFER = 7;
  const INVULN = 90;
  const HEARTS_PER_LIFE = 50;
  const TRAM_ROOF = 20;      // ligne du toit dans l'image 96x48 du tram
  const FOX_ANCHOR = 27;     // x du centre de la tete dans la case de 48 px
  const RABBIT_RUN_FRAMES = 4;
  const CLIMB_V = f(1.6);
  const GLIDE_GRAVITY = f(0.05), GLIDE_FALL = 1.5;   // Andrea plane : A tenu en tombant
  const PLATFORM_V = f(0.8), PLATFORM_VV = f(0.6);
  const PLATFORM_W = 48, PLATFORM_H = 8;
  const CRUMBLE_DELAY = 50, CRUMBLE_BACK = 300;   // images avant l'effondrement, avant le retour
  const JET_PERIOD = 200, JET_SPUTTER = 110, JET_FULL = 140, JET_H = 64;
  const JET_V = 6.0, JET_V_HOLD = f(6.6);
  const WIND_W = 8 * 16, WIND_H = 10 * 16, WIND_PERIOD = 240, WIND_ON = 150;
  const ROCK_PERIOD = 150;

  // animations de la planche du renard / d'Andrea (une ligne par animation)
  const A_IDLE = 0, A_RUN = 1, A_JUMP = 2, A_FALL = 3, A_HURT = 4, A_WIN = 5, A_CLIMB = 6, A_GLIDE = 7;
  const ANIM_LEN = [4, 6, 1, 1, 1, 2, 2, 2];

  const F01 = f(0.01), F015 = f(0.15), F03 = f(0.3), F04 = f(0.04), F06 = f(0.6), F07 = f(0.07), F09 = f(0.9),
    F10 = f(0.10), F12 = f(1.2), F13 = f(1.3), F14 = f(1.4), F16 = f(1.6), F18 = f(0.18), F12c = f(0.12),
    F022 = f(0.22), F025 = f(0.025), F31 = f(3.1), F36 = f(3.6), F35 = f(3.5), F42 = f(4.2), F055 = f(0.55);

  const WHITE = 0xFFFF, BLACK = 0x0000, SHADOW = 0x18C3;
  const YELLOW = 0xFF2A, ORANGE = 0xFB00, DARKBLUE = 0x1926;

  // effets sonores (ordre de SFX_TABLE dans audio.cpp)
  const SFX_JUMP = 0, SFX_COIN = 1, SFX_STOMP = 2, SFX_HURT = 3, SFX_SPRING = 4, SFX_BUMP = 5, SFX_LIFE = 6,
    SFX_DIE = 7, SFX_WIN = 8, SFX_CHECK = 9, SFX_PAUSE = 10, SFX_BELL = 11, SFX_GAMEOVER = 12,
    SFX_THUNDER = 13, SFX_CRUMBLE = 14;

  // types d'entites
  const E_NONE = 0, E_HEART = 1, E_PIGEON = 2, E_WASP = 3, E_SPRING = 4, E_CHECK = 5, E_GOAL = 6, E_TRAM = 7,
    E_BENCH = 8, E_FLOWERS = 9, E_POP = 10, E_DUST = 11, E_SPARK = 12, E_TEXT = 13,
    E_PLATFORM = 14,    // passerelle mobile (state 0 horizontale, 1 verticale)
    E_BOTTLE = 15,      // bouteille d'eau : rend un coeur
    E_SQUIRREL = 16,    // ecureuil qui bondit vers le renard
    E_JET = 17,         // jet de la fontaine : crachote, puis propulse vers le haut
    E_WIND = 18,        // zone de vent (rafales vers la gauche)
    E_ROCKSPAWN = 19,   // en haut d'une paroi : laisse tomber des pierres
    E_ROCK = 20,        // pierre qui tombe
    E_HUT = 21;         // decor : la chata (refuge) des Tatras

  const S_TITLE = 0, S_INTRO = 1, S_PLAY = 2, S_PAUSE = 3, S_DYING = 4, S_CLEAR = 5, S_GAMEOVER = 6, S_END = 7;

  const ifloor = Math.floor;
  const lroundf = (v) => (v < 0 ? -Math.round(-v) : Math.round(v));
  const overlap = (ax, ay, aw, ah, bx, by, bw, bh) =>
    ax < f(bx + bw) && f(ax + aw) > bx && ay < f(by + bh) && f(ay + ah) > by;
  const clock = (secs) => `${(secs / 60) | 0}:${String(secs % 60).padStart(2, '0')}`;

  // Sauvegarde : niveaux debloques, meilleurs temps et medovniky par niveau.
  const SAVE_MAGIC = 0x5A52554E, SAVE_VERSION = 3;
  class SaveData {
    constructor() {
      this.magic = SAVE_MAGIC;
      this.version = SAVE_VERSION;
      this.unlocked = 1;
      this.bestTime = [0, 0, 0, 0, 0, 0, 0, 0];     // en secondes, 0 = jamais termine
      this.bestHearts = [0, 0, 0, 0, 0, 0, 0, 0];
      this.bestSecrets = [0, 0, 0, 0, 0, 0, 0, 0];  // passages secrets trouves par niveau
    }
    static from(o) {
      const d = new SaveData();
      if (o && typeof o === 'object') {
        d.magic = o.magic; d.version = o.version; d.unlocked = o.unlocked | 0;
        for (let i = 0; i < 8; i++) {
          d.bestTime[i] = (o.bestTime && o.bestTime[i]) | 0;
          d.bestHearts[i] = (o.bestHearts && o.bestHearts[i]) | 0;
          d.bestSecrets[i] = (o.bestSecrets && o.bestSecrets[i]) | 0;
        }
      }
      return d;
    }
  }

  class Entity {
    constructor() { this.reset(); }
    reset() {
      this.type = E_NONE;
      this.x = 0; this.y = 0; this.vx = 0; this.vy = 0;   // x, y : coin haut-gauche de la boite de collision
      this.w = 0; this.h = 0;
      this.timer = 0;
      this.state = 0;
      this.dir = 1;
      this.baseX = 0; this.baseY = 0;
      this.text = null;
    }
  }

  // Position et pose d'un personnage, pour le trace du compagnon (sans collisions)
  class Snap {
    constructor() { this.x = 0; this.y = 0; this.anim = 0; this.frame = 0; this.left = false; }
  }

  class Player {
    constructor() {
      this.x = 0; this.y = 0; this.vx = 0; this.vy = 0;
      this.onGround = false;
      this.coyote = 0; this.jumpBuf = 0;
      this.facingLeft = false;
      this.invuln = 0;
      this.hp = 3;
      this.anim = 0; this.animTime = 0; this.animFrame = 0;
      this.riding = -1;     // index de l'entite qui le porte (tram, passerelle)
      this.squash = 0;
      this.climbing = false;   // sur une echelle
      this.climbDist = 0;
    }
  }

  class Game {
    // assets : voir assets.js (planches, themes, polices, niveaux)
    constructor(assets) {
      this.A = assets;
      this.ALL_LEVELS = assets.levels;
      this.NUM_LEVELS = assets.levels.length;
      this.audio_ = null;
      this.lvl_ = new Level();
      this.levelIndex_ = 0;
      this.p_ = new Player();
      this.ent_ = [];
      for (let i = 0; i < Game.MAX_ENT; i++) this.ent_.push(new Entity());
      this.nent_ = 0;
      this.state_ = S_TITLE;
      this.stateTime_ = 0;
      this.frame_ = 0;
      this.camX_ = 0; this.camY_ = 0; this.lookAhead_ = 0;
      this.hearts_ = 0; this.lives_ = 3; this.levelHearts_ = 0;
      this.checkX_ = 0; this.checkY_ = 0;
      this.levelTime_ = 0;
      this.bumpTx_ = -1; this.bumpTy_ = -1; this.bumpT_ = 0;
      this.shake_ = 0;
      this.volumeShow_ = 0;
      this.holdA_ = false;
      this.respawn_ = false;
      this.storage_ = null;
      this.save_ = new SaveData();
      this.sel_ = 0;          // niveau choisi sur l'ecran titre
      this.rabbit_ = false;   // personnage actif : false = le renard, true = Andrea
      // compagnon : l'autre personnage suit le trace du personnage actif, avec un retard de TRAIL_DELAY images
      this.trail_ = [];
      for (let i = 0; i < Game.TRAIL_LEN; i++) this.trail_.push(new Snap());
      this.trailHead_ = 0;
      this.compHp_ = 3;       // points de vie du compagnon (echanges avec ceux du personnage actif)
      this.compDown_ = 0;     // images avant le retour du compagnon tombe (0 = debout)
      this.switched_ = false; // un personnage vient de tomber : ne pas traiter le reste de la logique
      // passages secrets du niveau : faux murs qui disparaissent quand on y entre
      this.secrets_ = [];
      for (let i = 0; i < 16; i++) this.secrets_.push({ revealed: false, t0: 0, ox: 0, oy: 0 });
      this.secretsFound_ = 0;
      this.crumbles_ = [];    // { tx, ty, t }
      this.ncrumbles_ = 0;
      this.weatherLevel_ = 0; // 0 rien, 1 pluie, 2 pluie et nuit
      this.windT_ = 0;        // dans une rafale (images restantes)
      this.flash_ = 0; this.thunderIn_ = 0; this.nextBolt_ = 240;
      this.rng_ = 12345;
    }

    rnd() {
      this.rng_ = (Math.imul(this.rng_, 1103515245) + 12345) >>> 0;
      return this.rng_ >>> 8;
    }

    // ---------------------------------------------------------------- mise en route
    begin(audio, storage) {
      this.audio_ = audio;
      if (storage) this.storage_ = storage;
      if (this.storage_) {
        const d = this.storage_.load();
        if (d && d.magic === SAVE_MAGIC && d.version === SAVE_VERSION) this.save_ = SaveData.from(d);
      }
      if (this.save_.unlocked < 1) this.save_.unlocked = 1;
      if (this.save_.unlocked > this.NUM_LEVELS) this.save_.unlocked = this.NUM_LEVELS;
      if (this.sel_ >= this.save_.unlocked) this.sel_ = this.save_.unlocked - 1;
      this.state_ = S_TITLE;
      this.stateTime_ = 0;
      this.audio_.music(this.ALL_LEVELS[this.sel_].music);
      // le niveau choisi sert de decor a l'ecran titre
      this.startLevel(this.sel_, true);
      this.camX_ = 0;
    }

    selectCharacter(rabbit) { this.rabbit_ = rabbit; }   // simulateur : Andrea (true) ou le renard (avant le debut)
    selectLevel(i) { this.sel_ = i; }                    // simulateur : niveau choisi sur l'ecran titre
    frameCount() { return this.frame_; }
    state() { return this.state_; }
    player() { return this.p_; }
    hearts() { return this.hearts_; }
    lives() { return this.lives_; }
    level() { return this.lvl_; }
    entities() { return this.ent_; }
    camX() { return this.camX_; }
    companionDown() { return this.compDown_ > 0; }

    levelCleared() {
      const i = this.levelIndex_, s = this.save_;
      const secs = (this.levelTime_ / 60) | 0;
      if (i < 8) {
        if (s.bestTime[i] === 0 || secs < s.bestTime[i]) s.bestTime[i] = secs;
        if (this.levelHearts_ > s.bestHearts[i]) s.bestHearts[i] = this.levelHearts_ > 255 ? 255 : this.levelHearts_;
        if (this.secretsFound_ > s.bestSecrets[i]) s.bestSecrets[i] = this.secretsFound_;
      }
      if (i + 2 > s.unlocked) s.unlocked = i + 2 > this.NUM_LEVELS ? this.NUM_LEVELS : i + 2;
      if (this.storage_) this.storage_.save(s);
    }

    spawn(t, x, y) {
      let slot = -1;
      for (let i = 0; i < this.nent_; i++) {
        if (this.ent_[i].type === E_NONE) { slot = i; break; }
      }
      if (slot < 0) {
        if (this.nent_ >= Game.MAX_ENT) return -1;
        slot = this.nent_++;
      }
      const e = this.ent_[slot];
      e.reset();
      e.type = t;
      e.x = f(x);
      e.y = f(y);
      e.baseX = e.x;
      e.baseY = e.y;
      return slot;
    }

    fx(t, x, y, text) {
      const i = this.spawn(t, x, y);
      if (i < 0) return;
      this.ent_[i].timer = t === E_TEXT ? 60 : (t === E_DUST ? 15 : 14);
      this.ent_[i].text = text === undefined ? null : text;
    }

    startLevel(index, fresh) {
      this.levelIndex_ = index;
      const lvl = this.lvl_;
      lvl.load(this.ALL_LEVELS[index]);
      this.nent_ = 0;
      const ent = this.ent_;
      for (let i = 0; i < lvl.nspawns; i++) {
        const s = lvl.spawns[i];
        const tx = s.tx * TILE, ty = s.ty * TILE;
        let k = -1;
        switch (s.kind) {
          case 'o': k = this.spawn(E_HEART, tx + 2, ty + 2); ent[k].w = 12; ent[k].h = 12; break;
          case 'p': k = this.spawn(E_PIGEON, tx - 9, ty - 8); ent[k].w = 34; ent[k].h = 24; ent[k].vx = -0.5; break;
          case 'w': k = this.spawn(E_WASP, tx - 3, ty); ent[k].w = 22; ent[k].h = 16; break;
          case 'P': k = this.spawn(E_SPRING, tx + 1, ty + 6); ent[k].w = 14; ent[k].h = 10; break;
          case 'C': k = this.spawn(E_CHECK, tx, ty + TILE - 48); ent[k].w = 16; ent[k].h = 48; break;
          case 'F': k = this.spawn(E_GOAL, tx, ty + TILE - 48); ent[k].w = 52; ent[k].h = 48; break;
          case 'T': k = this.spawn(E_TRAM, tx, ty + TILE - 48); ent[k].w = 96; ent[k].h = 48; ent[k].dir = 1; break;
          case 'M':
          case 'V':
            k = this.spawn(E_PLATFORM, tx, ty);
            ent[k].w = PLATFORM_W;
            ent[k].h = PLATFORM_H;
            ent[k].state = s.kind === 'V' ? 1 : 0;
            ent[k].dir = 1;   // l'ascenseur ('V') attend en bas qu'on monte dessus
            break;
          case 'b': k = this.spawn(E_BOTTLE, tx + 3, ty + 2); ent[k].w = 10; ent[k].h = 14; break;
          case 's': k = this.spawn(E_SQUIRREL, tx - 2, ty - 4); ent[k].w = 20; ent[k].h = 20; ent[k].dir = -1; break;
          case 'J':
            k = this.spawn(E_JET, tx, ty);
            ent[k].w = 12;
            ent[k].h = JET_H;
            ent[k].timer = (s.tx * 37) % JET_PERIOD;   // jets decales les uns des autres
            break;
          case 'z': k = this.spawn(E_WIND, tx, ty + TILE - WIND_H); ent[k].w = WIND_W; ent[k].h = WIND_H; break;
          case 'r': k = this.spawn(E_ROCKSPAWN, tx, ty); ent[k].timer = (s.tx * 23) % ROCK_PERIOD; break;
          case 'R': k = this.spawn(E_HUT, tx, ty + TILE - 64); break;
          case 'l': k = this.spawn(E_BENCH, tx, ty); break;
          case 'k': k = this.spawn(E_FLOWERS, tx, ty); break;
          default: break;
        }
      }
      this.respawn_ = false;
      for (const sr of this.secrets_) { sr.revealed = false; sr.t0 = 0; sr.ox = 0; sr.oy = 0; }
      this.secretsFound_ = 0;
      this.ncrumbles_ = 0;
      this.crumbles_ = [];
      this.weatherLevel_ = 0;
      this.flash_ = this.thunderIn_ = 0;
      if (fresh) {
        this.checkX_ = lvl.startX;
        this.checkY_ = lvl.startY;
        this.levelHearts_ = 0;
        this.levelTime_ = 0;
      }
      this.p_ = new Player();
      this.p_.x = f(this.checkX_ + 1);
      this.p_.y = f(this.checkY_ - Game.PH);
      this.compHp_ = 3;
      this.compDown_ = 0;
      this.switched_ = false;
      this.resetTrail();
      this.updateCamera(true);
    }

    debugTeleport(x, feetY) {
      if (feetY === undefined) feetY = -1;
      this.checkX_ = f(x);
      this.checkY_ = feetY >= 0 ? f(feetY) : this.lvl_.startY;
      this.p_.x = f(x);
      this.p_.y = f(this.checkY_ - Game.PH);
      this.p_.vx = this.p_.vy = 0;
      this.resetTrail();
      this.updateCamera(true);
    }

    // ---------------------------------------------------------------- compagnon
    duo() { return this.levelIndex_ >= 1; }   // a partir du niveau 2, les deux personnages avancent ensemble

    resetTrail() {
      const p = this.p_;
      for (let i = 0; i < Game.TRAIL_LEN; i++) {
        const s = this.trail_[i];
        s.x = p.x; s.y = p.y; s.anim = p.anim; s.frame = p.animFrame; s.left = p.facingLeft;
      }
      this.trailHead_ = 0;
    }

    recordTrail() {
      const s = this.trail_[this.trailHead_], p = this.p_;
      s.x = p.x; s.y = p.y; s.anim = p.anim; s.frame = p.animFrame; s.left = p.facingLeft;
      this.trailHead_ = (this.trailHead_ + 1) % Game.TRAIL_LEN;
    }

    // position du personnage actif, 'age' images plus tot
    trailAt(age) {
      const i = (this.trailHead_ - 1 - age + 2 * Game.TRAIL_LEN) % Game.TRAIL_LEN;
      return this.trail_[i];
    }

    // bascule sur le compagnon, la ou il se trouve sur le trace ; les points de vie suivent chaque personnage
    switchChar(onDeath) {
      const s = this.trailAt(Game.TRAIL_DELAY), p = this.p_;
      const sx = s.x, sy = s.y, sleft = s.left;
      const hp = p.hp;
      p.hp = this.compHp_;
      this.compHp_ = hp;
      p.x = sx;
      p.y = sy;
      p.vx = p.vy = 0;
      p.onGround = false;
      p.coyote = p.jumpBuf = 0;
      p.climbing = false;
      p.riding = -1;
      p.squash = 0;
      p.facingLeft = sleft;
      p.anim = A_IDLE;
      p.animTime = p.animFrame = 0;
      p.invuln = onDeath ? 60 : 0;   // pas de coup immediat sur la meme bete
      this.rabbit_ = !this.rabbit_;
      if (onDeath) this.compDown_ = Game.REVIVE_FRAMES;
      this.audio_.play(SFX_BUMP);
    }

    // ---------------------------------------------------------------- boucle logique
    step(inp) {
      this.frame_ = (this.frame_ + 1) >>> 0;
      this.stateTime_++;
      if (this.volumeShow_ > 0) this.volumeShow_--;
      if (inp.hit(BTN_VOLUME)) {
        this.audio_.setVolume((this.audio_.volume() + 1) % 5);
        this.volumeShow_ = 90;
        this.audio_.play(SFX_COIN);
      }
      const p = this.p_;
      switch (this.state_) {
        case S_TITLE:
          this.camX_ = f(this.camX_ + F06);
          if (this.camX_ > this.lvl_.pixelW() - SCREEN_W) this.camX_ = 0;
          if (this.save_.unlocked > 1 && (inp.hit(BTN_LEFT) || inp.hit(BTN_RIGHT))) {
            const n = this.sel_ + (inp.hit(BTN_RIGHT) ? 1 : -1);
            if (n >= 0 && n < this.save_.unlocked && n < this.NUM_LEVELS) {
              this.sel_ = n;
              this.startLevel(this.sel_, true);
              this.camX_ = 0;
              this.audio_.music(this.lvl_.def.music);
              this.audio_.play(SFX_BUMP);
            }
          }
          if (inp.hit(BTN_START) || inp.hit(BTN_A)) {
            if (this.sel_ === 0) this.rabbit_ = false;   // au niveau 1 le renard joue seul : Andrea arrive a la fin
            this.lives_ = 3;
            this.hearts_ = 0;
            this.startLevel(this.sel_, true);
            this.state_ = S_INTRO;
            this.stateTime_ = 0;
            this.audio_.music(-1);
            this.audio_.play(SFX_PAUSE);
          }
          break;
        case S_INTRO:
          if (this.stateTime_ > (this.respawn_ ? 70 : 130) || (this.stateTime_ > 30 && inp.hit(BTN_A | BTN_START))) {
            this.state_ = S_PLAY;
            this.stateTime_ = 0;
            this.audio_.music(this.lvl_.def.music);
          }
          break;
        case S_PLAY:
          if (inp.hit(BTN_START)) {
            this.state_ = S_PAUSE;
            this.audio_.music(-1);
            this.audio_.play(SFX_PAUSE);
            break;
          }
          if (inp.hit(BTN_SELECT) && this.duo() && this.compDown_ === 0) this.switchChar(false);
          this.updatePlay(inp);
          break;
        case S_PAUSE:
          if (inp.hit(BTN_START)) {
            this.state_ = S_PLAY;
            this.audio_.music(this.lvl_.def.music);
            this.audio_.play(SFX_PAUSE);
          } else if (inp.hit(BTN_SELECT)) {
            this.begin(this.audio_);
          }
          break;
        case S_DYING:
          p.vy = f(p.vy + GRAVITY);
          if (this.stateTime_ > 25) p.y = f(p.y + p.vy);
          this.updateEntities();
          if (this.stateTime_ > 130) {
            this.lives_--;
            if (this.lives_ <= 0) {
              this.state_ = S_GAMEOVER;
              this.stateTime_ = 0;
              this.audio_.play(SFX_GAMEOVER);
            } else {
              this.p_ = new Player();
              this.p_.x = f(this.checkX_ + 1);
              this.p_.y = f(this.checkY_ - Game.PH);
              this.p_.invuln = INVULN;
              this.compHp_ = 3;
              this.compDown_ = 0;
              this.resetTrail();
              this.updateCamera(true);
              this.state_ = S_INTRO;
              this.stateTime_ = 0;
              this.respawn_ = true;
            }
          }
          break;
        case S_CLEAR:
          this.updateEntities();
          p.animTime++;
          if (this.stateTime_ > 150 && inp.hit(BTN_A | BTN_START)) {
            if (this.levelIndex_ + 1 < this.NUM_LEVELS) {
              this.startLevel(this.levelIndex_ + 1, true);
              this.state_ = S_INTRO;
            } else {
              this.state_ = S_END;
              this.audio_.music(this.lvl_.def.music);
            }
            this.stateTime_ = 0;
          }
          break;
        case S_GAMEOVER:
        case S_END:
          if (this.stateTime_ > 90 && inp.hit(BTN_START | BTN_A)) this.begin(this.audio_);
          break;
      }
      if (this.shake_ > 0) this.shake_--;
      if (this.bumpT_ > 0) this.bumpT_--;
    }

    updatePlay(inp) {
      this.levelTime_++;
      this.switched_ = false;
      this.updatePlayer(inp);
      this.recordTrail();
      if (this.compDown_ > 0 && --this.compDown_ === 0) {   // le personnage tombe revient, a cote de l'autre
        this.compHp_ = 3;
        const s = this.trailAt(Game.TRAIL_DELAY);
        this.fx(E_SPARK, f(f(s.x + Game.PW / 2) - 6), f(f(s.y + Game.PH) - 20));
        this.audio_.play(SFX_CHECK);
      }
      this.updateCrumbles();
      this.updateEntities();
      this.interact();
      this.updateWeather();
      if (this.p_.y > this.lvl_.pixelH() + 8 && !this.switched_) this.killPlayer();
      this.updateCamera(false);
    }

    updatePlayer(inp) {
      const p = this.p_;
      if (p.climbing || this.tryStartClimb(inp)) {
        if (p.invuln > 0) p.invuln--;
        this.updateClimb(inp);
        return;
      }
      let left = inp.down(BTN_LEFT), right = inp.down(BTN_RIGHT);
      const run = inp.down(BTN_B);
      let maxv = run ? RUN_MAX : WALK_MAX;
      if (this.windT_ > 0) {   // face au vent : on avance moins vite
        this.windT_--;
        maxv = f(maxv - F09);
      }
      const acc = p.onGround ? (run ? RUN_ACCEL : ACCEL) : AIR_ACCEL;
      if (p.invuln > INVULN - 20) { left = right = false; }  // recul apres un coup
      if (right && !left) {
        if (p.vx < 0 && p.onGround) p.vx = f(p.vx + SKID);
        else if (p.vx < maxv) p.vx = f(p.vx + acc);
        p.facingLeft = false;
      } else if (left && !right) {
        if (p.vx > 0 && p.onGround) p.vx = f(p.vx - SKID);
        else if (p.vx > -maxv) p.vx = f(p.vx - acc);
        p.facingLeft = true;
      } else if (p.onGround) {
        if (p.vx > 0) p.vx = p.vx > FRICTION ? f(p.vx - FRICTION) : 0;
        else if (p.vx < 0) p.vx = p.vx < -FRICTION ? f(p.vx + FRICTION) : 0;
      }
      // depasser la vitesse max (apres B relache) : on revient doucement
      if (p.vx > maxv) p.vx = f(p.vx - FRICTION * 0.5);
      if (p.vx < -maxv) p.vx = f(p.vx + FRICTION * 0.5);
      // poussiere au derapage
      if (p.onGround && ((right && p.vx < -F12) || (left && p.vx > F12)) && (this.frame_ % 5) === 0)
        this.fx(E_DUST, f(f(p.x + Game.PW / 2) - 8), f(f(p.y + Game.PH) - 14));

      // saut : tampon d'appui + tolerance apres le bord (coyote)
      if (inp.hit(BTN_A)) p.jumpBuf = JUMP_BUFFER;
      else if (p.jumpBuf > 0) p.jumpBuf--;
      if (p.onGround) p.coyote = COYOTE;
      else if (p.coyote > 0) p.coyote--;
      if (p.jumpBuf > 0 && p.coyote > 0) {
        p.vy = -f(JUMP_V + f(Math.abs(p.vx) * JUMP_SPEED_BONUS));
        p.onGround = false;
        p.coyote = 0;
        p.jumpBuf = 0;
        p.riding = -1;
        this.audio_.play(SFX_JUMP);
      }
      this.holdA_ = inp.down(BTN_A);
      const hold = this.holdA_ && p.vy < 0;
      // Andrea : A tenu en tombant = vol plane, chute lente
      const glide = this.rabbit_ && this.holdA_ && p.vy >= 0 && !p.onGround;
      if (glide) {
        p.vy = f(p.vy + GLIDE_GRAVITY);
        if (p.vy > GLIDE_FALL) p.vy = GLIDE_FALL;
      } else {
        p.vy = f(p.vy + (hold ? GRAVITY_HOLD : GRAVITY));
        if (p.vy > MAX_FALL) p.vy = MAX_FALL;
      }

      const wasGround = p.onGround;
      const prevBottom = f(p.y + Game.PH);
      this.movePlayerX(p.vx);
      p.onGround = false;
      this.movePlayerY(p.vy, prevBottom);
      if (p.onGround && !wasGround) {
        p.squash = 6;
        if (prevBottom < f(f(p.y + Game.PH) - 0.5) || p.vy === 0)
          this.fx(E_DUST, f(f(p.x + Game.PW / 2) - 8), f(f(p.y + Game.PH) - 14));
      }
      if (p.squash > 0) p.squash--;
      if (p.invuln > 0) p.invuln--;

      // animation
      let a;
      if (p.invuln > INVULN - 20) a = A_HURT;
      else if (!p.onGround) a = glide ? A_GLIDE : (p.vy < 0 ? A_JUMP : A_FALL);
      else if (Math.abs(p.vx) > F015) a = A_RUN;
      else a = A_IDLE;
      if (a !== p.anim) {
        p.anim = a;
        p.animTime = 0;
      }
      // la course s'accelere avec la vitesse
      let speed = a === A_RUN ? Math.trunc(f(8 - f(Math.abs(p.vx) * F14))) : 12;
      if (speed < 3) speed = 3;
      p.animTime++;
      p.animFrame = ((p.animTime / speed) | 0) % ANIM_LEN[a];
    }

    // ---------------------------------------------------------------- echelles
    tryStartClimb(inp) {
      const p = this.p_, lvl = this.lvl_;
      if (p.invuln > INVULN - 20) return false;
      const bottom = ifloor(p.y) + Game.PH;
      // colonne de l'echelle : celle du centre du corps, sinon celle d'un des bords
      const cols = [(ifloor(p.x) + Game.PW / 2 | 0) >> 4, ifloor(p.x) >> 4, (ifloor(p.x) + Game.PW - 1) >> 4];
      let cx = -1;
      for (let k = 0; k < 3 && cx < 0; k++) {
        const c = cols[k];
        if (inp.down(BTN_UP)) {
          let found = false;
          // une echelle derriere le corps (pas seulement au-dessus de la tete)
          for (let yy = bottom - 4; yy >= ifloor(p.y) + 12; yy -= 8)
            if (lvl.at(c, yy >> 4) === T_LADDER) found = true;
          // en haut d'une echelle, deja debout dessus : rien a monter
          if (found && p.onGround && lvl.ladderTop(c, bottom >> 4) && lvl.at(c, (bottom - 1) >> 4) !== T_LADDER)
            found = false;
          if (found) cx = c;
        } else if (inp.down(BTN_DOWN) && p.onGround && lvl.ladderTop(c, bottom >> 4)) {
          cx = c;
          p.y = f(p.y + 4);
        }
      }
      if (cx < 0) return false;
      p.climbing = true;
      p.vx = p.vy = 0;
      p.onGround = false;
      p.riding = -1;
      p.x = cx * TILE + TILE / 2 - Game.PW / 2;
      p.anim = A_CLIMB;
      return true;
    }

    updateClimb(inp) {
      const p = this.p_, lvl = this.lvl_;
      if (inp.hit(BTN_A)) {   // on saute de l'echelle
        p.climbing = false;
        p.vy = -4.5;
        p.vx = inp.down(BTN_LEFT) ? -2.0 : (inp.down(BTN_RIGHT) ? 2.0 : 0.0);
        if (p.vx) p.facingLeft = p.vx < 0;
        this.audio_.play(SFX_JUMP);
        return;
      }
      let dy = inp.down(BTN_UP) ? -CLIMB_V : (inp.down(BTN_DOWN) ? CLIMB_V : 0.0);
      const cx = (ifloor(p.x) + Game.PW / 2 | 0) >> 4;
      if (dy < 0 && lvl.kindAt(cx, ifloor(f(p.y + dy)) >> 4) === K_SOLID) dy = 0;   // plafond
      p.y = f(p.y + dy);
      p.climbDist = f(p.climbDist + (dy < 0 ? -dy : dy));
      const bottom = ifloor(p.y) + Game.PH;
      if (dy > 0) {   // en bas : on pose le pied
        const t = lvl.at(cx, bottom >> 4);
        const k = kind(t);
        if (t !== T_LADDER && (k === K_SOLID || k === K_ONEWAY)) {
          p.y = (bottom >> 4) * TILE - Game.PH;
          p.climbing = false;
          p.onGround = true;
          return;
        }
      }
      const onLadder = lvl.at(cx, (bottom - 1) >> 4) === T_LADDER ||
        lvl.at(cx, (ifloor(p.y) + Game.PH / 2) >> 4) === T_LADDER;
      if (!onLadder) {
        if (lvl.ladderTop(cx, bottom >> 4)) {   // en haut : debout sur l'echelle
          p.y = (bottom >> 4) * TILE - Game.PH;
          p.onGround = true;
        }
        p.climbing = false;
        return;
      }
      p.anim = A_CLIMB;
      p.animFrame = ((p.climbDist / 8) | 0) % 2;
    }

    solidAt(px, py) { return this.lvl_.kindAt(px >> 4, py >> 4) === K_SOLID; }

    movePlayerX(dx) {
      const p = this.p_, lvl = this.lvl_;
      p.x = f(p.x + dx);
      const top = ifloor(p.y) >> 4, bot = (ifloor(p.y) + Game.PH - 1) >> 4;
      if (dx > 0) {
        const tx = (ifloor(p.x) + Game.PW - 1) >> 4;
        for (let ty = top; ty <= bot; ty++) {
          if (lvl.kindAt(tx, ty) === K_SOLID) {
            p.x = tx * TILE - Game.PW;
            p.vx = 0;
            break;
          }
        }
      } else if (dx < 0) {
        const tx = ifloor(p.x) >> 4;
        for (let ty = top; ty <= bot; ty++) {
          if (lvl.kindAt(tx, ty) === K_SOLID) {
            p.x = (tx + 1) * TILE;
            p.vx = 0;
            break;
          }
        }
      }
      if (p.x < 0) p.x = 0;
    }

    movePlayerY(dy, prevBottom) {
      const p = this.p_, lvl = this.lvl_, PW = Game.PW, PH = Game.PH;
      p.y = f(p.y + dy);
      const l = ifloor(p.x) >> 4, r = (ifloor(p.x) + PW - 1) >> 4;
      if (dy > 0) {
        const bottom = f(p.y + PH);
        // la case qui contient le bord bas : on atterrit des qu'on y entre, meme d'une fraction de pixel
        const ty = ifloor(bottom) >> 4;
        for (let tx = l; tx <= r; tx++) {
          const k = lvl.kindAt(tx, ty);
          const top = k === K_ONEWAY || (k === K_LADDER && lvl.ladderTop(tx, ty));
          if (k === K_SOLID || (top && prevBottom <= f(ty * TILE + F01))) {
            p.y = ty * TILE - PH;
            p.vy = 0;
            p.onGround = true;
            p.riding = -1;
            return;
          }
        }
        // toit du tram, passerelles mobiles
        for (let i = 0; i < this.nent_; i++) {
          const e = this.ent_[i];
          if (e.type !== E_TRAM && e.type !== E_PLATFORM) continue;
          const roof = this.platformTop(e);
          const tol = e.type === E_PLATFORM ? 2.0 : 0.5;
          const inset = e.type === E_PLATFORM ? 0 : 4;
          if (prevBottom <= f(roof + tol) && bottom >= roof && f(p.x + PW) > f(e.x + inset) && p.x < f(f(e.x + e.w) - inset)) {
            p.y = f(roof - PH);
            p.vy = 0;
            p.onGround = true;
            p.riding = i;
            return;
          }
        }
        p.riding = -1;
      } else if (dy < 0) {
        const ty = ifloor(p.y) >> 4;
        // la case la plus proche du centre est celle qu'on cogne
        const cx = (ifloor(p.x) + PW / 2 | 0) >> 4;
        const order = [cx, l, r];
        for (let k = 0; k < 3; k++) {
          const tx = order[k];
          if (tx < l || tx > r) continue;
          const tt = lvl.at(tx, ty);
          if ((lvl.kindAt(tx, ty) === K_SOLID || tt === T_HIDDEN) && ty >= 0) {
            p.y = (ty + 1) * TILE;
            p.vy = 0.5;
            if (tt === T_HIDDEN) {   // un bloc cache : il devient un bloc bonus, avec 5 medovniky
              lvl.set(tx, ty, T_BONUS);
              this.secretsFound_++;
              for (let n = 0; n < 4; n++) this.collectHeart(tx * TILE, ty * TILE - 12, true);
              this.fx(E_TEXT, tx * TILE - 12, ty * TILE - 26, 'Tajné!');
            }
            this.bumpTile(tx, ty);
            return;
          }
        }
      }
    }

    platformTop(e) { return e.type === E_TRAM ? f(e.y + TRAM_ROOF) : e.y; }

    bumpTile(tx, ty) {
      const t = this.lvl_.at(tx, ty);
      this.bumpTx_ = tx;
      this.bumpTy_ = ty;
      this.bumpT_ = 8;
      if (t === T_BONUS) {
        this.lvl_.set(tx, ty, T_BONUS_USED);
        const i = this.spawn(E_POP, tx * TILE + 2, ty * TILE - 10);
        if (i >= 0) {
          this.ent_[i].vy = -F36;
          this.ent_[i].timer = 34;
        }
        this.collectHeart(tx * TILE, ty * TILE - 12, true);
      } else {
        this.audio_.play(SFX_BUMP);
      }
      // un pigeon pose sur la case cognee est assomme
      for (let i = 0; i < this.nent_; i++) {
        const e = this.ent_[i];
        if (e.type === E_PIGEON && e.state === 0 && Math.abs(f(f(e.y + e.h) - ty * TILE)) < 2 &&
            f(e.x + e.w) > tx * TILE && e.x < tx * TILE + TILE) {
          e.state = 1;
          e.timer = 30;
          this.audio_.play(SFX_STOMP);
        }
      }
    }

    // entre dans un faux mur : tout le passage secret se revele
    revealSecrets() {
      const lvl = this.lvl_, p = this.p_;
      if (lvl.nsecrets === 0) return;
      const l = ifloor(p.x) >> 4, r = (ifloor(p.x) + Game.PW - 1) >> 4;
      const t = ifloor(p.y) >> 4, b = (ifloor(p.y) + Game.PH - 1) >> 4;
      for (let ty = t; ty <= b; ty++) {
        for (let tx = l; tx <= r; tx++) {
          if (lvl.at(tx, ty) !== T_SECRET) continue;
          const id = lvl.regionAt(tx, ty);
          if (id <= 0 || this.secrets_[id].revealed) continue;
          this.secrets_[id] = { revealed: true, t0: this.frame_, ox: tx, oy: ty };
          this.secretsFound_++;
          this.audio_.play(SFX_LIFE);
          this.fx(E_TEXT, f(p.x - 14), f(p.y - 16), 'Tajné!');
        }
      }
    }

    // un medovnik ou une bouteille dans un passage secret encore ferme n'est pas dessine
    hiddenBySecret(e) {
      if (e.type !== E_HEART && e.type !== E_BOTTLE) return false;
      const tx = Math.trunc(f(e.x + e.w / 2)) >> 4, ty = Math.trunc(f(e.y + e.h / 2)) >> 4;
      if (this.lvl_.at(tx, ty) !== T_SECRET) return false;
      return !this.secrets_[this.lvl_.regionAt(tx, ty)].revealed;
    }

    collectHeart(x, y, popped) {
      this.hearts_++;
      this.levelHearts_++;
      this.audio_.play(SFX_COIN);
      if (!popped) this.fx(E_SPARK, x, y);
      if (this.hearts_ % HEARTS_PER_LIFE === 0) {
        this.lives_++;
        this.audio_.play(SFX_LIFE);
        this.fx(E_TEXT, f(this.p_.x - 10), f(this.p_.y - 14), '+1 život');
      }
    }

    // ---------------------------------------------------------------- sol qui s'effondre
    updateCrumbles() {
      const p = this.p_, lvl = this.lvl_;
      // le renard debout sur une case fragile la met en route
      if (p.onGround) {
        const by = (ifloor(p.y) + Game.PH) >> 4;
        for (let tx = ifloor(p.x) >> 4; tx <= (ifloor(p.x) + Game.PW - 1) >> 4; tx++) {
          if (lvl.at(tx, by) !== T_CRUMBLE) continue;
          let known = false;
          for (let i = 0; i < this.ncrumbles_; i++)
            if (this.crumbles_[i].tx === tx && this.crumbles_[i].ty === by) known = true;
          if (!known && this.ncrumbles_ < 24) {
            this.crumbles_[this.ncrumbles_++] = { tx, ty: by, t: 0 };
          }
        }
      }
      for (let i = 0; i < this.ncrumbles_; i++) {
        const c = this.crumbles_[i];
        c.t++;
        if (c.t === CRUMBLE_DELAY) {
          lvl.set(c.tx, c.ty, T_EMPTY);
          this.fx(E_DUST, c.tx * TILE, c.ty * TILE);
          this.audio_.play(SFX_CRUMBLE);
        }
        if (c.t >= CRUMBLE_DELAY + CRUMBLE_BACK) {
          // revient, sauf si le renard est dedans
          const inside = overlap(p.x, p.y, Game.PW, Game.PH, c.tx * TILE, c.ty * TILE, TILE, TILE);
          if (inside) {
            c.t--;
            continue;
          }
          lvl.set(c.tx, c.ty, T_CRUMBLE);
          this.crumbles_[i--] = this.crumbles_[--this.ncrumbles_];
        }
      }
      this.crumbles_.length = this.ncrumbles_;
    }

    // ---------------------------------------------------------------- orage (niveau des Tatras)
    updateWeather() {
      const lvl = this.lvl_;
      if (!lvl.def.weather) return;
      const k = f(this.p_.x / lvl.pixelW());
      this.weatherLevel_ = k < 0.25 ? 0 : (k < F055 ? 1 : 2);
      if (this.flash_ > 0) this.flash_--;
      if (this.thunderIn_ > 0 && --this.thunderIn_ === 0) this.audio_.play(SFX_THUNDER);
      if (this.weatherLevel_ >= 1 && --this.nextBolt_ <= 0) {
        // d'abord l'eclair, ensuite le tonnerre (najprv blesk, potom hrom)
        this.flash_ = 5;
        this.thunderIn_ = 18 + this.rnd() % 30;
        this.shake_ = 4;
        this.nextBolt_ = (this.weatherLevel_ === 2 ? 170 : 330) + this.rnd() % 200;
      }
    }

    updateEntities() {
      const lvl = this.lvl_, p = this.p_, ent = this.ent_;
      for (let i = 0; i < this.nent_; i++) {
        const e = ent[i];
        switch (e.type) {
          case E_PIGEON:
            if (e.state === 0) {
              // demi-tour devant un mur, une butee ou un trou
              const nx = f(e.x + e.vx);
              const frontX = e.vx < 0 ? ifloor(nx) : ifloor(nx) + e.w - 1;
              const ftx = frontX >> 4, fty = (ifloor(e.y) + e.h - 1) >> 4;
              const wall = lvl.kindAt(ftx, fty) === K_SOLID || lvl.mark(ftx, fty);
              const hole = lvl.kindAt(ftx, fty + 1) === K_AIR;
              if (wall || hole) e.vx = -e.vx;
              else e.x = nx;
              e.timer++;
            } else if (--e.timer <= 0) {
              this.fx(E_SPARK, f(e.x + 2), f(e.y - 4));
              e.type = E_NONE;
            }
            break;
          case E_WASP:
            if (e.state === 0) {
              e.timer++;
              const t = f(e.timer * F025);
              const ox = e.x;
              e.x = f(e.baseX + f(f(Math.sin(t)) * 40));
              e.y = f(e.baseY + f(f(Math.sin(f(t * F31))) * 6));
              e.dir = e.x >= ox ? 1 : -1;
            } else {
              e.vy = f(e.vy + GRAVITY);
              e.y = f(e.y + e.vy);
              if (e.y > lvl.pixelH() + 20) e.type = E_NONE;
            }
            break;
          case E_SPRING:
            if (e.timer > 0) e.timer--;
            break;
          case E_TRAM: {
            if (e.timer > 0) {   // arret aux butees
              if (--e.timer === 0) this.audio_.play(SFX_BELL);
              break;
            }
            const dx = f(F09 * e.dir);
            const lead = e.dir > 0 ? f(f(e.x + e.w) + dx) : f(e.x + dx);
            const ltx = ifloor(lead) >> 4, lty = (ifloor(e.y) + e.h - 1) >> 4;
            if (lvl.mark(ltx, lty) || lvl.kindAt(ltx, lty) === K_SOLID) {
              e.dir = -e.dir;
              e.timer = 70;
              break;
            }
            e.x = f(e.x + dx);
            if (p.riding === i) {
              this.movePlayerX(dx);
              p.y = f(f(e.y + TRAM_ROOF) - Game.PH);
            }
            break;
          }
          case E_PLATFORM: {
            // l'ascenseur reste en haut tant que le renard est dessus
            if (e.state === 1 && e.dir > 0 && e.y < e.baseY && p.riding === i && e.timer < 30) e.timer = 30;
            if (e.timer > 0) {   // petite pause aux butees
              e.timer--;
              break;
            }
            let dx = 0, dy = 0;
            const tl = ifloor(e.x) >> 4;
            if (e.state === 0) {
              dx = f(PLATFORM_V * e.dir);
              const lead = e.dir > 0 ? f(f(e.x + e.w) + dx) : f(e.x + dx);
              const ltx = ifloor(lead) >> 4, lty = ifloor(e.y) >> 4;
              if (lvl.mark(ltx, lty) || lvl.kindAt(ltx, lty) === K_SOLID) {
                e.dir = -e.dir;
                e.timer = 30;
                break;
              }
            } else {
              // ascenseur : attend en bas qu'on monte dessus, monte, puis redescend tout seul
              const atBase = e.dir > 0 && Math.abs(e.y - e.baseY) < 0.5;
              if (atBase && e.y >= e.baseY) {
                e.y = e.baseY;
                if (p.riding !== i) break;
                e.dir = -1;
              }
              if (e.dir > 0 && f(e.y + PLATFORM_VV) >= e.baseY) {   // de retour en bas
                dy = f(e.baseY - e.y);
                e.y = e.baseY;
                if (p.riding === i && !p.climbing) p.y = f(e.y - Game.PH);
                break;
              }
              dy = f(PLATFORM_VV * e.dir);
              const lead = e.dir > 0 ? f(f(e.y + e.h) + dy) : f(e.y + dy);
              const lty = ifloor(lead) >> 4;
              if (lvl.mark(tl, lty) || lvl.kindAt(tl, lty) === K_SOLID || lty < 0 || lty >= lvl.h) {
                // arret pile sur la grille : le dessus de la passerelle au ras de la roche voisine
                if (e.dir < 0) e.y = (lty + 1) * TILE;
                if (p.riding === i && !p.climbing) p.y = f(e.y - Game.PH);
                e.dir = -e.dir;
                e.timer = 40;
                break;
              }
            }
            e.x = f(e.x + dx);
            e.y = f(e.y + dy);
            if (p.riding === i && !p.climbing) {
              if (dx) this.movePlayerX(dx);
              p.y = f(e.y - Game.PH);
            }
            break;
          }
          case E_SQUIRREL:
            if (e.state === 0) {
              e.vy = f(e.vy + GRAVITY);
              if (e.vy > MAX_FALL) e.vy = MAX_FALL;
              // en l'air : avance ; a terre : attend puis bondit vers le renard
              const nx = f(e.x + e.vx);
              const frontX = e.vx < 0 ? ifloor(nx) : ifloor(nx) + e.w - 1;
              const ftx = frontX >> 4, fty = (ifloor(e.y) + e.h - 1) >> 4;
              if (e.vx && (lvl.kindAt(ftx, fty) === K_SOLID || lvl.mark(ftx, fty))) e.vx = -e.vx;
              else e.x = nx;
              const prevB = f(e.y + e.h);
              e.y = f(e.y + e.vy);
              const bty = (ifloor(e.y) + e.h) >> 4;
              const bl = ifloor(e.x) >> 4, br = (ifloor(e.x) + e.w - 1) >> 4;
              let landed = false;
              for (let tx = bl; tx <= br && !landed; tx++) {
                const k = lvl.kindAt(tx, bty);
                const top = k === K_ONEWAY || (k === K_LADDER && lvl.ladderTop(tx, bty));
                if (e.vy >= 0 && (k === K_SOLID || (top && prevB <= f(bty * TILE + F01)))) landed = true;
              }
              if (landed) {
                e.y = bty * TILE - e.h;
                e.vy = 0;
                e.vx = 0;
                if (++e.timer > 70 && Math.abs(f(p.x - e.x)) < 150 && Math.abs(f(p.y - e.y)) < 90) {
                  e.dir = p.x < e.x ? -1 : 1;
                  e.vx = f(F13 * e.dir);
                  e.vy = -F42;
                  e.timer = 0;
                }
              }
              if (e.y > lvl.pixelH() || lvl.kindAt((ifloor(e.x) + 7) >> 4, (ifloor(e.y) + e.h - 2) >> 4) === K_WATER)
                e.type = E_NONE;
            } else if (--e.timer <= 0) {
              this.fx(E_SPARK, e.x, f(e.y - 4));
              e.type = E_NONE;
            }
            break;
          case E_BOTTLE:
            e.timer++;
            break;
          case E_JET:
            e.timer = (e.timer + 1) % JET_PERIOD;
            if (e.state > 0) e.state--;   // pas de double propulsion
            break;
          case E_WIND:
            e.timer = (e.timer + 1) % WIND_PERIOD;
            break;
          case E_ROCKSPAWN:
            if (Math.abs(f(p.x - e.x)) < 220 && ++e.timer >= ROCK_PERIOD) {
              e.timer = 0;
              const k = this.spawn(E_ROCK, e.x + 2, e.y);
              if (k >= 0) {
                ent[k].w = 12;
                ent[k].h = 12;
              }
            }
            break;
          case E_ROCK: {
            e.vy = f(e.vy + F022);
            if (e.vy > 6) e.vy = 6;
            e.y = f(e.y + e.vy);
            e.timer++;
            const bx = (ifloor(e.x) + 6) >> 4, by = (ifloor(e.y) + e.h) >> 4;
            const k = lvl.kindAt(bx, by);
            if (e.timer > 8 && (k === K_SOLID || k === K_WATER || e.y > lvl.pixelH())) {
              this.fx(E_DUST, f(e.x - 2), by * TILE - 14);
              e.type = E_NONE;
            }
            break;
          }
          case E_POP:
            e.vy = f(e.vy + F03);
            e.y = f(e.y + e.vy);
            if (--e.timer <= 0) {
              this.fx(E_SPARK, e.x, e.y);
              e.type = E_NONE;
            }
            break;
          case E_DUST:
          case E_SPARK:
            if (--e.timer <= 0) e.type = E_NONE;
            break;
          case E_TEXT:
            e.y = f(e.y - 0.5);
            if (--e.timer <= 0) e.type = E_NONE;
            break;
          case E_GOAL:
          case E_CHECK:
            e.timer++;
            break;
          default:
            break;
        }
      }
    }

    interact() {
      const p = this.p_, lvl = this.lvl_, PW = Game.PW, PH = Game.PH;
      this.revealSecrets();
      const px = p.x, py = p.y;
      // pointes
      const l = ifloor(px) >> 4, r = (ifloor(px) + PW - 1) >> 4;
      const t = ifloor(py) >> 4, b = (ifloor(py) + PH - 1) >> 4;
      for (let ty = t; ty <= b; ty++)
        for (let tx = l; tx <= r; tx++)
          if (lvl.kindAt(tx, ty) === K_HAZARD && overlap(px, py, PW, PH, tx * TILE + 1, ty * TILE + 7, 14, 9))
            this.hurtPlayer(tx * TILE + 8);
      // l'eau du torrent : plouf
      if (lvl.kindAt((ifloor(px) + PW / 2 | 0) >> 4, (ifloor(py) + PH - 4) >> 4) === K_WATER) {
        this.fx(E_DUST, f(f(px + PW / 2) - 8), (((ifloor(py) + PH - 4) >> 4) * TILE - 12));
        this.killPlayer();
        return;
      }

      for (let i = 0; i < this.nent_ && !this.switched_; i++) {
        const e = this.ent_[i];
        if (e.type === E_NONE) continue;
        switch (e.type) {
          case E_HEART:
            if (overlap(px, py, PW, PH, e.x, e.y, e.w, e.h)) {
              this.collectHeart(f(e.x - 2), f(e.y - 2), false);
              e.type = E_NONE;
            }
            break;
          case E_JET: {
            // colonne d'eau au-dessus de la buse, seulement quand le jet est plein
            if (e.timer < JET_FULL) break;
            const top = f(f(e.y + TILE) - JET_H);
            const inside = overlap(px, py, PW, PH, e.x + 2, top, e.w, JET_H);
            if (inside && e.state) e.state = 20;   // une seule propulsion tant qu'on reste dans la colonne
            if (inside && !e.state) {
              p.vy = this.holdA_ ? -JET_V_HOLD : -JET_V;
              p.onGround = false;
              p.riding = -1;
              p.climbing = false;
              p.coyote = 0;
              e.state = 20;
              this.audio_.play(SFX_SPRING);
            }
            break;
          }
          case E_WIND:
            if (e.timer > 30 && e.timer < WIND_ON && !p.climbing && overlap(px, py, PW, PH, e.x, e.y, e.w, e.h)) {
              // rafale de face : on avance encore en marchant, mieux en courant (B)
              p.vx = f(p.vx - (p.onGround ? F07 : F10));
              if (p.vx < -F16) p.vx = -F16;
              this.windT_ = 2;
            }
            break;
          case E_ROCK:
            if (overlap(f(px + 2), f(py + 2), PW - 4, PH - 4, e.x, e.y, e.w, e.h)) {
              this.hurtPlayer(f(e.x + 6));
              this.fx(E_DUST, f(e.x - 2), e.y);
              e.type = E_NONE;
            }
            break;
          case E_BOTTLE:
            if (overlap(px, py, PW, PH, e.x, e.y, e.w, e.h)) {
              if (p.hp < 3) p.hp++;
              this.audio_.play(SFX_CHECK);
              this.fx(E_SPARK, f(e.x - 3), f(e.y - 2));
              this.fx(E_TEXT, f(e.x - 14), f(e.y - 14), 'Voda!');
              e.type = E_NONE;
            }
            break;
          case E_PIGEON:
          case E_WASP:
          case E_SQUIRREL:
            if (e.state !== 0) break;
            if (overlap(f(px + 1), f(py + 2), PW - 2, PH - 2, e.x, e.y, e.w, e.h)) {
              if (p.vy > 0 && f(f(py + PH) - p.vy) <= f(e.y + 8)) {
                // on lui saute dessus
                e.state = 1;
                e.timer = 30;
                if (e.type === E_WASP) e.vy = -2;
                p.vy = this.holdA_ ? -STOMP_V_HOLD : -STOMP_V;
                p.coyote = 0;
                this.audio_.play(SFX_STOMP);
                this.fx(E_SPARK, f(f(e.x + (e.w >> 1)) - 8), f(e.y - 6));
              } else {
                this.hurtPlayer(f(e.x + (e.w >> 1)));
              }
            }
            break;
          case E_SPRING:
            if (p.vy >= 0 && overlap(px, py, PW, PH, e.x, e.y, e.w, e.h) && f(f(py + PH) - p.vy) <= f(e.y + 5)) {
              p.y = f(e.y - PH);
              p.vy = this.holdA_ ? -SPRING_V_HOLD : -SPRING_V;
              p.onGround = false;
              p.coyote = 0;
              e.timer = 12;
              this.audio_.play(SFX_SPRING);
            }
            break;
          case E_CHECK:
            if (e.state === 0 && overlap(px, py, PW, PH, e.x, e.y, e.w, e.h)) {
              e.state = 1;
              this.checkX_ = e.x;
              this.checkY_ = f(e.y + e.h);
              this.audio_.play(SFX_CHECK);
              this.fx(E_SPARK, e.x, f(e.y - 4));
            }
            break;
          case E_TRAM:
            if (p.riding !== i && p.invuln === 0 &&
                overlap(f(px + 2), f(py + 4), PW - 4, PH - 6, f(e.x + 4), f(f(e.y + TRAM_ROOF) + 6), e.w - 8, e.h - TRAM_ROOF - 8))
              this.hurtPlayer(f(e.x + (e.w >> 1)));
            break;
          case E_GOAL:
            if (overlap(px, py, PW, PH, e.x, e.y, e.w, e.h)) {
              this.state_ = S_CLEAR;
              this.stateTime_ = 0;
              this.levelCleared();
              p.vx = 0;
              p.climbing = false;
              p.anim = A_WIN;
              p.animTime = 0;
              p.facingLeft = false;
              this.audio_.music(-1);
              this.audio_.play(SFX_WIN);
            }
            break;
          default:
            break;
        }
      }
    }

    hurtPlayer(fromX) {
      const p = this.p_;
      if (p.invuln > 0 || this.state_ !== S_PLAY) return;
      p.hp--;
      this.shake_ = 8;
      if (p.hp <= 0) {
        this.killPlayer();
        return;
      }
      p.invuln = INVULN;
      p.climbing = false;
      p.vx = (f(p.x + Game.PW / 2) < fromX) ? -2.5 : 2.5;
      p.vy = -F35;
      p.onGround = false;
      p.riding = -1;
      this.audio_.play(SFX_HURT);
    }

    killPlayer() {
      if (this.state_ !== S_PLAY) return;
      if (this.duo() && this.compDown_ === 0) {   // l'autre est debout : on bascule, sans perdre de vie
        this.switchChar(true);
        this.switched_ = true;
        this.fx(E_TEXT, f(this.p_.x - 6), f(this.p_.y - 16), 'Hop!');
        return;
      }
      this.state_ = S_DYING;
      this.stateTime_ = 0;
      this.p_.vy = -6.5;
      this.p_.vx = 0;
      this.p_.anim = A_HURT;
      this.p_.riding = -1;
      this.p_.climbing = false;
      this.audio_.music(-1);
      this.audio_.play(SFX_DIE);
    }

    updateCamera(snap) {
      const p = this.p_, lvl = this.lvl_;
      let target = f(f(p.x + Game.PW / 2) - 150);
      this.lookAhead_ = f(this.lookAhead_ + f(f(f(p.vx * 22) - this.lookAhead_) * F04));
      target = f(target + this.lookAhead_);
      const maxX = lvl.pixelW() - SCREEN_W;
      const ty = f(f(p.y + Game.PH) - 160);
      const maxY = lvl.pixelH() - SCREEN_H;
      if (snap) {
        this.camX_ = target;
        this.camY_ = ty;
      } else {
        this.camX_ = f(this.camX_ + f(f(target - this.camX_) * F18));
        this.camY_ = f(this.camY_ + f(f(ty - this.camY_) * F12c));
      }
      if (this.camX_ < 0) this.camX_ = 0;
      if (this.camX_ > maxX) this.camX_ = maxX;
      if (this.camY_ > maxY) this.camY_ = maxY;
      if (this.camY_ < 0) this.camY_ = 0;
    }

    // ---------------------------------------------------------------- rendu
    renderBand(b) {
      this.drawWorld(b);
      this.postProcess(b);
      if (this.state_ === S_TITLE) {
        this.drawTitle(b);
        return;
      }
      this.drawHud(b);
      this.drawOverlay(b);
    }

    // pluie, nuit qui tombe et eclairs (niveau des Tatras), sur le decor mais sous le HUD
    postProcess(b) {
      if (!this.lvl_.def.weather || this.state_ === S_TITLE) return;
      const n = SCREEN_W * b.h, buf = b.buf, frame_ = this.frame_;
      if (this.flash_ > 0) {
        for (let i = 0; i < n; i++) buf[i] |= 0x39E7;   // tout s'eclaire
      } else if (this.weatherLevel_ === 2) {
        for (let i = 0; i < n; i++) buf[i] = (buf[i] >> 1) & 0x7BEF;   // moitie moins de lumiere
      } else if (this.weatherLevel_ === 1) {
        for (let i = 0; i < n; i++) {
          const c = buf[i];
          buf[i] = c - ((c >> 2) & 0x39E7);   // un quart de moins
        }
      }
      if (this.weatherLevel_ >= 1) {
        // pluie : traits obliques qui tombent vite, plus nombreux quand l'orage est la
        const drops = this.weatherLevel_ === 2 ? 90 : 45;
        const col = this.weatherLevel_ === 2 ? 0x8C71 : 0xAD97;
        for (let i = 0; i < drops; i++) {
          let x = ((Math.imul(i, 2654435761) >>> 7) % 340) - (((frame_ * 3) >>> 0) % 340);
          const y = ((((Math.imul(i, 40503) + Math.imul(frame_, 11)) >>> 0) % 260)) - 10;
          x = ((x % 340) + 340) % 340 - 10;
          for (let k = 0; k < 6; k++) {
            const yy = y + k, xx = x - ((k / 2) | 0);
            if (yy < b.y0 || yy >= b.y0 + b.h || xx < 0 || xx >= SCREEN_W) continue;
            buf[(yy - b.y0) * SCREEN_W + xx] = col;
          }
        }
      }
    }

    drawWorld(b) {
      const lvl = this.lvl_, th = lvl.def.theme, frame_ = this.frame_;
      const cx = Math.trunc(this.camX_) + (this.shake_ ? ((frame_ & 2) ? 2 : -2) : 0);
      const cy = Math.trunc(this.camY_);
      b.wrapRow(th.far, (cx / 5) | 0, -((cy / 10) | 0));
      const ground = lvl.pixelH() - 2 * TILE - cy;
      b.wrapRow(th.mid, (cx / 2) | 0, ground - th.mid.h);
      // sous le niveau de la rue : terre sombre (les trous ne montrent pas le ciel)
      b.fillRect(0, ground, SCREEN_W, SCREEN_H - ground, th.under);
      b.fillRect(0, ground + 20, SCREEN_W, SCREEN_H - ground - 20, th.under2);

      // decor et objets derriere le joueur
      for (let i = 0; i < this.nent_; i++) {
        const e = this.ent_[i];
        if (e.type === E_BENCH || e.type === E_FLOWERS || e.type === E_CHECK || e.type === E_GOAL ||
            e.type === E_TRAM || e.type === E_HUT)
          this.drawEntity(b, e);
      }

      // tuiles
      const ts = th.tiles;
      const ty0 = (b.y0 + cy) >> 4, ty1 = (b.y0 + b.h - 1 + cy) >> 4;
      const tx0 = cx >> 4, tx1 = (cx + SCREEN_W - 1) >> 4;
      for (let ty = ty0; ty <= ty1; ty++) {
        for (let tx = tx0; tx <= tx1; tx++) {
          let t = lvl.at(tx, ty);
          if (t === T_EMPTY || t === T_HIDDEN || tx < 0 || tx >= lvl.w) continue;
          if (t === T_SECRET) {
            const sr = this.secrets_[lvl.regionAt(tx, ty)];
            if (sr.revealed) {
              // le mur s'efface en vague a partir de l'endroit ou l'on est entre
              const age = frame_ - sr.t0, d = Math.abs(tx - sr.ox) + Math.abs(ty - sr.oy);
              if (age >= 5 * d + 10) continue;
              if (age >= 5 * d && ((frame_ / 2) | 0) % 2) continue;
            }
            t = lvl.lookAt(tx, ty);
          }
          let dy = ty * TILE - cy;
          if (this.bumpT_ > 0 && tx === this.bumpTx_ && ty === this.bumpTy_) dy -= (this.bumpT_ > 4 ? 8 - this.bumpT_ : this.bumpT_);
          if (t === T_WATER_TOP && ((((frame_ / 16) | 0) + tx) & 1)) t = T_WATER_TOP2;   // vaguelettes
          let dx = 0;
          if (t === T_CRUMBLE) {
            for (let i = 0; i < this.ncrumbles_; i++) {
              const c = this.crumbles_[i];
              if (c.tx === tx && c.ty === ty && c.t < CRUMBLE_DELAY) {
                t = T_CRUMBLE_HIT;                               // ca se fissure...
                dx = ((c.t / 2) | 0) % 2 ? 1 : -1;               // ... et ca tremble
              }
            }
          }
          b.blit(ts, (t % 8) * TILE, ((t / 8) | 0) * TILE, TILE, TILE, tx * TILE - cx + dx, dy);
        }
      }

      for (let i = 0; i < this.nent_; i++) {
        const e = this.ent_[i];
        if (e.type === E_NONE || e.type === E_BENCH || e.type === E_FLOWERS || e.type === E_CHECK ||
            e.type === E_GOAL || e.type === E_TRAM || e.type === E_DUST || e.type === E_SPARK || e.type === E_TEXT ||
            e.type === E_HUT || this.hiddenBySecret(e))
          continue;
        this.drawEntity(b, e);
      }
      if (this.state_ !== S_TITLE) {
        if (this.duo()) this.drawFollower(b);
        this.drawPlayer(b);
      }
      for (let i = 0; i < this.nent_; i++) {
        const e = this.ent_[i];
        if (e.type === E_DUST || e.type === E_SPARK || e.type === E_TEXT) this.drawEntity(b, e);
      }
    }

    drawEntity(b, e) {
      const A = this.A, frame_ = this.frame_, lvl = this.lvl_;
      const cx = Math.trunc(this.camX_), cy = Math.trunc(this.camY_);
      const sx = lroundf(e.x) - cx, sy = lroundf(e.y) - cy;
      if (sx < -110 || sx > SCREEN_W + 10) return;
      if (sy > b.y0 + b.h || sy + (e.h > 64 ? e.h : 64) < b.y0) return;
      switch (e.type) {
        case E_HEART: {
          const bob = (((frame_ + Math.trunc(e.baseX)) / 20) | 0) % 2;
          b.frame(A.spr_medovnik, (((frame_ / 8) | 0) + ((Math.trunc(e.baseX) / 16) | 0)) % 4, sx - 2, sy - 2 - bob);
          break;
        }
        case E_POP:
          b.frame(A.spr_medovnik, ((frame_ / 4) | 0) % 4, sx - 2, sy - 2);
          break;
        case E_PIGEON:
          if (e.state === 0) b.frame(A.spr_holub, ((e.timer / 10) | 0) % 2, sx - 7, sy - 8, e.vx < 0);
          else b.frame(A.spr_holub, 2, sx - 7, sy - 8, e.vx < 0);
          break;
        case E_WASP:
          b.frame(A.spr_osa, ((frame_ / 3) | 0) % 2, sx - 5, sy - 8, e.dir < 0);
          break;
        case E_SPRING:
          b.frame(A.spr_pruzina, e.timer > 0 ? 1 : 0, sx - 1, sy - 6);
          break;
        case E_CHECK:
          b.frame(lvl.def.theme.check, e.state ? 1 : 0, sx, sy);
          break;
        case E_GOAL: {
          const cleared = this.state_ === S_CLEAR;
          const row = cleared ? A_WIN : A_IDLE;
          const f = cleared ? ((this.stateTime_ / 14) | 0) % 2 : ((e.timer / 14) | 0) % 4;
          const rabbit = A.spr_rabbit;
          if (lvl.def.npc) {
            // le personnage du niveau (pan Orol, le jardinier, le chatar), Andrea a cote
            b.frame(rabbit, row * rabbit.cols + f, sx, sy, true);
            b.frame(lvl.def.npc, cleared && this.stateTime_ > 40 ? ((this.stateTime_ / 20) | 0) % 2 : 0, sx + 26, sy);
          } else {
            // Andrea attend a l'arret de tram
            b.frame(A.spr_zastavka, 0, sx, sy);
            b.frame(rabbit, row * rabbit.cols + f, sx + 20, sy, true);
          }
          break;
        }
        case E_PLATFORM:
          b.frame(A.spr_platforma, 0, sx, sy);
          break;
        case E_JET: {
          // buse au sol, colonne d'eau au-dessus (image 0 eteint, 1 crachote, 2-3 plein)
          const f = e.timer < JET_SPUTTER ? 0 : (e.timer < JET_FULL ? 1 : 2 + ((frame_ / 4) | 0) % 2);
          b.frame(A.spr_tryska, f, sx, sy + TILE - A.spr_tryska.fh);
          break;
        }
        case E_WIND:
          if (e.timer > 30 && e.timer < WIND_ON) {
            // rafale : quelques traits blancs qui filent vers la gauche
            for (let i = 0; i < 9; i++) {
              const lx = sx + e.w - ((frame_ * 6 + i * 47) % (e.w + 20));
              const ly = sy + 10 + (i * 37) % (e.h - 20);
              b.fillRect(lx, ly, 10 + (i % 3) * 4, 1, 0xE73C);
            }
          }
          break;
        case E_ROCK:
          b.frame(A.spr_kamen, ((e.timer / 6) | 0) % 2, sx - 2, sy - 2);
          break;
        case E_HUT:
          b.frame(A.spr_chata, 0, sx, sy);
          break;
        case E_BOTTLE:
          b.frame(A.spr_flasa, 0, sx - 3, sy - 2 - (((frame_ / 20) | 0) % 2));
          break;
        case E_SQUIRREL:
          b.frame(A.spr_vevericka, e.state ? 2 : (e.vy < 0 || e.vx ? 1 : 0), sx - 6, sy - 12, e.dir < 0);
          break;
        case E_TRAM:
          if (e.dir < 0) {
            // le tram repart dans l'autre sens : le corps est retourne, mais le pantographe reste a sa place
            const im = A.spr_elektricka.img;
            b.blit(im, 0, TRAM_ROOF, im.w, im.h - TRAM_ROOF, sx, sy + TRAM_ROOF, true);
            b.blit(im, 0, 0, im.w, TRAM_ROOF, sx, sy, false);
          } else {
            b.frame(A.spr_elektricka, 0, sx, sy);
          }
          break;
        case E_BENCH:
          b.frame(lvl.def.theme.decor, 0, sx, sy);
          break;
        case E_FLOWERS:
          b.frame(lvl.def.theme.decor, 1, sx, sy);
          break;
        case E_DUST:
          b.frame(A.spr_fx, 2 - ((e.timer / 6) | 0), sx, sy);
          break;
        case E_SPARK:
          b.frame(A.spr_fx, 3 + ((e.timer / 7) | 0) % 2, sx, sy);
          break;
        case E_TEXT:
          if (e.text) b.text(A.fonts.font_small, sx, sy, e.text, YELLOW, SHADOW);
          break;
        default:
          break;
      }
    }

    drawPlayer(b) {
      const p = this.p_;
      if (p.invuln > 0 && p.invuln < INVULN - 20 && ((this.frame_ / 3) | 0) % 2 && this.state_ === S_PLAY) return;   // clignote
      let row = p.anim, f = p.animFrame;
      const flip = p.facingLeft;
      if (this.state_ === S_CLEAR) f = ((this.stateTime_ / 14) | 0) % 2;
      if (this.state_ === S_DYING) { row = A_HURT; f = 0; }
      this.drawChar(b, this.rabbit_, row, f, flip, p.x, p.y);
    }

    // le compagnon : son image est celle du trace, 10 images plus tot (il n'est pas dans la logique)
    drawFollower(b) {
      if (this.compDown_ > 0) return;
      const s = this.trailAt(Game.TRAIL_DELAY);
      this.drawChar(b, !this.rabbit_, s.anim, s.frame, s.left, s.x, s.y);
    }

    // dessine le renard (rabbit = false) ou Andrea ; (x, y) = coin haut-gauche de la boite de collision
    drawChar(b, rabbit, row, frameNo, flip, x, y) {
      const cx = Math.trunc(this.camX_), cy = Math.trunc(this.camY_);
      const hx = lroundf(f(x + Game.PW / 2)) - cx;
      const feet = lroundf(f(y + Game.PH)) - cy;
      const sh = rabbit ? this.A.spr_rabbit : this.A.spr_fox;   // meme format de cases (48x48) et memes lignes
      if (row === A_CLIMB && ((sh.img.h / sh.fh) | 0) <= A_CLIMB) {
        // pas encore d'images d'echelle : la pose du saut, retournee a chaque pas
        row = A_JUMP;
        flip = frameNo !== 0;
        frameNo = 0;
      }
      if (row === A_GLIDE && ((sh.img.h / sh.fh) | 0) <= A_GLIDE) {   // pas encore de planche de vol : la chute
        row = A_FALL;
        frameNo = 0;
      }
      const sx = flip ? hx - (sh.fw - 1 - FOX_ANCHOR) : hx - FOX_ANCHOR;
      b.frame(sh, row * sh.cols + frameNo, sx, feet - 47, flip);
    }

    drawHud(b) {
      if (b.y0 > 20) return;
      const A = this.A, small = A.fonts.font_small;
      for (let i = 0; i < 3; i++) b.frame(A.spr_hud, i < this.p_.hp ? 0 : 1, 6 + i * 10, 6);
      b.frame(A.spr_hud, 2, 44, 6);
      b.text(small, 55, 3, `${this.hearts_}`, WHITE, SHADOW);
      b.frame(A.spr_hud, 3, 88, 6);
      b.text(small, 98, 3, `×${this.lives_}`, WHITE, SHADOW);
      const secs = (this.levelTime_ / 60) | 0;
      const s = clock(secs);
      b.text(small, SCREEN_W - 8 - textWidth(small, s), 3, s, WHITE, SHADOW);
      const name = this.lvl_.def.name;
      b.text(small, SCREEN_W - 52 - textWidth(small, name), 3, name, 0xC618, SHADOW);
    }

    centerText(b, f, y, s, c, shadow) {
      b.text(f, ((SCREEN_W - textWidth(f, s)) / 2) | 0, y, s, c, shadow);
    }

    drawTitle(b) {
      const A = this.A, F = A.fonts, frame_ = this.frame_;
      // personnages au milieu de l'ecran
      const ground = SCREEN_H - 2 * TILE;
      const run = ((frame_ / 6) | 0) % 6;
      b.frame(A.spr_fox, A_RUN * A.spr_fox.cols + run, 136 - FOX_ANCHOR, ground - 47);
      b.frame(A.spr_rabbit, A_RUN * A.spr_rabbit.cols + (((frame_ / 8) | 0) + 2) % RABBIT_RUN_FRAMES, 168, ground - 47);
      // titre
      b.fillRect(0, 26, SCREEN_W, 58, 0x18E6);
      b.fillRect(0, 26, SCREEN_W, 2, ORANGE);
      b.fillRect(0, 82, SCREEN_W, 2, ORANGE);
      this.centerText(b, F.font_title, 30, 'ZAJKA RUN', ORANGE, BLACK);
      this.centerText(b, F.font_small, 66, 'Košice  →  Bratislava', WHITE, BLACK);
      if (this.save_.unlocked > 1) {
        // choix du niveau : < SVET 1-2 · Slovensky raj >
        const sel = this.sel_;
        const s = `${sel > 0 ? '‹' : ' '}  SVET 1-${sel + 1} · ${this.ALL_LEVELS[sel].name}  ${sel + 1 < this.save_.unlocked ? '›' : ' '}`;
        b.fillRect(0, 88, SCREEN_W, 16, 0x18E6);
        this.centerText(b, F.font_small, 89, s, YELLOW, BLACK);
      }
      if (((frame_ / 30) | 0) % 2 === 0) this.centerText(b, F.font_big, 108, 'Stlač START', WHITE, BLACK);
      b.fillRect(0, SCREEN_H - 14, SCREEN_W, 14, 0x18E6);
      this.centerText(b, F.font_small, SCREEN_H - 13, 'Slovingo · Zajka Friends', 0xBDF7, -1);
      if (this.volumeShow_) this.drawOverlay(b);
    }

    drawOverlay(b) {
      const A = this.A, F = A.fonts, frame_ = this.frame_, lvl = this.lvl_, save = this.save_;
      let s;
      switch (this.state_) {
        case S_INTRO: {
          if (this.respawn_) {
            // retour au point de passage : juste les vies
            b.fillRect(110, 100, 100, 36, DARKBLUE);
            b.frame(A.spr_hud, 3, 140, 113);
            b.text(F.font_big, 154, 106, `×${this.lives_}`, WHITE, BLACK);
            break;
          }
          const rec = this.levelIndex_ < 8 && save.bestTime[this.levelIndex_];
          b.fillRect(0, 70, SCREEN_W, rec ? 112 : 96, DARKBLUE);
          b.fillRect(0, 70, SCREEN_W, 2, ORANGE);
          b.fillRect(0, rec ? 180 : 164, SCREEN_W, 2, ORANGE);
          this.centerText(b, F.font_small, 78, `SVET 1-${this.levelIndex_ + 1}`, YELLOW, BLACK);
          this.centerText(b, F.font_title, 92, lvl.def.name, WHITE, BLACK);
          this.centerText(b, F.font_small, 128, lvl.def.subtitle, 0xBDF7, BLACK);
          b.frame(A.spr_hud, 3, 140, 148);
          b.text(F.font_small, 152, 145, `×${this.lives_}`, WHITE, BLACK);
          if (rec) {
            const bt = save.bestTime[this.levelIndex_];
            s = `Rekord  ${(bt / 60) | 0}:${String(bt % 60).padStart(2, '0')}  ·  medovníky : ${save.bestHearts[this.levelIndex_]}`;
            this.centerText(b, F.font_small, 163, s, YELLOW, BLACK);
          }
          break;
        }
        case S_PAUSE:
          b.fillRect(80, 80, 160, 80, DARKBLUE);
          b.fillRect(80, 80, 160, 2, ORANGE);
          this.centerText(b, F.font_big, 88, 'PAUZA', WHITE, BLACK);
          this.centerText(b, F.font_small, 118, 'START  pokračovať', 0xBDF7, -1);
          this.centerText(b, F.font_small, 134, 'SELECT  koniec', 0xBDF7, -1);
          break;
        case S_GAMEOVER:
          b.fillRect(0, 90, SCREEN_W, 60, DARKBLUE);
          this.centerText(b, F.font_title, 96, 'KONIEC HRY', WHITE, BLACK);
          if (this.stateTime_ > 90) this.centerText(b, F.font_small, 132, 'Stlač START', 0xBDF7, -1);
          break;
        case S_CLEAR: {
          // bulle d'Andrea
          if (this.stateTime_ > 40) {
            for (let i = 0; i < this.nent_; i++) {
              const e = this.ent_[i];
              if (e.type !== E_GOAL) continue;
              // la bulle sort de la bouche de pan Orol, ou d'Andrea a l'arret de tram
              const ax = Math.trunc(e.x) - Math.trunc(this.camX_) + (lvl.def.npc ? 44 : 36);
              const ay = Math.trunc(e.y) - Math.trunc(this.camY_) - 26;
              const hint = lvl.def.hint;
              const w = textWidth(F.font_small, hint) + 12;
              let bx = ax - ((w / 2) | 0);
              if (bx + w > SCREEN_W - 4) bx = SCREEN_W - 4 - w;
              b.fillRect(bx, ay, w, 18, WHITE);
              b.fillRect(bx - 1, ay + 1, 1, 16, WHITE);
              b.fillRect(bx + w, ay + 1, 1, 16, WHITE);
              b.fillRect(ax - 3, ay + 18, 7, 2, WHITE);
              b.fillRect(ax - 1, ay + 20, 3, 2, WHITE);
              b.text(F.font_small, bx + 6, ay + 3, hint, DARKBLUE, -1);
            }
          }
          if (this.stateTime_ > 90) {
            const total = lvl.nsecrets + lvl.nhidden;
            b.fillRect(60, 30, 200, total > 0 ? 90 : 74, DARKBLUE);
            b.fillRect(60, 30, 200, 2, ORANGE);
            this.centerText(b, F.font_big, 36, 'Výborne!', YELLOW, BLACK);
            this.centerText(b, F.font_small, 62, `Medovníky : ${this.levelHearts_}`, WHITE, -1);
            const secs = (this.levelTime_ / 60) | 0;
            this.centerText(b, F.font_small, 76, `Čas : ${clock(secs)}`, WHITE, -1);
            if (total > 0) {
              this.centerText(b, F.font_small, 90, `Tajomstvá : ${this.secretsFound_}/${total}`,
                this.secretsFound_ === total ? YELLOW : WHITE, -1);
            }
            if (this.stateTime_ > 150 && ((frame_ / 30) | 0) % 2 === 0)
              this.centerText(b, F.font_small, total > 0 ? 104 : 90, 'Stlač A', 0xBDF7, -1);
          }
          break;
        }
        case S_END:
          b.fillRect(0, 60, SCREEN_W, 110, DARKBLUE);
          this.centerText(b, F.font_big, 70, 'Pokračovanie nabudúce…', WHITE, BLACK);
          this.centerText(b, F.font_small, 100, `Ďalšia zastávka : ${lvl.def.next}`, 0xBDF7, -1);
          this.centerText(b, F.font_small, 120, `Medovníky : ${this.hearts_}`, YELLOW, -1);
          if (this.stateTime_ > 90) this.centerText(b, F.font_small, 146, 'Stlač START', WHITE, -1);
          break;
        default:
          break;
      }
      if (this.volumeShow_ > 0) {
        b.fillRect(100, 200, 120, 24, DARKBLUE);
        b.text(F.font_small, 108, 206, 'Hlasitosť', WHITE, -1);
        for (let i = 0; i < 4; i++) b.fillRect(170 + i * 10, 208, 7, 8, i < this.audio_.volume() ? YELLOW : 0x4208);
      }
    }
  }

  Game.PW = 14; Game.PH = 38;      // boite de collision du renard
  Game.MAX_ENT = 200;
  Game.TRAIL_LEN = 32; Game.TRAIL_DELAY = 10;
  Game.REVIVE_FRAMES = 150;        // 2,5 s avant le retour du personnage tombe

  Object.assign(R, {
    Game, SaveData, S_TITLE, S_INTRO, S_PLAY, S_PAUSE, S_DYING, S_CLEAR, S_GAMEOVER, S_END,
    E_NONE, E_HEART, E_PIGEON, E_WASP, E_TRAM, E_PLATFORM,
  });
})(globalThis);
