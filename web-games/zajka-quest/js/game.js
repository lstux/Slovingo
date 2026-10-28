// Zajka Quest : port de esp32/zajka-quest/src/game/{game,story,chapter1,chapter2,save}.{h,cpp}.
// Logique a 60 Hz (step), rendu par bandes (renderBand). La sauvegarde garde le meme format
// d'octets que le firmware (68 octets, somme de controle FNV-1a), mais dans le navigateur.
(function (root) {
  'use strict';
  const C = root.ZajkaCommon;
  const Q = root.ZajkaQuest = root.ZajkaQuest || {};
  const { SCREEN_W, SCREEN_H, BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_A, BTN_B, BTN_START, BTN_SELECT, BTN_VOLUME } = C;

  const WHITE = 0xFFFF, BLACK = 0x0000;
  const YELLOW = 0xFF2A, ORANGE = 0xFB00, DARKBLUE = 0x1926, GREY = 0xBDF7, DIM = 0x6B4D;
  const SKY_TOP = 0x2A4E, SKY_LOW = 0x5C9B, STONE = 0x8C51, STONE_DARK = 0x6B4D;
  const IDLE_ROW = 0, IDLE_FRAMES = 4, FOX_ANCHOR = 27;
  const FADE_HALF = 14;
  const BANNER_TIME = 150;
  const NUM_CHAPTERS = 2;

  const S_TITLE = 0, S_CHAPTER = 1, S_WORLD = 2, S_FADE = 3, S_DIALOG = 4, S_END = 5, S_NOTE = 6, S_MENU = 7;
  const AFTER_NOTHING = 0, AFTER_EPISODE = 1, AFTER_SLEEP = 2, AFTER_EVENT = 3;
  const SAVE_MAGIC = 0x3153515A, SAVE_VERSION = 3, SAVE_EPISODES = 16, SAVE_ITEMS = 16, SAVE_BITS = 8, SAVE_SIZE = 68;
  const DIR_DOWN = Q.DIR_DOWN, DIR_UP = Q.DIR_UP, DIR_RIGHT = Q.DIR_RIGHT;

  // ---------------------------------------------------------------- sauvegarde (save.h)
  function fnv(u8, n) {
    let h = 2166136261;
    for (let i = 0; i < n; i++) h = Math.imul(h ^ u8[i], 16777619) >>> 0;
    return h >>> 0;
  }

  const SaveData = {
    // d : { map, tx, ty, dir, chapter, stage, flags, items, picked, words, bonus }  ->  Uint8Array(68)
    encode(d) {
      const u = new Uint8Array(SAVE_SIZE), v = new DataView(u.buffer);
      v.setUint32(0, SAVE_MAGIC, true);
      u[4] = SAVE_VERSION; u[5] = d.map; u[6] = d.tx; u[7] = d.ty; v.setInt8(8, d.dir); u[9] = d.chapter; u[10] = d.stage;
      v.setUint16(12, d.flags, true);
      u.set(d.items, 16); u.set(d.picked, 32); u.set(d.words, 40); u.set(d.bonus, 48);
      v.setUint32(64, fnv(u, 64), true);
      return u;
    },
    // null si le bloc n'est pas valide
    decode(u) {
      if (!u || u.length !== SAVE_SIZE) return null;
      const v = new DataView(u.buffer, u.byteOffset, u.byteLength);
      if (v.getUint32(0, true) !== SAVE_MAGIC || u[4] !== SAVE_VERSION || v.getUint32(64, true) !== fnv(u, 64)) return null;
      return {
        map: u[5], tx: u[6], ty: u[7], dir: v.getInt8(8), chapter: u[9], stage: u[10], flags: v.getUint16(12, true),
        items: u.slice(16, 32), picked: u.slice(32, 40), words: u.slice(40, 48), bonus: u.slice(48, 64),
      };
    },
  };

  // stockage d'un bloc d'octets : localStorage (navigateur) ou memoire (tests)
  class BrowserByteStorage {
    constructor(key) { this.key = key; }
    read() {
      try {
        const s = root.localStorage.getItem(this.key);
        if (!s) return null;
        return Uint8Array.from(atob(s), (c) => c.charCodeAt(0));
      } catch (e) { return null; }
    }
    write(u8) {
      try { root.localStorage.setItem(this.key, btoa(String.fromCharCode(...u8))); return true; } catch (e) { return false; }
    }
    erase() { try { root.localStorage.removeItem(this.key); } catch (e) { /* ignore */ } }
  }

  class MemoryByteStorage {
    constructor() { this.buf = null; this.writes_ = 0; }
    read() { return this.buf ? this.buf.slice() : null; }
    write(u8) { this.buf = u8.slice(); this.writes_++; return true; }
    erase() { this.buf = null; }
    writes() { return this.writes_; }
  }

  // ---------------------------------------------------------------- drapeaux des chapitres
  const F1_TRAM_INTRO = 1 << 0, F1_TETA_INTRO = 1 << 1, F1_SLEPT = 1 << 2, F1_TICKET = 1 << 3, F1_DRIVER = 1 << 4,
    F1_STAMPED = 1 << 5, F1_CONTROL = 1 << 6, F1_CONTROL_OK = 1 << 7;
  const EV1_TICKET = 1, EV1_DRIVER = 2, EV1_STAMP = 3, EV1_CONTROL_BOARD = 4, EV1_CONTROL_OK = 5;
  const EP1_FIRST = 0, EP1_CONTROL = 2, EP1_TRAM_END = 5, EP1_TETA = 5;
  const F2_GIVEN = 1 << 0, F2_STATION_INTRO = 1 << 1, F2_NOTE = 1 << 2, F2_TICKETS = 1 << 3;
  const EV2_STATION = 1, EV2_NOTE = 2, EV2_NOTE_READ = 3, EV2_DROPS = 4, EV2_TICKETS = 5, EV2_BOARD = 6;
  const EP2_FIRST = 6, DROPS_NEEDED = 5;
  const S6_NOTE = 5, S6_DESK = 6, S6_TRAIN = 11;

  const CHAPTER_PLACES = [['Košice', 'To si ty?'], ['Sucho', 'Je horúco']];

  class Game {
    constructor(A) {
      this.A = A;
      this.K = A.K;
      this.world_ = new Q.World(A);
      this.dialog_ = new Q.Dialog(A);
      this.audio_ = null; this.storage_ = null;
      this.state_ = S_TITLE; this.stateTime_ = 0; this.frame_ = 0; this.volumeShow_ = 0;
      this.warp_ = null; this.autoWarp_ = null;
      this.lastTalk_ = 0; this.talks_ = 0;
      this.hasSave_ = false; this.saved_ = null;
      this.menu_ = 0; this.confirm_ = false; this.savedShow_ = 0;
      this.menuTab_ = 0; this.menuScroll_ = 0;
      this.noteTitle_ = ''; this.noteSk_ = ''; this.noteFr_ = ''; this.noteIcon_ = -1; this.noteFr_on_ = false;
      this.chapter_ = 0; this.stage_ = 0; this.flags_ = 0;
      this.items_ = new Uint8Array(SAVE_ITEMS); this.picked_ = new Uint8Array(SAVE_BITS);
      this.words_ = new Uint8Array(SAVE_BITS); this.bonus_ = new Array(SAVE_EPISODES).fill(0);
      this.after_ = AFTER_NOTHING; this.afterEvent_ = 0; this.banner_ = 0; this.bannerEp_ = 0;
    }

    // ---- acces pour les tests et la page
    state() { return this.state_; }
    world() { return this.world_; }
    lastTalk() { return this.lastTalk_; }
    talks() { return this.talks_; }
    dialog() { return this.dialog_; }
    chapter() { return this.chapter_; }
    stage() { return this.stage_; }
    itemCount(i) { return i >= 0 && i < SAVE_ITEMS ? this.items_[i] : 0; }
    hasSave() { return this.hasSave_; }
    frameCount() { return this.frame_; }
    debugChapter(c) { this.startChapter(c); }
    bonusFound() { let n = 0; for (const b of this.bonus_) n += b; return n; }
    bonusTotal(chapter) {
      if (chapter === undefined) chapter = -1;
      const eps = this.A.course.episodes;
      let n = 0;
      for (let i = 0; i < eps.length && i < SAVE_EPISODES; i++) {
        if (chapter < 0 || Math.trunc(i / 6) === chapter) n += eps[i].bonus.length;
      }
      return n;
    }
    wordsFound() {
      let n = 0;
      for (let i = 0; i < this.A.signWords.length; i++) if (this.words_[i >> 3] & (1 << (i & 7))) n++;
      return n;
    }
    goal() { return this.chapter_ === 0 ? this.ch1Goal() : this.ch2Goal(); }

    flag(f) { return (this.flags_ & f) !== 0; }
    setFlag(f) { this.flags_ = (this.flags_ | f) & 0xFFFF; }

    // ---------------------------------------------------------------- mise en route
    begin(audio, storage) {
      const K = this.K;
      this.audio_ = audio;
      this.storage_ = storage || null;
      this.chapter_ = 0; this.stage_ = 0; this.flags_ = 0;
      this.items_.fill(0); this.picked_.fill(0); this.words_.fill(0); this.bonus_.fill(0);
      this.world_.setPicked(this.picked_);
      this.world_.setFollower(K.SPK_NONE);
      this.after_ = AFTER_NOTHING;
      this.banner_ = 0;
      this.lastTalk_ = K.SPK_NONE;
      this.hasSave_ = false;
      const d = this.storage_ ? SaveData.decode(this.storage_.read()) : null;
      if (d && d.map < this.A.maps.length && d.chapter < NUM_CHAPTERS) {
        this.saved_ = d;
        const m = this.A.maps[d.map];
        this.hasSave_ = d.tx < m.w && d.ty < m.h && d.dir >= 0 && d.dir <= DIR_RIGHT;
      }
      this.menu_ = 0;
      this.confirm_ = false;
      this.setState(S_TITLE);
      this.audio_.music(0);
    }

    save() {
      if (!this.storage_) return;
      const w = this.world_, p = w.player();
      const d = {
        map: w.mapIndex(), tx: p.tx, ty: p.ty, dir: p.dir, chapter: this.chapter_, stage: this.stage_,
        flags: this.flags_, items: this.items_, picked: this.picked_, words: this.words_,
        bonus: Uint8Array.from(this.bonus_),
      };
      const u = SaveData.encode(d);
      if (this.storage_.write(u)) {
        this.saved_ = SaveData.decode(u);
        this.hasSave_ = true;
        this.savedShow_ = 70;
      }
    }

    newGame() {
      if (this.storage_ && this.hasSave_) this.storage_.erase();
      this.hasSave_ = false;
      this.items_.fill(0); this.picked_.fill(0); this.words_.fill(0); this.bonus_.fill(0);
      this.audio_.play(Q.SFX_PAUSE);
      this.startChapter(0);
    }

    // carte du chapitre, puis sa premiere carte (S_CHAPTER -> S_WORLD)
    startChapter(chapter) {
      this.chapter_ = chapter;
      this.stage_ = 0;
      this.flags_ = 0;
      this.world_.setFollower(this.K.SPK_NONE);
      this.audio_.music(-1);
      this.setState(S_CHAPTER);
    }

    continueGame() {
      if (!this.hasSave_) return false;
      const s = this.saved_;
      this.chapter_ = s.chapter; this.stage_ = s.stage; this.flags_ = s.flags;
      this.items_.set(s.items); this.picked_.set(s.picked); this.words_.set(s.words);
      for (let i = 0; i < SAVE_EPISODES; i++) this.bonus_[i] = s.bonus[i];
      this.world_.setFollower(this.K.SPK_NONE);
      this.world_.load(s.map, s.tx, s.ty, s.dir);
      this.audio_.play(Q.SFX_PAUSE);
      this.audio_.music(this.world_.map().music);
      this.setState(S_WORLD);
      this.storyRefresh();
      this.storyEnteredMap();   // ex. : une partie arretee juste avant un passage impose le refait
      return true;
    }

    // ecran titre : START ou A. Avec une partie enregistree : continuer ou recommencer (confirme)
    stepTitle(inp) {
      const ok = inp.hit(BTN_START) || inp.hit(BTN_A);
      if (!this.hasSave_) { if (ok) this.newGame(); return; }
      if (this.confirm_) {
        if (inp.hit(BTN_A)) this.newGame();
        else if (inp.hit(BTN_B) || inp.hit(BTN_START)) { this.confirm_ = false; this.audio_.play(Q.SFX_BUMP); }
        return;
      }
      if (inp.hit(BTN_UP) || inp.hit(BTN_DOWN)) { this.menu_ ^= 1; this.audio_.play(Q.SFX_COIN); }
      if (ok) {
        if (this.menu_ === 0) this.continueGame();
        else { this.confirm_ = true; this.audio_.play(Q.SFX_PAUSE); }
      }
    }

    setState(s) { this.state_ = s; this.stateTime_ = 0; }

    // ---------------------------------------------------------------- boucle logique
    step(inp) {
      const K = this.K, world = this.world_;
      this.frame_ = (this.frame_ + 1) >>> 0;
      this.stateTime_++;
      if (this.volumeShow_ > 0) this.volumeShow_--;
      if (this.savedShow_ > 0) this.savedShow_--;
      if (this.banner_ > 0 && this.state_ !== S_FADE) this.banner_--;
      if (inp.hit(BTN_VOLUME)) {
        this.audio_.setVolume((this.audio_.volume() + 1) % 5);
        this.volumeShow_ = 90;
        this.audio_.play(Q.SFX_COIN);
      }
      switch (this.state_) {
        case S_TITLE:
          this.stepTitle(inp);
          break;
        case S_CHAPTER:
          if (this.stateTime_ > 30 && (inp.hit(BTN_START) || inp.hit(BTN_A))) {
            this.audio_.play(Q.SFX_PAUSE);
            world.loadStart(this.chapterStartMap());
            this.audio_.music(world.map().music);
            this.setState(S_WORLD);
            this.storyRefresh();
            this.save();
            this.storyEnteredMap();
          }
          break;
        case S_WORLD:
          if (inp.hit(BTN_START)) {
            this.audio_.play(Q.SFX_PAUSE);
            this.menuScroll_ = 0;
            this.setState(S_MENU);
            break;
          }
          switch (world.step(inp)) {
            case Q.WE_WARP:
              if (!this.storyWarp(world.pendingWarp())) break;
              this.warp_ = world.pendingWarp();
              this.audio_.play(Q.SFX_CHECK);
              this.setState(S_FADE);
              break;
            case Q.WE_BUMP:
              this.audio_.play(Q.SFX_BUMP);
              break;
            case Q.WE_TALK:
              this.lastTalk_ = world.npc(world.talkNpc()).who;
              this.talks_++;
              this.storyTalk(world.talkNpc());
              break;
            case Q.WE_INSPECT:
              this.storyInspect(world.inspectX(), world.inspectY());
              break;
            case Q.WE_SIGN:
              this.storySign(world.event());
              break;
            case Q.WE_PICKUP:
              this.storyPickup(world.event());
              break;
            case Q.WE_ZONE:
              this.storyZone(world.event());
              break;
            default:
              break;
          }
          break;
        case S_FADE:
          if (this.stateTime_ === FADE_HALF && this.warp_) {
            const from = world.mapIndex(), w = this.warp_;
            world.load(w.target, w.tx, w.ty, w.dir);
            this.storyRefresh();
            if (world.map().music !== this.A.maps[from].music) this.audio_.music(world.map().music);
            this.warp_ = null;
          }
          if (this.stateTime_ >= 2 * FADE_HALF) {
            this.setState(S_WORLD);
            this.save();
            this.storyEnteredMap();
          }
          break;
        case S_DIALOG:
          this.dialog_.step(inp, this.audio_);
          if (!this.dialog_.active()) {
            this.setState(S_WORLD);
            this.storyDialogDone();   // peut changer d'etat (message, passage impose, fin du chapitre)
            if (this.state_ === S_WORLD || this.state_ === S_FADE || this.state_ === S_END) this.save();
          }
          break;
        case S_NOTE:
          if (inp.hit(BTN_SELECT)) { this.noteFr_on_ = !this.noteFr_on_; this.audio_.play(Q.SFX_PAUSE); }
          if (this.stateTime_ > 20 && (inp.hit(BTN_A) || inp.hit(BTN_B))) {
            this.setState(S_WORLD);
            this.save();
          }
          break;
        case S_MENU:
          this.stepMenu(inp);
          break;
        case S_END:
          if (this.stateTime_ > 120 && (inp.hit(BTN_A) || inp.hit(BTN_START))) {
            this.audio_.play(Q.SFX_PAUSE);
            if (this.chapter_ + 1 < NUM_CHAPTERS) this.startChapter(this.chapter_ + 1);   // on garde le sac et le Slovnik
            else this.begin(this.audio_, this.storage_);
          }
          break;
      }
    }

    chapterStartMap() { return this.chapter_ === 0 ? this.K.MAP_KOSICE_HLAVNA : this.K.MAP_SUCHO_ZAHRADA; }

    // ---------------------------------------------------------------- petit message
    note(title, sk, fr, icon) {
      this.noteTitle_ = title; this.noteSk_ = sk; this.noteFr_ = fr;
      this.noteIcon_ = icon === undefined ? -1 : icon;
      this.noteFr_on_ = false;
      this.setState(S_NOTE);
    }

    drawNote(b) {
      const A = this.A, small = A.fonts.font_small, big = A.fonts.font_big;
      const x = 40, y = 84, w = SCREEN_W - 80, h = 64;
      if (b.y0 > y + h || b.y0 + b.h < y) return;
      b.fillRect(x, y, w, h, DARKBLUE);
      b.fillRect(x, y, w, 2, ORANGE);
      b.fillRect(x, y + h - 2, w, 2, ORANGE);
      this.centerText(b, small, y + 6, this.noteTitle_, YELLOW, BLACK);
      let tx = x + 16;
      if (this.noteIcon_ >= 0) { b.frame(A.item_icons, this.noteIcon_, tx, y + 26); tx += 22; }
      b.text(big, tx, y + 22, this.noteSk_, WHITE, BLACK);
      if (this.noteFr_on_ && this.noteFr_) b.text(small, tx, y + 44, this.noteFr_, YELLOW, -1);
      else if (this.noteFr_) b.text(small, x + w - 86, y + h - 15, 'SELECT : FR', DIM, -1);
    }

    // ---------------------------------------------------------------- menu START
    // trois onglets : Batoh (objets), Slovnik (mots trouves), Ciel (objectif)
    stepMenu(inp) {
      if (inp.hit(BTN_B) || inp.hit(BTN_START)) {
        this.audio_.play(Q.SFX_PAUSE);
        this.setState(S_WORLD);
        return;
      }
      if (inp.hit(BTN_LEFT)) { this.menuTab_ = (this.menuTab_ + 2) % 3; this.menuScroll_ = 0; this.audio_.play(Q.SFX_COIN); }
      if (inp.hit(BTN_RIGHT)) { this.menuTab_ = (this.menuTab_ + 1) % 3; this.menuScroll_ = 0; this.audio_.play(Q.SFX_COIN); }
      if (inp.hit(BTN_DOWN)) this.menuScroll_++;
      if (inp.hit(BTN_UP) && this.menuScroll_ > 0) this.menuScroll_--;
      if (inp.hit(BTN_SELECT)) this.noteFr_on_ = !this.noteFr_on_;
    }

    vocabFr(sk) {
      for (const w of this.A.course.vocab) if (w.sk === sk) return w.fr;
      return '';
    }

    drawMenu(b) {
      const A = this.A, small = A.fonts.font_small, big = A.fonts.font_big;
      const x = 16, y = 16, w = SCREEN_W - 32, h = SCREEN_H - 32;
      b.fillRect(x, y, w, h, DARKBLUE);
      b.fillRect(x, y, w, 2, ORANGE);
      b.fillRect(x, y + h - 2, w, 2, ORANGE);
      const tabs = ['Batoh', 'Slovník', 'Cieľ'];
      const tw = Math.trunc(w / 3);
      for (let i = 0; i < 3; i++) {
        const cx = x + i * tw;
        if (i === this.menuTab_) b.fillRect(cx + 4, y + 6, tw - 8, 20, 0x2A4E);
        const tl = C.textWidth(big, tabs[i]);
        b.text(big, cx + Math.trunc((tw - tl) / 2), y + 7, tabs[i], i === this.menuTab_ ? YELLOW : GREY, BLACK);
      }
      b.fillRect(x + 8, y + 30, w - 16, 1, ORANGE);
      const ly = y + 38, rowH = 18, rows = 8;
      const nItems = A.items.length, words = A.signWords;
      if (this.menuTab_ === 0) {
        let shown = 0, idx = 0;
        for (let i = 0; i < nItems && i < SAVE_ITEMS; i++) {
          if (!this.items_[i]) continue;
          if (idx++ < this.menuScroll_) continue;
          if (shown >= rows) break;
          const yy = ly + shown * rowH;
          b.frame(A.item_icons, i, x + 12, yy);
          const s = this.items_[i] > 1 ? `${A.items[i].sk}  ×${this.items_[i]}` : A.items[i].sk;
          b.text(small, x + 34, yy + 2, s, WHITE, -1);
          b.text(small, x + 160, yy + 2, A.items[i].fr, GREY, -1);
          shown++;
        }
        if (idx === 0) b.text(small, x + 12, ly, 'Batoh je prázdny.', GREY, -1);
      } else if (this.menuTab_ === 1) {
        const s = `${this.wordsFound()} / ${words.length}`;
        b.text(small, x + w - 12 - C.textWidth(small, s), y + 36, s, YELLOW, -1);
        let shown = 0, idx = 0;
        for (let i = 0; i < words.length; i++) {
          if (!(this.words_[i >> 3] & (1 << (i & 7)))) continue;
          if (idx++ < this.menuScroll_) continue;
          if (shown >= rows) break;
          const yy = ly + 4 + shown * rowH;
          b.text(small, x + 12, yy, words[i], WHITE, -1);
          b.text(small, x + 130, yy, this.vocabFr(words[i]), GREY, -1);
          shown++;
        }
        if (idx === 0) b.text(small, x + 12, ly + 4, 'Čítaj nápisy (A).', GREY, -1);
      } else {
        const g = this.goal();
        if (g >= 0) {
          const l = A.course.scripts[g][0];
          b.text(big, x + 12, ly + 4, l.sk, WHITE, BLACK);
          if (this.noteFr_on_) b.text(small, x + 12, ly + 30, l.fr, YELLOW, -1);
          else b.text(small, x + 12, ly + 30, 'SELECT : FR', DIM, -1);
        }
        b.text(small, x + 12, y + h - 40, `Tajné repliky : ${this.bonusFound()} / ${this.bonusTotal()}`, GREY, -1);
        b.text(small, x + 12, y + h - 24, `Slovník : ${this.wordsFound()} / ${words.length}`, GREY, -1);
      }
      b.text(small, x + w - 72, y + h - 16, 'B : späť', DIM, -1);
    }

    // ---------------------------------------------------------------- rendu
    renderBand(b) {
      switch (this.state_) {
        case S_TITLE: this.drawBackdrop(b); this.drawTitle(b); break;
        case S_CHAPTER: this.drawBackdrop(b); this.drawChapter(b); break;
        case S_WORLD: this.world_.render(b); break;
        case S_DIALOG: this.world_.render(b); this.dialog_.render(b); break;
        case S_NOTE: this.world_.render(b); this.drawNote(b); break;
        case S_MENU: this.world_.render(b); this.drawMenu(b); break;
        case S_END: this.drawEnd(b); break;
        case S_FADE: {
          this.world_.render(b);
          const t = this.stateTime_ <= FADE_HALF ? this.stateTime_ : 2 * FADE_HALF - this.stateTime_;
          this.drawFade(b, Math.trunc((t * (SCREEN_H >> 1)) / FADE_HALF));
          break;
        }
      }
      if (this.banner_ > 0 && (this.state_ === S_WORLD || this.state_ === S_DIALOG || this.state_ === S_FADE)) this.drawBanner(b);
      if (this.savedShow_ > 0 && this.state_ === S_WORLD && b.y0 < 20 && this.banner_ === 0) {
        // petit rappel en haut a droite : la partie vient d'etre enregistree
        const small = this.A.fonts.font_small;
        const w = C.textWidth(small, 'Uložené') + 12;
        b.fillRect(SCREEN_W - w - 4, 4, w, 15, DARKBLUE);
        b.text(small, SCREEN_W - w + 2, 5, 'Uložené', WHITE, -1);
      }
      if (this.volumeShow_ > 0) this.drawVolume(b);
    }

    // rideau : deux bandes noires qui se rejoignent au milieu de l'ecran
    drawFade(b, covered) {
      b.fillRect(0, 0, SCREEN_W, covered, BLACK);
      b.fillRect(0, SCREEN_H - covered, SCREEN_W, covered, BLACK);
    }

    // decor de l'ecran titre : ciel en degrade et paves
    drawBackdrop(b) {
      for (let y = b.y0; y < b.y0 + b.h; y++) {
        let c;
        if (y < 150) c = (y < 60) ? SKY_TOP : SKY_LOW;
        else c = ((((y - 150) / 8) | 0) % 2) ? STONE : STONE_DARK;
        b.fillRect(0, y, SCREEN_W, 1, c);
      }
      for (let y = 150; y < SCREEN_H; y += 8) {
        const shift = ((((y - 150) / 8) | 0) % 2) ? 8 : 0;
        for (let x = shift; x < SCREEN_W; x += 16) b.fillRect(x, y, 1, 8, STONE_DARK);
      }
    }

    drawTitle(b) {
      const A = this.A, small = A.fonts.font_small, big = A.fonts.font_big, title = A.fonts.font_title;
      const f = ((this.frame_ / 12) | 0) % IDLE_FRAMES;
      b.frame(A.spr_fox, IDLE_ROW * A.spr_fox.cols + f, 136 - FOX_ANCHOR, 132);
      b.frame(A.spr_rabbit, IDLE_ROW * A.spr_rabbit.cols + (f + 2) % IDLE_FRAMES, 168, 132, true);
      b.fillRect(0, 26, SCREEN_W, 58, 0x18E6);
      b.fillRect(0, 26, SCREEN_W, 2, ORANGE);
      b.fillRect(0, 82, SCREEN_W, 2, ORANGE);
      this.centerText(b, title, 30, 'ZAJKA QUEST', ORANGE, BLACK);
      this.centerText(b, small, 66, 'Košice  →  Bratislava', WHITE, BLACK);
      if (this.confirm_) {
        b.fillRect(40, 176, SCREEN_W - 80, 44, DARKBLUE);
        b.fillRect(40, 176, SCREEN_W - 80, 2, ORANGE);
        this.centerText(b, small, 182, 'Vymazať uloženú hru?', WHITE, BLACK);
        this.centerText(b, small, 200, 'A : áno     B : nie', GREY, -1);
      } else if (this.hasSave_) {
        const items = ['Pokračovať', 'Nová hra'];
        for (let i = 0; i < 2; i++) {
          const y = 182 + i * 20;
          const c = i === this.menu_ ? YELLOW : WHITE;
          const w = C.textWidth(big, items[i]);
          b.text(big, Math.trunc((SCREEN_W - w) / 2), y, items[i], c, BLACK);
          if (i === this.menu_) b.text(big, Math.trunc((SCREEN_W - w) / 2) - 16, y, '›', YELLOW, BLACK);
        }
      } else if ((((this.frame_ / 30) | 0) % 2) === 0) {
        this.centerText(b, big, 194, 'Stlač START', WHITE, BLACK);
      }
      b.fillRect(0, SCREEN_H - 14, SCREEN_W, 14, 0x18E6);
      this.centerText(b, small, SCREEN_H - 13, 'Slovingo · Zajka Friends', GREY, -1);
    }

    drawChapter(b) {
      const small = this.A.fonts.font_small, title = this.A.fonts.font_title;
      b.fillRect(0, 70, SCREEN_W, 96, DARKBLUE);
      b.fillRect(0, 70, SCREEN_W, 2, ORANGE);
      b.fillRect(0, 164, SCREEN_W, 2, ORANGE);
      this.centerText(b, small, 78, `KAPITOLA ${this.chapter_ + 1}`, YELLOW, BLACK);
      this.centerText(b, title, 92, CHAPTER_PLACES[this.chapter_][0], WHITE, BLACK);
      this.centerText(b, small, 128, CHAPTER_PLACES[this.chapter_][1], GREY, BLACK);
      if (this.stateTime_ > 30 && (((this.frame_ / 30) | 0) % 2) === 0) this.centerText(b, small, 146, 'Stlač A', GREY, -1);
    }

    drawVolume(b) {
      const small = this.A.fonts.font_small;
      b.fillRect(100, 200, 120, 24, DARKBLUE);
      b.text(small, 108, 206, 'Hlasitosť', WHITE, -1);
      for (let i = 0; i < 4; i++) b.fillRect(170 + i * 10, 208, 7, 8, i < this.audio_.volume() ? YELLOW : 0x4208);
    }

    centerText(b, f, y, s, c, shadow) {
      b.text(f, Math.trunc((SCREEN_W - C.textWidth(f, s)) / 2), y, s, c, shadow);
    }

    drawBanner(b) {
      if (b.y0 > 34) return;
      const big = this.A.fonts.font_big;
      const ep = this.episode(this.bannerEp_);
      const s = `${ep.episode} · ${ep.title}`;
      const w = C.textWidth(big, s) + 24;
      const x = Math.trunc((SCREEN_W - w) / 2), y = 8;
      b.fillRect(x, y, w, 24, 0x1926);
      b.fillRect(x, y + 22, w, 2, 0xFB00);
      b.text(big, x + 12, y + 2, s, 0xFFFF, 0x0000);
    }

    drawEnd(b) {
      const small = this.A.fonts.font_small, title = this.A.fonts.font_title;
      const PLACES = ['KOŠICE', 'SUCHO'];
      const NEXT = ['Ďalšia kapitola : Sucho', 'Ďalšia kapitola : Slovenský raj'];
      b.fill(0x0841);
      b.fillRect(0, 50, SCREEN_W, 2, 0xFB00);
      b.fillRect(0, 188, SCREEN_W, 2, 0xFB00);
      this.centerText(b, small, 62, PLACES[this.chapter_], 0xFF2A, 0x0000);
      this.centerText(b, title, 78, `Koniec ${this.chapter_ + 1}. kapitoly`, 0xFFFF, 0x0000);
      this.centerText(b, small, 118, `Tajné repliky : ${this.bonusFound()} / ${this.bonusTotal()}`, 0xBDF7, -1);
      this.centerText(b, small, 134, `Slovník : ${this.wordsFound()} / ${this.A.signWords.length}`, 0xBDF7, -1);
      this.centerText(b, small, 154, NEXT[this.chapter_], 0xBDF7, -1);
      if (this.stateTime_ > 120 && (((this.frame_ / 30) | 0) % 2) === 0) this.centerText(b, small, 170, 'Stlač A', 0xFFFF, -1);
    }

    // ---------------------------------------------------------------- histoire : outils communs (story.cpp)
    episode(i) { const e = this.A.course.episodes; return e[i < e.length ? i : e.length - 1]; }

    // bonusEpisode >= 0 : ses choix peuvent gagner les repliques secretes de cet episode
    say(lines, bonusEpisode, after, event) {
      this.after_ = after;
      this.afterEvent_ = event || 0;
      if (lines.length <= 0) { this.storyDialogDone(); return; }
      const name = this.A.course.defaultName;
      if (bonusEpisode >= 0 && bonusEpisode < SAVE_EPISODES) {
        this.dialog_.start(lines, name, this.episode(bonusEpisode).bonus, this.bonus_, bonusEpisode);
      } else {
        this.dialog_.start(lines, name);
      }
      this.setState(S_DIALOG);
    }

    sayScript(script, after, event) { this.say(this.A.course.scripts[script], -1, after || AFTER_NOTHING, event || 0); }

    // repliques [from, to) du dialogue de l'episode ep (index global) ; banner : affiche son titre
    playEpisode(ep, from, to, after, banner, event) {
      if (banner) { this.bannerEp_ = ep; this.banner_ = BANNER_TIME; }
      this.say(this.episode(ep).dialog.slice(from, to), ep, after, event || 0);
    }

    sayLast(ep, who) {
      const e = this.episode(ep);
      for (let i = e.dialog.length - 1; i >= 0; i--) {
        if (e.dialog[i].s === who) { this.say([e.dialog[i]], -1, AFTER_NOTHING); return; }
      }
      this.audio_.play(Q.SFX_BUMP);
    }

    // passant ou voyageur : ses « repliques en plus » (repere « npc ... say »)
    passerby(w) {
      const mk = w.mark, eps = this.A.course.episodes;
      if (mk && mk.sayCount && mk.sayEp >= 1 && mk.sayEp <= eps.length) {
        const ep = eps[mk.sayEp - 1];
        const from = mk.sayFrom;
        let n = mk.sayCount;
        if (from + n > ep.extra.length) n = ep.extra.length - from;
        if (n > 0) { this.say(ep.extra.slice(from, from + n), -1, AFTER_NOTHING); return; }
      }
      this.audio_.play(Q.SFX_BUMP);
    }

    give(item, count, show) {
      if (count === undefined) count = 1;
      if (show === undefined) show = true;
      if (item < 0 || item >= SAVE_ITEMS) return;
      const n = this.items_[item] + count;
      this.items_[item] = n > 255 ? 255 : n < 0 ? 0 : n;
      if (count > 0 && show) {
        this.audio_.play(Q.SFX_LIFE);
        this.note('Do batohu :', this.A.items[item].sk, this.A.items[item].fr, item);
      }
    }

    storySign(m) {
      if (m.word < 0) return;
      const w = this.A.signWords[m.word];
      const fresh = !(this.words_[m.word >> 3] & (1 << (m.word & 7)));
      this.words_[m.word >> 3] |= 1 << (m.word & 7);
      this.audio_.play(fresh ? Q.SFX_LIFE : Q.SFX_PAUSE);
      this.note(fresh ? 'Nové slovo !' : 'Nápis', w, this.vocabFr(w));
    }

    storyPickup(m) {
      this.picked_[m.pickup >> 3] |= 1 << (m.pickup & 7);
      this.give(m.item);
    }

    storyEnteredMap() { if (this.chapter_ === 0) this.ch1EnteredMap(); else this.ch2EnteredMap(); }
    storyTalk(npc) {
      const w = this.world_.npc(npc);
      if (this.chapter_ === 0) this.ch1Talk(w); else this.ch2Talk(w);
    }
    storyInspect(tx, ty) { if (this.chapter_ === 0) this.ch1Inspect(tx, ty); else this.ch2Inspect(tx, ty); }
    storyZone(m) { if (this.chapter_ === 0) this.ch1Zone(m); else this.ch2Zone(m); }
    storyWarp(m) { return this.chapter_ === 0 ? this.ch1Warp(m) : this.ch2Warp(m); }
    storyRefresh() { if (this.chapter_ === 0) this.ch1Refresh(); else this.ch2Refresh(); }

    storyDialogDone() {
      const a = this.after_;
      this.after_ = AFTER_NOTHING;
      if (a === AFTER_EPISODE) {
        this.stage_++;
        if (this.chapter_ === 0) this.ch1EpisodeDone(); else this.ch2EpisodeDone();
      } else if (a === AFTER_EVENT) {
        if (this.chapter_ === 0) this.ch1Event(this.afterEvent_); else this.ch2Event(this.afterEvent_);
      } else if (a === AFTER_SLEEP) {
        this.endChapter();
      }
      if (this.state_ === S_WORLD) this.storyRefresh();
    }

    endChapter() {
      this.audio_.play(Q.SFX_WIN);
      this.setState(S_END);
    }

    // passage impose par l'histoire : le meme rideau qu'une porte. tx < 0 : au depart de la carte
    goTo(map, tx, ty, dir) {
      if (tx < 0) {
        const t = this.A.maps[map];
        tx = (t.w / 2) | 0;
        ty = (t.h / 2) | 0;
        for (let i = 0; i < t.nMarks; i++) {
          if (t.marks[i].type === Q.M_START) { tx = t.marks[i].x; ty = t.marks[i].y; dir = t.marks[i].dir; }
        }
      }
      this.autoWarp_ = { type: Q.M_WARP, target: map, tx, ty, dir, x: 0, y: 0, tag: 0 };
      this.warp_ = this.autoWarp_;
      this.audio_.play(Q.SFX_CHECK);
      this.setState(S_FADE);
    }

    // ---------------------------------------------------------------- chapitre 1 : Kosice (chapter1.cpp)
    ep1(i) { return this.A.course.episodes[EP1_FIRST + i]; }
    leadingNarrator(e) {
      let n = 0;
      while (n < e.dialog.length && e.dialog[n].s === this.K.SPK_NARRATOR) n++;
      return n;
    }
    trailingNarrator(e) {
      let n = 0;
      while (n < e.dialog.length && e.dialog[e.dialog.length - 1 - n].s === this.K.SPK_NARRATOR) n++;
      return n;
    }

    ch1Goal() {
      const K = this.K, m = this.world_.mapIndex();
      if (this.flag(F1_SLEPT)) return K.SC_CIEL_HOTOVO;
      if (m === K.MAP_KOSICE_TETA) return this.stage_ > EP1_TETA ? K.SC_CIEL_JABLON : K.SC_CIEL_ANDREA;
      if (!this.flag(F1_TICKET)) return K.SC_CIEL_LISTOK;
      if (m === K.MAP_KOSICE_HLAVNA) return K.SC_CIEL_NASTUP;
      if (this.stage_ >= EP1_CONTROL && !this.flag(F1_CONTROL_OK)) return K.SC_CIEL_REVIZOR;
      return K.SC_CIEL_ANDREA;
    }

    ch1Refresh() {
      const K = this.K, m = this.world_.mapIndex();
      if (m === K.MAP_KOSICE_HLAVNA) this.world_.setNpcHidden(K.TAG_VODIC, this.flag(F1_DRIVER));
      if (m === K.MAP_KOSICE_TRAM) this.world_.setNpcHidden(K.TAG_REVIZOR, !this.flag(F1_CONTROL));
    }

    ch1EnteredMap() {
      const K = this.K, m = this.world_.mapIndex();
      if (m === K.MAP_KOSICE_TRAM) {
        if (this.stage_ >= EP1_TRAM_END) {   // episode 5 fini (partie reprise dans le tram) : chez la tante
          this.goTo(K.MAP_KOSICE_TETA, -1, -1, DIR_UP);
          return;
        }
        if (!this.flag(F1_TRAM_INTRO)) {
          this.setFlag(F1_TRAM_INTRO);
          this.playEpisode(EP1_FIRST, 0, this.leadingNarrator(this.ep1(0)), AFTER_NOTHING, true);
        }
      } else if (m === K.MAP_KOSICE_TETA && !this.flag(F1_TETA_INTRO)) {
        this.setFlag(F1_TETA_INTRO);
        this.playEpisode(EP1_FIRST + EP1_TETA, 0, this.leadingNarrator(this.ep1(EP1_TETA)), AFTER_NOTHING, true);
      }
    }

    ch1Talk(w) {
      const K = this.K, m = this.world_.mapIndex();
      if (w.who === K.SPK_ANDREA) {
        if (m === K.MAP_KOSICE_TRAM && this.stage_ < EP1_TRAM_END) {
          if (this.stage_ >= EP1_CONTROL && !this.flag(F1_CONTROL_OK)) { this.sayScript(K.SC_ANDREA_POZOR); return; }
          const e = this.ep1(this.stage_);
          // episode 1 : le debut du Narrateur a deja ete joue en entrant
          const from = (this.stage_ === 0 && this.flag(F1_TRAM_INTRO)) ? this.leadingNarrator(e) : 0;
          this.playEpisode(EP1_FIRST + this.stage_, from, e.dialog.length, AFTER_EPISODE, this.stage_ > 0);
        } else if (m === K.MAP_KOSICE_TETA && this.stage_ === EP1_TETA) {
          const e = this.ep1(EP1_TETA);
          this.playEpisode(EP1_FIRST + EP1_TETA, this.leadingNarrator(e), e.dialog.length - this.trailingNarrator(e), AFTER_EPISODE, false);
        } else {
          this.sayLast(EP1_FIRST + (this.stage_ > 0 ? this.stage_ - 1 : 0), K.SPK_ANDREA);
        }
        return;
      }
      const tag = w.mark ? w.mark.tag : 0;
      if (tag === K.TAG_PREDAVAC) {
        if (!this.flag(F1_TICKET)) this.sayScript(K.SC_TRAFIKA, AFTER_EVENT, EV1_TICKET);
        else this.sayScript(K.SC_TRAFIKA_POTOM);
      } else if (tag === K.TAG_VODIC) {
        if (this.itemCount(K.ITEM_LISTOK)) this.sayScript(K.SC_VODIC_S_LISTKOM, AFTER_EVENT, EV1_DRIVER);
        else this.sayScript(K.SC_VODIC_BEZ_LISTKA);
      } else if (tag === K.TAG_REVIZOR) {
        if (this.flag(F1_CONTROL_OK)) this.sayScript(K.SC_REVIZOR_POTOM);
        else if (this.flag(F1_STAMPED)) this.sayScript(K.SC_REVIZOR_OK, AFTER_EVENT, EV1_CONTROL_OK);
        else this.sayScript(K.SC_REVIZOR_NEOZNACENY);
      } else {
        this.passerby(w);
      }
    }

    ch1Inspect(tx, ty) {
      const K = this.K, m = this.world_.mapIndex(), t = this.world_.tileAt(tx, ty);
      if (m === K.MAP_KOSICE_TRAM && t === K.TL_COMPOSTEUR) {
        if (this.flag(F1_STAMPED)) this.sayScript(K.SC_OZNACOVAC_ZNOVA);
        else if (this.itemCount(K.ITEM_LISTOK)) this.sayScript(K.SC_OZNACOVAC, AFTER_EVENT, EV1_STAMP);
        return;
      }
      if (m === K.MAP_KOSICE_TETA && t === K.TL_POMMIER && this.stage_ > EP1_TETA) {
        // la nuit sous le pommier : la derniere replique du Narrateur, puis la fin du chapitre
        const e = this.ep1(EP1_TETA);
        const n = this.trailingNarrator(e);
        this.setFlag(F1_SLEPT);
        this.say(e.dialog.slice(e.dialog.length - n), -1, AFTER_SLEEP);
      }
    }

    ch1Zone() {}
    ch1Warp() { return true; }

    ch1Event(event) {
      const K = this.K;
      switch (event) {
        case EV1_TICKET: this.setFlag(F1_TICKET); this.give(K.ITEM_LISTOK); break;
        case EV1_DRIVER: this.setFlag(F1_DRIVER); break;   // il s'ecarte (ch1Refresh le cache)
        case EV1_STAMP: this.setFlag(F1_STAMPED); break;
        case EV1_CONTROL_BOARD: this.setFlag(F1_CONTROL); break;
        case EV1_CONTROL_OK: this.setFlag(F1_CONTROL_OK); break;
      }
    }

    ch1EpisodeDone() {
      const K = this.K;
      if (this.world_.mapIndex() !== K.MAP_KOSICE_TRAM) return;
      if (this.stage_ === EP1_CONTROL && !this.flag(F1_CONTROL)) {
        // le tram s'arrete, un revizor monte
        this.sayScript(K.SC_REVIZOR_NASTUPUJE, AFTER_EVENT, EV1_CONTROL_BOARD);
      } else if (this.stage_ === EP1_TRAM_END) {
        // « To je moja zastavka. Pod! » : on descend avec Andrea, le soir, chez sa tante
        this.goTo(K.MAP_KOSICE_TETA, -1, -1, DIR_UP);
      }
    }

    // ---------------------------------------------------------------- chapitre 2 : Sucho (chapter2.cpp)
    ep2(i) { return this.A.course.episodes[EP2_FIRST + i]; }

    ch2Goal() {
      const K = this.K;
      switch (this.stage_) {
        case 0: case 1: case 2: return K.SC_CIEL_PARK;
        case 3: return this.flag(F2_GIVEN) ? K.SC_CIEL_ANDREA : K.SC_CIEL_KVAPKY;
        case 4: return K.SC_CIEL_FONTANA;
        case 5:
          if (this.world_.mapIndex() !== K.MAP_SUCHO_STANICA) return K.SC_CIEL_STANICA;
          return this.flag(F2_TICKETS) ? K.SC_CIEL_VLAK : K.SC_CIEL_OKIENKO;
        default: return K.SC_CIEL_HOTOVO;
      }
    }

    ch2Refresh() {
      // a partir de l'episode 2, Andrea suit le renard (celle posee sur la carte du jardin disparait)
      if (this.stage_ >= 1 && !this.world_.hasFollower()) this.world_.setFollower(this.K.SPK_ANDREA);
    }

    ch2EnteredMap() {
      const K = this.K, m = this.world_.mapIndex();
      if (m === K.MAP_SUCHO_ZAHRADA && this.stage_ === 0) {
        this.playEpisode(EP2_FIRST, 0, this.ep2(0).dialog.length, AFTER_EPISODE, true);
      } else if (m === K.MAP_SUCHO_STANICA && this.stage_ === 5 && !this.flag(F2_STATION_INTRO)) {
        this.setFlag(F2_STATION_INTRO);
        this.playEpisode(EP2_FIRST + 5, 0, S6_NOTE, AFTER_EVENT, true, EV2_STATION);
      } else if (m === K.MAP_SUCHO_STANICA && this.stage_ === 5 && !this.flag(F2_NOTE)) {
        this.sayScript(K.SC_ANDREA_TETA_LISTOK, AFTER_EVENT, EV2_NOTE);   // partie reprise avant le mot
      }
    }

    ch2Warp(m) {
      const K = this.K;
      if (this.world_.mapIndex() !== K.MAP_SUCHO_CESTA) return true;
      if (m.target === K.MAP_SUCHO_HLAVNA && this.stage_ < 4) { this.sayScript(K.SC_ANDREA_NAJPRV_PARK); return false; }
      if (m.target === K.MAP_SUCHO_STANICA && this.stage_ < 5) {
        this.sayScript(this.stage_ < 4 ? K.SC_ANDREA_NAJPRV_PARK : K.SC_ANDREA_FONTANA);
        return false;
      }
      return true;
    }

    ch2Zone(m) {
      const K = this.K, st = this.stage_;
      if (m.tag === K.TAG_SUCHO2 && st === 1) this.playEpisode(EP2_FIRST + 1, 0, this.ep2(1).dialog.length, AFTER_EPISODE, true);
      else if (m.tag === K.TAG_SUCHO3 && st === 2) this.playEpisode(EP2_FIRST + 2, 0, this.ep2(2).dialog.length, AFTER_EPISODE, true);
      else if (m.tag === K.TAG_SUCHO5 && st === 4) this.playEpisode(EP2_FIRST + 4, 0, this.ep2(4).dialog.length, AFTER_EPISODE, true);
      else if (m.tag === K.TAG_VLAK && st === 5) {
        if (this.flag(F2_TICKETS)) this.sayScript(K.SC_SPRIEVODCA, AFTER_EVENT, EV2_BOARD);
        else this.sayScript(K.SC_ANDREA_NAJPRV_LISTKY);
      }
    }

    ch2Talk(w) {
      const K = this.K, st = this.stage_;
      const tag = w.mark ? w.mark.tag : 0;
      if (w.who === K.SPK_ANDREA) {
        if (st === 3 && this.flag(F2_GIVEN)) this.playEpisode(EP2_FIRST + 3, 0, this.ep2(3).dialog.length, AFTER_EPISODE, true);
        else if (st === 3) this.sayScript(K.SC_ANDREA_KVAPKY);
        else if (st < 3) this.sayScript(K.SC_ANDREA_NAJPRV_PARK);
        else if (st === 4) this.sayScript(K.SC_ANDREA_FONTANA);
        else if (st === 5 && !this.flag(F2_TICKETS) && this.world_.mapIndex() === K.MAP_SUCHO_STANICA) this.sayScript(K.SC_ANDREA_NAJPRV_LISTKY);
        else this.sayLast(EP2_FIRST + (st > 0 ? st - 1 : 0), K.SPK_ANDREA);
        return;
      }
      if (w.who === K.SPK_MOUSE || w.who === K.SPK_CHICK) {
        if (this.flag(F2_GIVEN)) this.sayScript(K.SC_ZVIERATA_POTOM);
        else if (st >= 3 && this.itemCount(K.ITEM_KVAPKA) >= DROPS_NEEDED) this.sayScript(K.SC_ZVIERATA_DAKUJEME, AFTER_EVENT, EV2_DROPS);
        else if (this.itemCount(K.ITEM_KVAPKA) > 0) this.sayScript(K.SC_ZVIERATA_MALO);
        else this.sayScript(K.SC_ZVIERATA_SMAD);
        return;
      }
      if (tag === K.TAG_POKLADNA) {
        if (st === 5 && !this.flag(F2_TICKETS)) this.playEpisode(EP2_FIRST + 5, S6_DESK, S6_TRAIN, AFTER_EVENT, false, EV2_TICKETS);
        else this.sayLast(EP2_FIRST + 5, K.SPK_HUMAN);
        return;
      }
      if (tag === K.TAG_ZAHRADNIK) {
        if (st >= 5) this.sayLast(EP2_FIRST + 4, K.SPK_HUMAN);   // « Vela stastia. »
        else this.audio_.play(Q.SFX_BUMP);
        return;
      }
      this.passerby(w);
    }

    ch2Inspect() {}

    ch2Event(event) {
      const K = this.K;
      switch (event) {
        case EV2_STATION:   // « Pockaj. Musim napisat tete listok. » : on l'aide
          this.sayScript(K.SC_ANDREA_TETA_LISTOK, AFTER_EVENT, EV2_NOTE);
          break;
        case EV2_NOTE:
          this.setFlag(F2_NOTE);
          this.say([this.ep2(5).dialog[S6_NOTE]], -1, AFTER_NOTHING);   // « Andrea pise: ... »
          break;
        case EV2_DROPS:
          this.setFlag(F2_GIVEN);
          this.give(K.ITEM_KVAPKA, -DROPS_NEEDED);
          break;
        case EV2_TICKETS:
          this.setFlag(F2_TICKETS);
          this.give(K.ITEM_LISTKY);
          break;
        case EV2_BOARD:   // « Vlak ide na zapad. »
          this.say([this.ep2(5).dialog[S6_TRAIN]], -1, AFTER_SLEEP);
          break;
      }
    }

    ch2EpisodeDone() {
      const K = this.K;
      if (this.stage_ === 1) {
        // « Tu mas vodu. Len trochu. » ; ensuite Andrea suit le renard
        this.give(K.ITEM_VODA);
        this.world_.setFollower(K.SPK_ANDREA);
      } else if (this.stage_ === 4) {
        // « Mam batoh a mapu. » (Andrea part avec, le renard les porte)
        this.give(K.ITEM_MAPA, 1, false);
        this.give(K.ITEM_BATOH);
      }
    }
  }

  Object.assign(Q, {
    Game, SaveData, BrowserByteStorage, MemoryByteStorage,
    S_TITLE, S_CHAPTER, S_WORLD, S_FADE, S_DIALOG, S_END, S_NOTE, S_MENU, NUM_CHAPTERS,
  });
})(globalThis);
