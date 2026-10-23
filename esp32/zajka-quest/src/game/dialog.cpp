#include "dialog.h"

#include <string.h>

#include "gen/assets.h"

namespace {
const uint16_t WHITE = 0xFFFF, BLACK = 0x0000, GREY = 0xBDF7, DIM = 0x6B4D;
const uint16_t YELLOW = 0xFF2A, ORANGE = 0xFB00, DARKBLUE = 0x1926, RED = 0xF9A6, TEAL = 0x2D6F;

constexpr int BOX_X = 4, BOX_Y = 166, BOX_W = 312, BOX_H = 70;
constexpr int TEXT_X = 12, TEXT_Y = 172, TEXT_W = BOX_W - 44, ROW_H = 13, PAGE_ROWS = 4;  // marge : etiquette SK/FR

bool isCont(char c) { return ((uint8_t)c & 0xC0) == 0x80; }  // octet de suite UTF-8

// largeur d'un morceau de texte [s, s + n)
int spanWidth(const Font& f, const char* s, int n) {
  char tmp[160];
  if (n > (int)sizeof tmp - 1) n = sizeof tmp - 1;
  memcpy(tmp, s, n);
  tmp[n] = 0;
  return textWidth(f, tmp);
}

void drawSpan(Band& b, const Font& f, int x, int y, const char* s, int n, uint16_t c) {
  char tmp[160];
  if (n > (int)sizeof tmp - 1) n = sizeof tmp - 1;
  memcpy(tmp, s, n);
  tmp[n] = 0;
  b.text(f, x, y, tmp, c, -1);
}

// copie src dans dst en remplacant NAME_MARK par le nom du joueur
void resolve(char* dst, int cap, const char* src, const char* name) {
  int k = 0;
  for (const char* p = src; *p && k < cap - 1; p++) {
    if (*p == NAME_MARK) {
      for (const char* q = name; *q && k < cap - 1; q++) dst[k++] = *q;
    } else {
      dst[k++] = *p;
    }
  }
  dst[k] = 0;
}
}  // namespace

// ---------------------------------------------------------------- deroulement
void Dialog::start(const Line* lines, int n, const char* playerName, const Line* bonus, int nBonus,
                   int* bonusUsed) {
  lines_ = lines;
  n_ = n;
  index_ = -1;
  name_ = playerName;
  bonus_ = bonus;
  nBonus_ = nBonus;
  bonusUsed_ = bonusUsed;
  bonusQueued_ = 0;
  earned_ = mistakes_ = 0;
  mode_ = M_TEXT;
  next();
}

void Dialog::next() {
  if (index_ + 1 >= n_) {
    // fin des repliques : les repliques secretes gagnees en chemin
    if (bonusQueued_ > 0 && *bonusUsed_ < nBonus_) {
      bonusQueued_--;
      beginLine(&bonus_[(*bonusUsed_)++]);
      return;
    }
    index_ = n_;
    mode_ = M_DONE;
    cur_ = nullptr;
    return;
  }
  ++index_;
  cur_ = &lines_[index_];
  if (cur_->choice) beginChoice();
  else beginLine(cur_);
}

void Dialog::beginLine(const Line* l) {
  cur_ = l;
  blink_ = 0;
  mode_ = M_TEXT;
  view_ = V_SK;
  layout();
}

void Dialog::beginChoice() {
  const Choice& c = *cur_->choice;
  mode_ = M_CHOICE;
  view_ = V_SK;
  blink_ = 0;
  lookedUp_ = false;
  firstTry_ = true;
  placed_ = 0;
  cursor_ = 0;
  int n = c.n < MAX_OPT ? c.n : MAX_OPT;
  for (int i = 0; i < n; i++) {
    order_[i] = i;
    gone_[i] = false;
  }
  // melange (Fisher-Yates, generateur fixe : le simulateur rejoue toujours pareil) ;
  // on evite de laisser la bonne reponse en premier
  for (int tries = 0; tries < 4; tries++) {
    for (int i = n - 1; i > 0; i--) {
      seed_ = seed_ * 1103515245u + 12345u;
      int j = (seed_ >> 16) % (i + 1);
      uint8_t t = order_[i];
      order_[i] = order_[j];
      order_[j] = t;
    }
    if (order_[0] != 0) break;
  }
}

int Dialog::correctOption() const {
  if (mode_ != M_CHOICE) return -1;
  int want = cur_->choice->type == CH_ORDER ? placed_ : 0;
  for (int i = 0; i < cur_->choice->n && i < MAX_OPT; i++)
    if (order_[i] == want) return i;
  return -1;
}

void Dialog::pick(Audio* audio) {
  const Choice& c = *cur_->choice;
  int item = order_[cursor_];
  if (gone_[cursor_]) return;
  bool right = c.type == CH_ORDER ? item == placed_ : item == 0;
  if (!right) {
    mistakes_++;
    firstTry_ = false;
    shake_ = 12;
    if (c.type == CH_PICK) gone_[cursor_] = true;  // on l'ecarte, il reste les autres
    audio->play(SFX_BUMP);
    return;
  }
  audio->play(SFX_COIN);
  gone_[cursor_] = true;
  if (c.type == CH_ORDER && ++placed_ < c.n) {
    // curseur sur le prochain morceau encore disponible
    for (int k = 0; k < c.n; k++)
      if (!gone_[(cursor_ + k) % c.n]) {
        cursor_ = (cursor_ + k) % c.n;
        break;
      }
    return;
  }
  // reussi : le renard dit sa replique ; du premier coup et sans traduction, une replique secrete
  if (firstTry_ && !lookedUp_ && bonus_ && bonusUsed_ && *bonusUsed_ + bonusQueued_ < nBonus_) {
    bonusQueued_++;
    earned_++;
    audio->play(SFX_LIFE);
  }
  beginLine(cur_);
}

const char* Dialog::viewText() const {
  if (view_ == V_FR) return cur_->fr;
  if (view_ == V_HELP && cur_->help) return cur_->help;
  return cur_->sk;
}

void Dialog::cycleView() {
  if (view_ == V_SK) view_ = V_FR;
  else if (view_ == V_FR && cur_->help) view_ = V_HELP;
  else view_ = V_SK;
  if (mode_ == M_CHOICE) {
    lookedUp_ = lookedUp_ || view_ != V_SK;
    return;
  }
  layout();
  // on montre tout de suite le texte en entier
  shown_ = rows_ ? rowEnd_[(page_ + 1) * PAGE_ROWS - 1 < rows_ ? (page_ + 1) * PAGE_ROWS - 1 : rows_ - 1] : 0;
}

// Coupe text_ en lignes de TEXT_W pixels au plus, aux espaces.
void Dialog::layout() {
  resolve(text_, sizeof text_, viewText(), name_);
  rows_ = 0;
  page_ = 0;
  shown_ = 0;
  int len = strlen(text_);
  int start = 0;
  while (start < len && rows_ < MAX_ROWS) {
    while (start < len && text_[start] == ' ') start++;
    if (start >= len) break;
    int end = start, lastSpace = -1;
    while (end < len) {
      int next = end + 1;
      while (next < len && isCont(text_[next])) next++;
      if (spanWidth(font_small, text_ + start, next - start) > TEXT_W) break;
      if (text_[end] == ' ') lastSpace = end;
      end = next;
    }
    if (end < len && lastSpace > start) end = lastSpace;
    rowStart_[rows_] = start;
    rowEnd_[rows_] = end;
    rows_++;
    start = end;
  }
}

void Dialog::step(const Input& in, Audio* audio) {
  if (mode_ == M_DONE) return;
  blink_++;
  if (shake_ > 0) shake_--;
  if (in.hit(BTN_SELECT)) {
    cycleView();
    audio->play(SFX_PAUSE);
  }
  if (mode_ == M_CHOICE) {
    const Choice& c = *cur_->choice;
    int n = c.n < MAX_OPT ? c.n : MAX_OPT;
    // options a l'ecran, dans l'ordre d'affichage (un morceau deja place disparait)
    int vis[MAX_OPT], nv = 0;
    for (int i = 0; i < n; i++)
      if (!(c.type == CH_ORDER && gone_[i])) vis[nv++] = i;
    int cols = nv > 3 ? 2 : 1;
    int move = 0;
    if (in.hit(BTN_DOWN)) move = cols;
    if (in.hit(BTN_UP)) move = -cols;
    if (in.hit(BTN_RIGHT)) move = 1;
    if (in.hit(BTN_LEFT)) move = -1;
    if (move && nv > 0) {
      int k = 0;
      while (k < nv && vis[k] != cursor_) k++;
      if (k == nv) k = 0;
      // on avance dans ce sens jusqu'a une option encore libre
      for (int step = 0; step < nv; step++) {
        k = ((k + move) % nv + nv) % nv;
        if (!gone_[vis[k]]) break;
      }
      cursor_ = vis[k];
    }
    if (in.hit(BTN_A)) pick(audio);
    return;
  }
  // texte qui s'ecrit : une lettre par image, trois si A est tenu
  int pageEnd = rows_ ? rowEnd_[(page_ + 1) * PAGE_ROWS - 1 < rows_ ? (page_ + 1) * PAGE_ROWS - 1 : rows_ - 1] : 0;
  int pageStart = rows_ ? rowStart_[page_ * PAGE_ROWS] : 0;
  if (shown_ < pageStart) shown_ = pageStart;
  if (shown_ < pageEnd) {
    if (in.hit(BTN_A)) {
      shown_ = pageEnd;
      return;
    }
    int speed = in.down(BTN_A) ? 3 : 1;
    for (int k = 0; k < speed && shown_ < pageEnd; k++) {
      shown_++;
      while (shown_ < pageEnd && isCont(text_[shown_])) shown_++;
    }
    return;
  }
  if (in.hit(BTN_A)) {
    if ((page_ + 1) * PAGE_ROWS < rows_) {
      page_++;
      shown_ = rowStart_[page_ * PAGE_ROWS];
    } else {
      next();
    }
  }
}

// ---------------------------------------------------------------- rendu
void Dialog::drawBox(Band& b, const char* name) {
  int sx = shake_ ? ((shake_ / 2) % 2 ? 2 : -2) : 0;
  if (name && *name) {
    int w = textWidth(font_small, name) + 14;
    b.fillRect(BOX_X + 6 + sx, BOX_Y - 15, w, 16, ORANGE);
    b.text(font_small, BOX_X + 13 + sx, BOX_Y - 14, name, WHITE, BLACK);
  }
  b.fillRect(BOX_X + sx, BOX_Y, BOX_W, BOX_H, DARKBLUE);
  b.fillRect(BOX_X + sx, BOX_Y, BOX_W, 2, ORANGE);
  b.fillRect(BOX_X + sx, BOX_Y + BOX_H - 2, BOX_W, 2, ORANGE);
  // langue affichee, en haut a droite
  const char* tag = view_ == V_FR ? "FR" : view_ == V_HELP ? "?" : "SK";
  uint16_t tagc = view_ == V_SK ? TEAL : 0x8410;
  int tw = textWidth(font_small, tag) + 8;
  b.fillRect(BOX_X + BOX_W - tw - 4 + sx, BOX_Y + 4, tw, 14, tagc);
  b.text(font_small, BOX_X + BOX_W - tw + sx, BOX_Y + 4, tag, WHITE, -1);
}

void Dialog::render(Band& b) {
  if (mode_ == M_DONE || !cur_) return;
  if (b.y0 + b.h <= BOX_Y - 16) return;  // la boite n'occupe que le bas de l'ecran
  char nm[48];
  const char* who = cur_->speaker == SPK_FOX ? name_ : SPEAKER_NAMES[cur_->speaker];
  resolve(nm, sizeof nm, who, name_);
  drawBox(b, nm);
  int sx = shake_ ? ((shake_ / 2) % 2 ? 2 : -2) : 0;

  if (mode_ == M_CHOICE) {
    const Choice& c = *cur_->choice;
    int n = c.n < MAX_OPT ? c.n : MAX_OPT;
    int y = TEXT_Y;
    if (view_ != V_SK) {
      // la traduction (ou l'aide) de ce que le renard veut dire, en guise d'indice
      char t[200];
      resolve(t, sizeof t, view_ == V_FR || !cur_->help ? cur_->fr : cur_->help, name_);
      b.text(font_small, TEXT_X + sx, y, t, YELLOW, -1);
      y += ROW_H;
    }
    if (c.type == CH_ORDER) {
      // la phrase en cours de construction
      char built[200];
      int k = 0;
      for (int i = 0; i < placed_ && k < 180; i++) {
        resolve(built + k, sizeof built - k, c.items[i], name_);
        k = strlen(built);
        built[k++] = ' ';
      }
      built[k] = 0;
      if (view_ == V_SK) {
        int x = b.text(font_small, TEXT_X + sx, y, built, WHITE, -1);
        b.text(font_small, x, y, "…", GREY, -1);
        y += ROW_H;
      }
    }
    int nv = 0;
    for (int i = 0; i < n; i++)
      if (!(c.type == CH_ORDER && gone_[i])) nv++;
    int cols = nv > 3 ? 2 : 1;
    int colW = TEXT_W / cols;
    int slot = 0;
    for (int i = 0; i < n; i++) {
      if (c.type == CH_ORDER && gone_[i]) continue;  // morceau deja place : il disparait
      int cx = TEXT_X + 10 + (slot % cols) * colW + sx, cy = y + (slot / cols) * ROW_H;
      slot++;
      char t[160];
      resolve(t, sizeof t, c.items[order_[i]], name_);
      uint16_t col = gone_[i] ? DIM : (i == cursor_ ? YELLOW : WHITE);
      if (i == cursor_ && (blink_ / 15) % 4 != 3) b.text(font_small, cx - 10, cy, "›", YELLOW, -1);
      b.text(font_small, cx, cy, t, col, -1);
      if (c.type == CH_PICK && gone_[i]) b.fillRect(cx, cy + 6, textWidth(font_small, t), 1, RED);
    }
    return;
  }

  int first = page_ * PAGE_ROWS;
  for (int r = first; r < rows_ && r < first + PAGE_ROWS; r++) {
    int s = rowStart_[r], e = rowEnd_[r];
    if (shown_ <= s) break;
    if (shown_ < e) e = shown_;
    drawSpan(b, font_small, TEXT_X + sx, TEXT_Y + (r - first) * ROW_H, text_ + s, e - s,
             view_ == V_SK ? WHITE : (view_ == V_FR ? YELLOW : GREY));
  }
  int last = rows_ ? rowEnd_[(first + PAGE_ROWS - 1) < rows_ ? first + PAGE_ROWS - 1 : rows_ - 1] : 0;
  if (shown_ >= last && (blink_ / 20) % 2 == 0)
    b.text(font_small, BOX_X + BOX_W - 16, BOX_Y + BOX_H - 17, "›", YELLOW, -1);
  // rappel discret de la traduction, tant qu'il reste de la place sous le texte
  if (view_ == V_SK && cur_->fr && *cur_->fr && blink_ < 240 && rows_ - first < PAGE_ROWS)
    b.text(font_small, BOX_X + BOX_W - 96, BOX_Y + BOX_H - 17, "SELECT : FR", DIM, -1);
}
