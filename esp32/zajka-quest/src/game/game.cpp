#include "game.h"

#include <stdio.h>
#include <string.h>

#include "gen/assets.h"
#include "gen/scripts.h"

namespace {
const uint16_t WHITE = 0xFFFF, BLACK = 0x0000;
const uint16_t YELLOW = 0xFF2A, ORANGE = 0xFB00, DARKBLUE = 0x1926, GREY = 0xBDF7, DIM = 0x6B4D;
const uint16_t SKY_TOP = 0x2A4E, SKY_LOW = 0x5C9B, STONE = 0x8C51, STONE_DARK = 0x6B4D;

// le renard et Andrea de l'ecran titre : les planches de Zajka Run (ligne 0 = repos) ;
// le renard est dans des cases de 48 x 48, la tete a x = 27
constexpr int IDLE_ROW = 0, IDLE_FRAMES = 4, FOX_ANCHOR = 27;
constexpr int FADE_HALF = 14;  // images pour fermer le rideau, autant pour le rouvrir

struct ChapterInfo {
  const char* place;
  const char* subtitle;
  int startMap;
};
const ChapterInfo CHAPTERS[NUM_CHAPTERS] = {
    {"Košice", "To si ty?", MAP_KOSICE_HLAVNA},
    {"Sucho", "Je horúco", MAP_SUCHO_ZAHRADA},
    {"Slovenský raj", "Kam teraz?", MAP_RAJ_STANICA},
};

// FR d'un mot du Slovnik (vocabulaire du cours), ou ""
const char* vocabFr(const char* sk) {
  for (int i = 0; i < COURSE_NUM_VOCAB; i++)
    if (strcmp(COURSE_VOCAB[i].sk, sk) == 0) return COURSE_VOCAB[i].fr;
  return "";
}
}  // namespace

// ---------------------------------------------------------------- mise en route
void Game::begin(Audio* audio, Storage* storage) {
  audio_ = audio;
  storage_ = storage;
  chapter_ = 0;
  stage_ = 0;
  flags_ = 0;
  memset(items_, 0, sizeof items_);
  memset(picked_, 0, sizeof picked_);
  memset(words_, 0, sizeof words_);
  for (int& b : bonus_) b = 0;
  world_.setPicked(picked_);
  world_.setFollower(SPK_NONE);
  after_ = AFTER_NOTHING;
  banner_ = 0;
  lastTalk_ = SPK_NONE;
  hasSave_ = false;
  if (storage_ && storage_->read(&saved_, sizeof saved_) && saved_.valid() && saved_.map < NUM_MAPS &&
      saved_.chapter < NUM_CHAPTERS) {
    const MapDef& m = *ALL_MAPS[saved_.map];
    hasSave_ = saved_.tx < m.w && saved_.ty < m.h && saved_.dir >= 0 && saved_.dir <= DIR_RIGHT;
  }
  menu_ = 0;
  confirm_ = false;
  setState(S_TITLE);
  audio_->music(0);
}

// ---------------------------------------------------------------- sauvegarde
void Game::save() {
  if (!storage_) return;
  SaveData d;
  d.map = (uint8_t)world_.mapIndex();
  d.tx = (uint8_t)world_.player().tx;
  d.ty = (uint8_t)world_.player().ty;
  d.dir = world_.player().dir;
  d.chapter = (uint8_t)chapter_;
  d.stage = (uint8_t)stage_;
  d.flags = flags_;
  memcpy(d.items, items_, sizeof d.items);
  memcpy(d.picked, picked_, sizeof d.picked);
  memcpy(d.words, words_, sizeof d.words);
  for (int i = 0; i < SAVE_EPISODES; i++) d.bonus[i] = (uint8_t)bonus_[i];
  d.check = d.sum();
  if (storage_->write(&d, sizeof d)) {
    saved_ = d;
    hasSave_ = true;
    savedShow_ = 70;
  }
}

void Game::newGame() {
  if (storage_ && hasSave_) storage_->erase();
  hasSave_ = false;
  memset(items_, 0, sizeof items_);
  memset(picked_, 0, sizeof picked_);
  memset(words_, 0, sizeof words_);
  for (int& b : bonus_) b = 0;
  audio_->play(SFX_PAUSE);
  startChapter(0);
}

// Carte du chapitre, puis sa premiere carte (S_CHAPTER -> S_WORLD).
void Game::startChapter(int chapter) {
  chapter_ = chapter;
  stage_ = 0;
  flags_ = 0;
  world_.setFollower(SPK_NONE);
  audio_->music(-1);
  setState(S_CHAPTER);
}

bool Game::continueGame() {
  if (!hasSave_) return false;
  chapter_ = saved_.chapter;
  stage_ = saved_.stage;
  flags_ = saved_.flags;
  memcpy(items_, saved_.items, sizeof items_);
  memcpy(picked_, saved_.picked, sizeof picked_);
  memcpy(words_, saved_.words, sizeof words_);
  for (int i = 0; i < SAVE_EPISODES; i++) bonus_[i] = saved_.bonus[i];
  world_.setFollower(SPK_NONE);
  world_.load(saved_.map, saved_.tx, saved_.ty, (Dir)saved_.dir);
  audio_->play(SFX_PAUSE);
  audio_->music(world_.map().music);
  setState(S_WORLD);
  storyRefresh();
  storyEnteredMap();  // ex. : une partie arretee juste avant un passage impose le refait
  return true;
}

// Ecran titre : START ou A. Avec une partie enregistree : continuer ou recommencer (confirme).
void Game::stepTitle(const Input& in) {
  bool ok = in.hit(BTN_START) || in.hit(BTN_A);
  if (!hasSave_) {
    if (ok) newGame();
    return;
  }
  if (confirm_) {
    if (in.hit(BTN_A)) newGame();
    else if (in.hit(BTN_B) || in.hit(BTN_START)) {
      confirm_ = false;
      audio_->play(SFX_BUMP);
    }
    return;
  }
  if (in.hit(BTN_UP) || in.hit(BTN_DOWN)) {
    menu_ ^= 1;
    audio_->play(SFX_COIN);
  }
  if (ok) {
    if (menu_ == 0) continueGame();
    else {
      confirm_ = true;
      audio_->play(SFX_PAUSE);
    }
  }
}

void Game::setState(GameState s) {
  state_ = s;
  stateTime_ = 0;
}

int Game::wordsFound() const {
  int n = 0;
  for (int i = 0; i < NUM_SIGN_WORDS; i++)
    if (words_[i >> 3] & (1 << (i & 7))) n++;
  return n;
}

// ---------------------------------------------------------------- boucle logique
void Game::step(const Input& in) {
  frame_++;
  stateTime_++;
  if (volumeShow_ > 0) volumeShow_--;
  if (savedShow_ > 0) savedShow_--;
  if (banner_ > 0 && state_ != S_FADE) banner_--;
  if (in.hit(BTN_VOLUME)) {
    audio_->setVolume((audio_->volume() + 1) % 5);
    volumeShow_ = 90;
    audio_->play(SFX_COIN);
  }
  switch (state_) {
    case S_TITLE:
      stepTitle(in);
      break;
    case S_CHAPTER:
      if (stateTime_ > 30 && (in.hit(BTN_START) || in.hit(BTN_A))) {
        audio_->play(SFX_PAUSE);
        world_.loadStart(CHAPTERS[chapter_].startMap);
        audio_->music(world_.map().music);
        setState(S_WORLD);
        storyRefresh();
        save();
        storyEnteredMap();
      }
      break;
    case S_WORLD:
      if (in.hit(BTN_START)) {
        audio_->play(SFX_PAUSE);
        menuScroll_ = 0;
        setState(S_MENU);
        break;
      }
      switch (world_.step(in)) {
        case WE_WARP:
          if (!storyWarp(*world_.pendingWarp())) break;
          warp_ = world_.pendingWarp();
          audio_->play(SFX_CHECK);
          setState(S_FADE);
          break;
        case WE_BUMP:
          audio_->play(SFX_BUMP);
          break;
        case WE_TALK:
          lastTalk_ = world_.npc(world_.talkNpc()).who;
          talks_++;
          storyTalk(world_.talkNpc());
          break;
        case WE_INSPECT:
          storyInspect(world_.inspectX(), world_.inspectY());
          break;
        case WE_SIGN:
          storySign(*world_.event());
          break;
        case WE_PICKUP:
          storyPickup(*world_.event());
          break;
        case WE_ZONE:
          storyZone(*world_.event());
          break;
        default:
          break;
      }
      if (state_ == S_WORLD && world_.stepped()) storyStep(world_.player().tx, world_.player().ty);
      if (state_ == S_WORLD) idleHint();
      break;
    case S_FADE:
      if (stateTime_ == FADE_HALF && warp_) {
        int from = world_.mapIndex();
        world_.load(warp_->target, warp_->tx, warp_->ty, warp_->dir);
        storyRefresh();
        if (world_.map().music != ALL_MAPS[from]->music) audio_->music(world_.map().music);
        warp_ = nullptr;
      }
      if (stateTime_ >= 2 * FADE_HALF) {
        setState(S_WORLD);
        save();
        storyEnteredMap();
      }
      break;
    case S_DIALOG:
      dialog_.step(in, audio_);
      if (!dialog_.active()) {
        setState(S_WORLD);
        storyDialogDone();  // peut changer d'etat (message, passage impose, fin du chapitre)
        if (state_ == S_WORLD || state_ == S_FADE || state_ == S_END) save();
      }
      break;
    case S_NOTE:
      if (in.hit(BTN_SELECT)) {
        noteFr_on_ = !noteFr_on_;
        audio_->play(SFX_PAUSE);
      }
      if (stateTime_ > 20 && (in.hit(BTN_A) || in.hit(BTN_B))) {
        setState(S_WORLD);
        save();
      }
      break;
    case S_MENU:
      stepMenu(in);
      break;
    case S_END:
      if (stateTime_ > 120 && (in.hit(BTN_A) || in.hit(BTN_START))) {
        audio_->play(SFX_PAUSE);
        if (chapter_ + 1 < NUM_CHAPTERS) {
          // chapitre suivant : on garde le sac et le Slovnik
          startChapter(chapter_ + 1);
        } else {
          begin(audio_, storage_);
        }
      }
      break;
  }
}

// Le joueur tourne en rond : apres IDLE_HINT images sans que l'objectif change, on le lui rappelle
// (le meme texte que « Ciel » dans le menu START), puis de nouveau un peu plus tard.
void Game::idleHint() {
  constexpr int IDLE_HINT = 75 * 60, IDLE_AGAIN = 150 * 60;
  int g = goal();
  if (g != idleGoal_) {
    idleGoal_ = g;
    idle_ = 0;
    return;
  }
  if (++idle_ < IDLE_HINT) return;
  idle_ = IDLE_HINT - IDLE_AGAIN;
  const Line& l = SCRIPTS[g].lines[0];
  audio_->play(SFX_PAUSE);
  note("Cieľ", l.sk, l.fr);
}

// ---------------------------------------------------------------- petit message
void Game::note(const char* title, const char* sk, const char* fr, int icon) {
  noteTitle_ = title;
  noteSk_ = sk;
  noteFr_ = fr;
  noteIcon_ = icon;
  noteFr_on_ = false;
  setState(S_NOTE);
}

void Game::drawNote(Band& b) {
  const int x = 40, y = 84, w = SCREEN_W - 80, h = 64;
  if (b.y0 > y + h || b.y0 + b.h < y) return;
  b.fillRect(x, y, w, h, DARKBLUE);
  b.fillRect(x, y, w, 2, ORANGE);
  b.fillRect(x, y + h - 2, w, 2, ORANGE);
  centerText(b, font_small, y + 6, noteTitle_, YELLOW, BLACK);
  int tx = x + 16;
  if (noteIcon_ >= 0) {
    b.frame(item_icons, noteIcon_, tx, y + 26);
    tx += 22;
  }
  b.text(font_big, tx, y + 22, noteSk_, WHITE, BLACK);
  if (noteFr_on_ && noteFr_ && *noteFr_) b.text(font_small, tx, y + 44, noteFr_, YELLOW, -1);
  else if (noteFr_ && *noteFr_) b.text(font_small, x + w - 86, y + h - 15, "SELECT : FR", DIM, -1);
}

// ---------------------------------------------------------------- menu START
// Trois onglets : Batoh (objets), Slovnik (mots trouves), Ciel (objectif). Gauche / droite : onglet,
// haut / bas : defiler, SELECT : traduction de l'objectif, B ou START : fermer.
void Game::stepMenu(const Input& in) {
  if (in.hit(BTN_B) || in.hit(BTN_START)) {
    audio_->play(SFX_PAUSE);
    setState(S_WORLD);
    return;
  }
  if (in.hit(BTN_LEFT)) {
    menuTab_ = (menuTab_ + 2) % 3;
    menuScroll_ = 0;
    audio_->play(SFX_COIN);
  }
  if (in.hit(BTN_RIGHT)) {
    menuTab_ = (menuTab_ + 1) % 3;
    menuScroll_ = 0;
    audio_->play(SFX_COIN);
  }
  if (in.hit(BTN_DOWN)) menuScroll_++;
  if (in.hit(BTN_UP) && menuScroll_ > 0) menuScroll_--;
  if (in.hit(BTN_SELECT)) noteFr_on_ = !noteFr_on_;
}

void Game::drawMenu(Band& b) {
  const int x = 16, y = 16, w = SCREEN_W - 32, h = SCREEN_H - 32;
  b.fillRect(x, y, w, h, DARKBLUE);
  b.fillRect(x, y, w, 2, ORANGE);
  b.fillRect(x, y + h - 2, w, 2, ORANGE);
  const char* tabs[3] = {"Batoh", "Slovník", "Cieľ"};
  int tw = w / 3;
  for (int i = 0; i < 3; i++) {
    int cx = x + i * tw;
    if (i == menuTab_) b.fillRect(cx + 4, y + 6, tw - 8, 20, 0x2A4E);
    int tl = textWidth(font_big, tabs[i]);
    b.text(font_big, cx + (tw - tl) / 2, y + 7, tabs[i], i == menuTab_ ? YELLOW : GREY, BLACK);
  }
  b.fillRect(x + 8, y + 30, w - 16, 1, ORANGE);
  int ly = y + 38;
  const int rowH = 18, rows = 8;
  if (menuTab_ == 0) {
    int shown = 0, idx = 0;
    for (int i = 0; i < NUM_ITEMS && i < SAVE_ITEMS; i++) {
      if (!items_[i]) continue;
      if (idx++ < menuScroll_) continue;
      if (shown >= rows) break;
      int yy = ly + shown * rowH;
      b.frame(item_icons, i, x + 12, yy);
      char s[64];
      if (items_[i] > 1) snprintf(s, sizeof s, "%s  ×%d", ITEMS[i].sk, items_[i]);
      else snprintf(s, sizeof s, "%s", ITEMS[i].sk);
      b.text(font_small, x + 34, yy + 2, s, WHITE, -1);
      b.text(font_small, x + 160, yy + 2, ITEMS[i].fr, GREY, -1);
      shown++;
    }
    if (idx == 0) b.text(font_small, x + 12, ly, "Batoh je prázdny.", GREY, -1);
  } else if (menuTab_ == 1) {
    char s[48];
    snprintf(s, sizeof s, "%d / %d", wordsFound(), NUM_SIGN_WORDS);
    b.text(font_small, x + w - 12 - textWidth(font_small, s), y + 36, s, YELLOW, -1);
    int shown = 0, idx = 0;
    for (int i = 0; i < NUM_SIGN_WORDS; i++) {
      if (!(words_[i >> 3] & (1 << (i & 7)))) continue;
      if (idx++ < menuScroll_) continue;
      if (shown >= rows) break;
      int yy = ly + 4 + shown * rowH;
      b.text(font_small, x + 12, yy, SIGN_WORDS[i], WHITE, -1);
      b.text(font_small, x + 130, yy, vocabFr(SIGN_WORDS[i]), GREY, -1);
      shown++;
    }
    if (idx == 0) b.text(font_small, x + 12, ly + 4, "Čítaj nápisy (A).", GREY, -1);
  } else {
    int g = goal();
    if (g >= 0) {
      const Line& l = SCRIPTS[g].lines[0];
      b.text(font_big, x + 12, ly + 4, l.sk, WHITE, BLACK);
      if (noteFr_on_) b.text(font_small, x + 12, ly + 30, l.fr, YELLOW, -1);
      else b.text(font_small, x + 12, ly + 30, "SELECT : FR", DIM, -1);
    }
    char s[64];
    snprintf(s, sizeof s, "Tajné repliky : %d / %d", bonusFound(), bonusTotal());
    b.text(font_small, x + 12, y + h - 40, s, GREY, -1);
    snprintf(s, sizeof s, "Slovník : %d / %d", wordsFound(), NUM_SIGN_WORDS);
    b.text(font_small, x + 12, y + h - 24, s, GREY, -1);
  }
  b.text(font_small, x + w - 72, y + h - 16, "B : späť", DIM, -1);
}

// ---------------------------------------------------------------- rendu
void Game::renderBand(Band& b) {
  switch (state_) {
    case S_TITLE:
      drawBackdrop(b);
      drawTitle(b);
      break;
    case S_CHAPTER:
      drawBackdrop(b);
      drawChapter(b);
      break;
    case S_WORLD:
      world_.render(b);
      if (chapter_ == 1) ch2Hud(b);
      if (chapter_ == 2) ch3Hud(b);
      break;
    case S_DIALOG:
      world_.render(b);
      dialog_.render(b);
      break;
    case S_NOTE:
      world_.render(b);
      drawNote(b);
      break;
    case S_MENU:
      world_.render(b);
      drawMenu(b);
      break;
    case S_END:
      drawEnd(b);
      break;
    case S_FADE: {
      world_.render(b);
      int t = stateTime_ <= FADE_HALF ? stateTime_ : 2 * FADE_HALF - stateTime_;
      drawFade(b, t * (SCREEN_H / 2) / FADE_HALF);
      break;
    }
  }
  if (banner_ > 0 && (state_ == S_WORLD || state_ == S_DIALOG || state_ == S_FADE)) drawBanner(b);
  if (savedShow_ > 0 && state_ == S_WORLD && b.y0 < 20 && banner_ == 0) {
    // petit rappel en haut a droite : la partie vient d'etre enregistree
    int w = textWidth(font_small, "Uložené") + 12;
    b.fillRect(SCREEN_W - w - 4, 4, w, 15, DARKBLUE);
    b.text(font_small, SCREEN_W - w + 2, 5, "Uložené", WHITE, -1);
  }
  if (volumeShow_ > 0) drawVolume(b);
}

// Rideau : deux bandes noires qui se rejoignent au milieu de l'ecran.
void Game::drawFade(Band& b, int covered) {
  b.fillRect(0, 0, SCREEN_W, covered, BLACK);
  b.fillRect(0, SCREEN_H - covered, SCREEN_W, covered, BLACK);
}

// Decor de l'ecran titre (provisoire) : ciel en degrade et paves.
void Game::drawBackdrop(Band& b) {
  for (int y = b.y0; y < b.y0 + b.h; y++) {
    uint16_t c;
    if (y < 150) c = (y < 60) ? SKY_TOP : SKY_LOW;
    else c = (((y - 150) / 8) % 2) ? STONE : STONE_DARK;
    b.fillRect(0, y, SCREEN_W, 1, c);
  }
  // joints des paves
  for (int y = 150; y < SCREEN_H; y += 8) {
    int shift = ((y - 150) / 8) % 2 ? 8 : 0;
    for (int x = shift; x < SCREEN_W; x += 16) b.fillRect(x, y, 1, 8, STONE_DARK);
  }
}

void Game::drawTitle(Band& b) {
  int f = (frame_ / 12) % IDLE_FRAMES;
  b.frame(spr_fox, IDLE_ROW * spr_fox.cols + f, 136 - FOX_ANCHOR, 132);
  b.frame(spr_rabbit, IDLE_ROW * spr_rabbit.cols + (f + 2) % IDLE_FRAMES, 168, 132, true);
  b.fillRect(0, 26, SCREEN_W, 58, 0x18E6);
  b.fillRect(0, 26, SCREEN_W, 2, ORANGE);
  b.fillRect(0, 82, SCREEN_W, 2, ORANGE);
  centerText(b, font_title, 30, "ZAJKA QUEST", ORANGE, BLACK);
  centerText(b, font_small, 66, "Košice  →  Bratislava", WHITE, BLACK);
  if (confirm_) {
    b.fillRect(40, 176, SCREEN_W - 80, 44, DARKBLUE);
    b.fillRect(40, 176, SCREEN_W - 80, 2, ORANGE);
    centerText(b, font_small, 182, "Vymazať uloženú hru?", WHITE, BLACK);
    centerText(b, font_small, 200, "A : áno     B : nie", GREY, -1);
  } else if (hasSave_) {
    const char* items[2] = {"Pokračovať", "Nová hra"};
    for (int i = 0; i < 2; i++) {
      int y = 182 + i * 20;
      uint16_t c = i == menu_ ? YELLOW : WHITE;
      int w = textWidth(font_big, items[i]);
      b.text(font_big, (SCREEN_W - w) / 2, y, items[i], c, BLACK);
      if (i == menu_) b.text(font_big, (SCREEN_W - w) / 2 - 16, y, "›", YELLOW, BLACK);
    }
  } else if ((frame_ / 30) % 2 == 0) {
    centerText(b, font_big, 194, "Stlač START", WHITE, BLACK);
  }
  b.fillRect(0, SCREEN_H - 14, SCREEN_W, 14, 0x18E6);
  centerText(b, font_small, SCREEN_H - 13, "Slovingo · Zajka Friends", GREY, -1);
}

void Game::drawChapter(Band& b) {
  b.fillRect(0, 70, SCREEN_W, 96, DARKBLUE);
  b.fillRect(0, 70, SCREEN_W, 2, ORANGE);
  b.fillRect(0, 164, SCREEN_W, 2, ORANGE);
  char s[32];
  snprintf(s, sizeof s, "KAPITOLA %d", chapter_ + 1);
  centerText(b, font_small, 78, s, YELLOW, BLACK);
  centerText(b, font_title, 92, CHAPTERS[chapter_].place, WHITE, BLACK);
  centerText(b, font_small, 128, CHAPTERS[chapter_].subtitle, GREY, BLACK);
  if (stateTime_ > 30 && (frame_ / 30) % 2 == 0) centerText(b, font_small, 146, "Stlač A", GREY, -1);
}

void Game::drawVolume(Band& b) {
  b.fillRect(100, 200, 120, 24, DARKBLUE);
  b.text(font_small, 108, 206, "Hlasitosť", WHITE, -1);
  for (int i = 0; i < 4; i++) b.fillRect(170 + i * 10, 208, 7, 8, i < audio_->volume() ? YELLOW : 0x4208);
}

void Game::centerText(Band& b, const Font& f, int y, const char* s, uint16_t c, int shadow) {
  b.text(f, (SCREEN_W - textWidth(f, s)) / 2, y, s, c, shadow);
}
