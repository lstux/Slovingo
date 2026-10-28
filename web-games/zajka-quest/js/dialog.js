// Boite de dialogue : port de esp32/zajka-quest/src/game/dialog.{h,cpp}.
// Les textes sont traites par points de code (le firmware les traite par octets UTF-8 : memes coupures).
(function (root) {
  'use strict';
  const C = root.ZajkaCommon;
  const { BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_A, BTN_SELECT } = C;
  const Q = root.ZajkaQuest = root.ZajkaQuest || {};

  const WHITE = 0xFFFF, BLACK = 0x0000, GREY = 0xBDF7, DIM = 0x6B4D;
  const YELLOW = 0xFF2A, ORANGE = 0xFB00, DARKBLUE = 0x1926, RED = 0xF9A6, TEAL = 0x2D6F;
  const BOX_X = 4, BOX_Y = 166, BOX_W = 312, BOX_H = 70;
  const TEXT_X = 12, TEXT_Y = 172, TEXT_W = BOX_W - 44, ROW_H = 13, PAGE_ROWS = 4;
  const NAME_MARK = '\x01';
  const MAX_OPT = 8, MAX_ROWS = 24;
  const CH_ORDER = 0, CH_PICK = 1;
  const M_TEXT = 0, M_CHOICE = 1, M_DONE = 2;
  const V_SK = 0, V_FR = 1, V_HELP = 2;

  // remplace NAME_MARK par le nom du joueur
  function resolve(src, name) { return src.split(NAME_MARK).join(name); }

  class Dialog {
    constructor(A) {
      this.A = A;
      this.lines_ = null; this.n_ = 0; this.index_ = -1;
      this.bonus_ = null; this.nBonus_ = 0; this.bonusArr_ = null; this.bonusIdx_ = 0;
      this.name_ = ''; this.cur_ = null; this.bonusQueued_ = 0;
      this.mode_ = M_DONE; this.view_ = V_SK; this.lookedUp_ = false;
      this.chars_ = []; this.cw_ = []; this.rowStart_ = []; this.rowEnd_ = [];
      this.rows_ = 0; this.page_ = 0; this.shown_ = 0; this.blink_ = 0;
      this.order_ = new Array(MAX_OPT).fill(0); this.gone_ = new Array(MAX_OPT).fill(false);
      this.placed_ = 0; this.cursor_ = 0; this.shake_ = 0; this.firstTry_ = true;
      this.earned_ = 0; this.mistakes_ = 0; this.seed_ = 12345;
    }

    // lines : tableau de repliques ; bonus : repliques secretes ; bonusArr[bonusIdx] : combien deja donnees
    start(lines, playerName, bonus, bonusArr, bonusIdx) {
      this.lines_ = lines; this.n_ = lines.length; this.index_ = -1; this.name_ = playerName;
      this.bonus_ = bonus || null; this.nBonus_ = bonus ? bonus.length : 0;
      this.bonusArr_ = bonusArr || null; this.bonusIdx_ = bonusIdx || 0;
      this.bonusQueued_ = 0; this.earned_ = this.mistakes_ = 0;
      this.mode_ = M_TEXT;
      this.next();
    }

    active() { return this.mode_ !== M_DONE; }
    choosing() { return this.mode_ === M_CHOICE; }
    cursor() { return this.cursor_; }
    bonusEarned() { return this.earned_; }
    mistakes() { return this.mistakes_; }
    current() { return this.cur_; }

    bonusUsed() { return this.bonusArr_[this.bonusIdx_]; }

    next() {
      if (this.index_ + 1 >= this.n_) {
        // fin des repliques : les repliques secretes gagnees en chemin
        if (this.bonusQueued_ > 0 && this.bonusUsed() < this.nBonus_) {
          this.bonusQueued_--;
          this.beginLine(this.bonus_[this.bonusArr_[this.bonusIdx_]++]);
          return;
        }
        this.index_ = this.n_;
        this.mode_ = M_DONE;
        this.cur_ = null;
        return;
      }
      ++this.index_;
      this.cur_ = this.lines_[this.index_];
      if (this.cur_.choice) this.beginChoice(); else this.beginLine(this.cur_);
    }

    beginLine(l) {
      this.cur_ = l;
      this.blink_ = 0;
      this.mode_ = M_TEXT;
      this.view_ = V_SK;
      this.layout();
    }

    beginChoice() {
      const c = this.cur_.choice;
      this.mode_ = M_CHOICE;
      this.view_ = V_SK;
      this.blink_ = 0;
      this.lookedUp_ = false;
      this.firstTry_ = true;
      this.placed_ = 0;
      this.cursor_ = 0;
      const n = c.items.length < MAX_OPT ? c.items.length : MAX_OPT;
      for (let i = 0; i < n; i++) { this.order_[i] = i; this.gone_[i] = false; }
      // melange (Fisher-Yates, generateur fixe) ; on evite de laisser la bonne reponse en premier
      for (let tries = 0; tries < 4; tries++) {
        for (let i = n - 1; i > 0; i--) {
          this.seed_ = (Math.imul(this.seed_, 1103515245) + 12345) >>> 0;
          const j = (this.seed_ >>> 16) % (i + 1);
          const t = this.order_[i]; this.order_[i] = this.order_[j]; this.order_[j] = t;
        }
        if (this.order_[0] !== 0) break;
      }
    }

    correctOption() {
      if (this.mode_ !== M_CHOICE) return -1;
      const want = this.cur_.choice.type === CH_ORDER ? this.placed_ : 0;
      const n = this.cur_.choice.items.length;
      for (let i = 0; i < n && i < MAX_OPT; i++) if (this.order_[i] === want) return i;
      return -1;
    }

    pick(audio) {
      const c = this.cur_.choice, n = c.items.length;
      const item = this.order_[this.cursor_];
      if (this.gone_[this.cursor_]) return;
      const right = c.type === CH_ORDER ? item === this.placed_ : item === 0;
      if (!right) {
        this.mistakes_++;
        this.firstTry_ = false;
        this.shake_ = 12;
        if (c.type === CH_PICK) this.gone_[this.cursor_] = true;   // on l'ecarte, il reste les autres
        audio.play(Q.SFX_BUMP);
        return;
      }
      audio.play(Q.SFX_COIN);
      this.gone_[this.cursor_] = true;
      if (c.type === CH_ORDER && ++this.placed_ < n) {
        // curseur sur le prochain morceau encore disponible
        for (let k = 0; k < n; k++) {
          if (!this.gone_[(this.cursor_ + k) % n]) { this.cursor_ = (this.cursor_ + k) % n; break; }
        }
        return;
      }
      // reussi : du premier coup et sans traduction, une replique secrete
      if (this.firstTry_ && !this.lookedUp_ && this.bonus_ && this.bonusArr_ &&
          this.bonusUsed() + this.bonusQueued_ < this.nBonus_) {
        this.bonusQueued_++;
        this.earned_++;
        audio.play(Q.SFX_LIFE);
      }
      this.beginLine(this.cur_);
    }

    viewText() {
      if (this.view_ === V_FR) return this.cur_.fr;
      if (this.view_ === V_HELP && this.cur_.help) return this.cur_.help;
      return this.cur_.sk;
    }

    pageEndOf(page) {
      const i = (page + 1) * PAGE_ROWS - 1;
      return this.rows_ ? this.rowEnd_[i < this.rows_ ? i : this.rows_ - 1] : 0;
    }

    cycleView() {
      if (this.view_ === V_SK) this.view_ = V_FR;
      else if (this.view_ === V_FR && this.cur_.help) this.view_ = V_HELP;
      else this.view_ = V_SK;
      if (this.mode_ === M_CHOICE) {
        this.lookedUp_ = this.lookedUp_ || this.view_ !== V_SK;
        return;
      }
      this.layout();
      // on montre tout de suite le texte en entier
      this.shown_ = this.pageEndOf(this.page_);
    }

    // coupe le texte en lignes de TEXT_W pixels au plus, aux espaces
    layout() {
      const font = this.A.fonts.font_small;
      const chars = Array.from(resolve(this.viewText(), this.name_));
      this.chars_ = chars;
      this.cw_ = chars.map((ch) => C.textWidth(font, ch));
      this.rows_ = 0; this.page_ = 0; this.shown_ = 0;
      const len = chars.length, cw = this.cw_;
      let start = 0;
      while (start < len && this.rows_ < MAX_ROWS) {
        while (start < len && chars[start] === ' ') start++;
        if (start >= len) break;
        let end = start, lastSpace = -1, w = 0;
        while (end < len) {
          if (w + cw[end] > TEXT_W) break;
          w += cw[end];
          if (chars[end] === ' ') lastSpace = end;
          end++;
        }
        if (end < len && lastSpace > start) end = lastSpace;
        this.rowStart_[this.rows_] = start;
        this.rowEnd_[this.rows_] = end;
        this.rows_++;
        start = end;
      }
    }

    step(inp, audio) {
      if (this.mode_ === M_DONE) return;
      this.blink_++;
      if (this.shake_ > 0) this.shake_--;
      if (inp.hit(BTN_SELECT)) {
        this.cycleView();
        audio.play(Q.SFX_PAUSE);
      }
      if (this.mode_ === M_CHOICE) {
        const c = this.cur_.choice;
        const n = c.items.length < MAX_OPT ? c.items.length : MAX_OPT;
        // options a l'ecran, dans l'ordre d'affichage (un morceau deja place disparait)
        const vis = [];
        for (let i = 0; i < n; i++) if (!(c.type === CH_ORDER && this.gone_[i])) vis.push(i);
        const nv = vis.length;
        const cols = nv > 3 ? 2 : 1;
        let move = 0;
        if (inp.hit(BTN_DOWN)) move = cols;
        if (inp.hit(BTN_UP)) move = -cols;
        if (inp.hit(BTN_RIGHT)) move = 1;
        if (inp.hit(BTN_LEFT)) move = -1;
        if (move && nv > 0) {
          let k = 0;
          while (k < nv && vis[k] !== this.cursor_) k++;
          if (k === nv) k = 0;
          // on avance dans ce sens jusqu'a une option encore libre
          for (let step = 0; step < nv; step++) {
            k = (((k + move) % nv) + nv) % nv;
            if (!this.gone_[vis[k]]) break;
          }
          this.cursor_ = vis[k];
        }
        if (inp.hit(BTN_A)) this.pick(audio);
        return;
      }
      // texte qui s'ecrit : une lettre par image, trois si A est tenu
      const pageEnd = this.pageEndOf(this.page_);
      const pageStart = this.rows_ ? this.rowStart_[this.page_ * PAGE_ROWS] : 0;
      if (this.shown_ < pageStart) this.shown_ = pageStart;
      if (this.shown_ < pageEnd) {
        if (inp.hit(BTN_A)) { this.shown_ = pageEnd; return; }
        const speed = inp.down(BTN_A) ? 3 : 1;
        for (let k = 0; k < speed && this.shown_ < pageEnd; k++) this.shown_++;
        return;
      }
      if (inp.hit(BTN_A)) {
        if ((this.page_ + 1) * PAGE_ROWS < this.rows_) {
          this.page_++;
          this.shown_ = this.rowStart_[this.page_ * PAGE_ROWS];
        } else {
          this.next();
        }
      }
    }

    // ---------------------------------------------------------------- rendu
    drawBox(b, name) {
      const A = this.A, small = A.fonts.font_small;
      const sx = this.shake_ ? (((this.shake_ / 2) | 0) % 2 ? 2 : -2) : 0;
      // portrait (art/portraits/), pose sur le bord de la boite, a gauche ; le nom a cote
      const face = this.cur_ ? A.portraits[this.cur_.s] : null;
      let nameX = BOX_X + 6;
      if (face) {
        b.fillRect(BOX_X + 4, BOX_Y - 52, 52, 52, ORANGE);
        b.fillRect(BOX_X + 6, BOX_Y - 50, 48, 48, DARKBLUE);
        b.blit(face, 0, 0, face.w, face.h, BOX_X + 6, BOX_Y - 50);
        nameX = BOX_X + 60;
      }
      if (name) {
        const w = C.textWidth(small, name) + 14;
        b.fillRect(nameX + sx, BOX_Y - 15, w, 16, ORANGE);
        b.text(small, nameX + 7 + sx, BOX_Y - 14, name, WHITE, BLACK);
      }
      b.fillRect(BOX_X + sx, BOX_Y, BOX_W, BOX_H, DARKBLUE);
      b.fillRect(BOX_X + sx, BOX_Y, BOX_W, 2, ORANGE);
      b.fillRect(BOX_X + sx, BOX_Y + BOX_H - 2, BOX_W, 2, ORANGE);
      // langue affichee, en haut a droite
      const tag = this.view_ === V_FR ? 'FR' : this.view_ === V_HELP ? '?' : 'SK';
      const tagc = this.view_ === V_SK ? TEAL : 0x8410;
      const tw = C.textWidth(small, tag) + 8;
      b.fillRect(BOX_X + BOX_W - tw - 4 + sx, BOX_Y + 4, tw, 14, tagc);
      b.text(small, BOX_X + BOX_W - tw + sx, BOX_Y + 4, tag, WHITE, -1);
    }

    render(b) {
      if (this.mode_ === M_DONE || !this.cur_) return;
      if (b.y0 + b.h <= BOX_Y - 52) return;   // la boite (et son portrait) n'occupe que le bas de l'ecran
      const A = this.A, small = A.fonts.font_small, cur = this.cur_;
      const who = cur.s === A.K.SPK_FOX ? this.name_ : A.course.speakerNames[cur.s];
      this.drawBox(b, resolve(who, this.name_));
      const sx = this.shake_ ? (((this.shake_ / 2) | 0) % 2 ? 2 : -2) : 0;

      if (this.mode_ === M_CHOICE) {
        const c = cur.choice;
        const n = c.items.length < MAX_OPT ? c.items.length : MAX_OPT;
        let y = TEXT_Y;
        if (this.view_ !== V_SK) {
          // la traduction (ou l'aide) de ce que le renard veut dire, en guise d'indice
          const t = resolve(this.view_ === V_FR || !cur.help ? cur.fr : cur.help, this.name_);
          b.text(small, TEXT_X + sx, y, t, YELLOW, -1);
          y += ROW_H;
        }
        if (c.type === CH_ORDER) {
          // la phrase en cours de construction
          let built = '';
          for (let i = 0; i < this.placed_; i++) built += resolve(c.items[i], this.name_) + ' ';
          if (this.view_ === V_SK) {
            const x = b.text(small, TEXT_X + sx, y, built, WHITE, -1);
            b.text(small, x, y, '…', GREY, -1);
            y += ROW_H;
          }
        }
        let nv = 0;
        for (let i = 0; i < n; i++) if (!(c.type === CH_ORDER && this.gone_[i])) nv++;
        const cols = nv > 3 ? 2 : 1;
        const colW = (TEXT_W / cols) | 0;
        let slot = 0;
        for (let i = 0; i < n; i++) {
          if (c.type === CH_ORDER && this.gone_[i]) continue;   // morceau deja place : il disparait
          const cx = TEXT_X + 10 + (slot % cols) * colW + sx, cy = y + ((slot / cols) | 0) * ROW_H;
          slot++;
          const t = resolve(c.items[this.order_[i]], this.name_);
          const col = this.gone_[i] ? DIM : (i === this.cursor_ ? YELLOW : WHITE);
          if (i === this.cursor_ && (((this.blink_ / 15) | 0) % 4) !== 3) b.text(small, cx - 10, cy, '›', YELLOW, -1);
          b.text(small, cx, cy, t, col, -1);
          if (c.type === CH_PICK && this.gone_[i]) b.fillRect(cx, cy + 6, C.textWidth(small, t), 1, RED);
        }
        return;
      }

      const first = this.page_ * PAGE_ROWS;
      for (let r = first; r < this.rows_ && r < first + PAGE_ROWS; r++) {
        const s = this.rowStart_[r];
        let e = this.rowEnd_[r];
        if (this.shown_ <= s) break;
        if (this.shown_ < e) e = this.shown_;
        b.text(small, TEXT_X + sx, TEXT_Y + (r - first) * ROW_H, this.chars_.slice(s, e).join(''),
          this.view_ === V_SK ? WHITE : (this.view_ === V_FR ? YELLOW : GREY), -1);
      }
      const last = this.pageEndOf(this.page_);
      if (this.shown_ >= last && (((this.blink_ / 20) | 0) % 2) === 0)
        b.text(small, BOX_X + BOX_W - 16, BOX_Y + BOX_H - 17, '›', YELLOW, -1);
      // rappel discret de la traduction, tant qu'il reste de la place sous le texte
      if (this.view_ === V_SK && cur.fr && this.blink_ < 240 && this.rows_ - first < PAGE_ROWS)
        b.text(small, BOX_X + BOX_W - 96, BOX_Y + BOX_H - 17, 'SELECT : FR', DIM, -1);
    }
  }

  Q.Dialog = Dialog;
})(globalThis);
