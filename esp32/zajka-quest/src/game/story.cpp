// Outils communs de l'histoire : conversations, episodes du cours, objets, panneaux, passages
// imposes, fin de chapitre. Ce qui se passe ou est ecrit chapitre par chapitre :
// chapter1.cpp (Kosice), chapter2.cpp (Sucho), chapter3.cpp (Slovensky raj).
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "gen/assets.h"
#include "gen/scripts.h"

namespace {
constexpr int BANNER_TIME = 150;

const Episode& episode(int i) { return *COURSE_EPISODES[i < COURSE_NUM_EPISODES ? i : COURSE_NUM_EPISODES - 1]; }

const char* vocabFr(const char* sk) {
  for (int i = 0; i < COURSE_NUM_VOCAB; i++)
    if (strcmp(COURSE_VOCAB[i].sk, sk) == 0) return COURSE_VOCAB[i].fr;
  return "";
}
}  // namespace

int Game::bonusFound() const {
  int n = 0;
  for (int b : bonus_) n += b;
  return n;
}

// Episodes du cours : 6 par chapitre (COURSE_EPISODES[6 * chapitre + n]).
int Game::bonusTotal(int chapter) const {
  int n = 0;
  for (int i = 0; i < COURSE_NUM_EPISODES && i < SAVE_EPISODES; i++)
    if (chapter < 0 || i / 6 == chapter) n += COURSE_EPISODES[i]->nBonus;
  return n;
}

int Game::goal() const { return chapter_ == 0 ? ch1Goal() : chapter_ == 1 ? ch2Goal() : ch3Goal(); }

// ---------------------------------------------------------------- conversations
// bonusEpisode >= 0 : ses choix peuvent gagner les repliques secretes de cet episode.
// after / event : ce qui se passe quand la conversation est finie (storyDialogDone).
void Game::say(const Line* lines, int n, int bonusEpisode, After after, int event) {
  after_ = after;
  afterEvent_ = event;
  if (n <= 0) {
    storyDialogDone();
    return;
  }
  if (bonusEpisode >= 0 && bonusEpisode < SAVE_EPISODES) {
    const Episode& ep = episode(bonusEpisode);
    dialog_.start(lines, n, COURSE_DEFAULT_NAME, ep.bonus, ep.nBonus, &bonus_[bonusEpisode]);
  } else {
    dialog_.start(lines, n, COURSE_DEFAULT_NAME);
  }
  setState(S_DIALOG);
}

void Game::sayScript(int script, After after, int event) {
  say(SCRIPTS[script].lines, SCRIPTS[script].n, -1, after, event);
}

// Repliques [from, to) du dialogue de l'episode ep (index global) ; banner : affiche son titre.
void Game::playEpisode(int ep, int from, int to, After after, bool banner, int event) {
  if (banner) {
    bannerEp_ = ep;
    banner_ = BANNER_TIME;
  }
  say(episode(ep).dialog + from, to - from, ep, after, event);
}

void Game::sayLast(int ep, Speaker who) {
  const Episode& e = episode(ep);
  for (int i = e.nDialog - 1; i >= 0; i--)
    if (e.dialog[i].speaker == who) {
      say(&e.dialog[i], 1, -1, AFTER_NOTHING);
      return;
    }
  audio_->play(SFX_BUMP);
}

// Passant ou voyageur : ses « repliques en plus » (repere « npc ... say »).
void Game::passerby(const Walker& w) {
  const MapMark* mk = w.mark;
  if (mk && mk->sayCount && mk->sayEp >= 1 && mk->sayEp <= COURSE_NUM_EPISODES) {
    const Episode& ep = *COURSE_EPISODES[mk->sayEp - 1];
    int from = mk->sayFrom, n = mk->sayCount;
    if (from + n > ep.nExtra) n = ep.nExtra - from;
    if (n > 0) {
      say(ep.extra + from, n, -1, AFTER_NOTHING);
      return;
    }
  }
  audio_->play(SFX_BUMP);
}

// ---------------------------------------------------------------- objets, panneaux
void Game::give(int item, int count, bool show) {
  if (item < 0 || item >= SAVE_ITEMS) return;
  int n = items_[item] + count;
  items_[item] = (uint8_t)(n > 255 ? 255 : n < 0 ? 0 : n);
  if (count > 0 && show) {
    audio_->play(SFX_LIFE);
    note("Do batohu :", ITEMS[item].sk, ITEMS[item].fr, item);
  }
}

void Game::storySign(const MapMark& m) {
  if (m.word < 0) return;
  const char* w = SIGN_WORDS[m.word];
  bool fresh = !(words_[m.word >> 3] & (1 << (m.word & 7)));
  words_[m.word >> 3] |= (uint8_t)(1 << (m.word & 7));
  audio_->play(fresh ? SFX_LIFE : SFX_PAUSE);
  note(fresh ? "Nové slovo !" : "Nápis", w, vocabFr(w));
}

void Game::storyPickup(const MapMark& m) {
  picked_[m.pickup >> 3] |= (uint8_t)(1 << (m.pickup & 7));
  give(m.item);
}

// ---------------------------------------------------------------- aiguillage par chapitre
void Game::storyEnteredMap() {
  if (chapter_ == 0) ch1EnteredMap();
  else if (chapter_ == 1) ch2EnteredMap();
  else ch3EnteredMap();
}

void Game::storyTalk(int npc) {
  const Walker& w = world_.npc(npc);
  if (chapter_ == 0) ch1Talk(w);
  else if (chapter_ == 1) ch2Talk(w);
  else ch3Talk(w);
}

void Game::storyInspect(int tx, int ty) {
  if (chapter_ == 0) ch1Inspect(tx, ty);
  else if (chapter_ == 1) ch2Inspect(tx, ty);
  else ch3Inspect(tx, ty);
}

void Game::storyZone(const MapMark& m) {
  if (chapter_ == 0) ch1Zone(m);
  else if (chapter_ == 1) ch2Zone(m);
  else ch3Zone(m);
}

bool Game::storyWarp(const MapMark& m) { return chapter_ == 0 ? ch1Warp(m) : chapter_ == 1 ? ch2Warp(m) : ch3Warp(m); }

void Game::storyStep(int tx, int ty) {
  if (chapter_ == 2) ch3Step(tx, ty);
}

void Game::storyRefresh() {
  if (chapter_ == 0) ch1Refresh();
  else if (chapter_ == 1) ch2Refresh();
  else ch3Refresh();
}

void Game::storyDialogDone() {
  After a = after_;
  after_ = AFTER_NOTHING;
  if (a == AFTER_EPISODE) {
    stage_++;
    if (chapter_ == 0) ch1EpisodeDone();
    else if (chapter_ == 1) ch2EpisodeDone();
    else ch3EpisodeDone();
  } else if (a == AFTER_EVENT) {
    if (chapter_ == 0) ch1Event(afterEvent_);
    else if (chapter_ == 1) ch2Event(afterEvent_);
    else ch3Event(afterEvent_);
  } else if (a == AFTER_SLEEP) {
    endChapter();
  }
  if (state_ == S_WORLD) storyRefresh();
}

void Game::endChapter() {
  audio_->play(SFX_WIN);
  setState(S_END);
}

// Passage impose par l'histoire : le meme rideau qu'une porte. tx < 0 : au depart de la carte.
void Game::goTo(int map, int tx, int ty, Dir dir) {
  if (tx < 0) {
    const MapDef& t = *ALL_MAPS[map];
    tx = t.w / 2;
    ty = t.h / 2;
    for (int i = 0; i < t.nMarks; i++)
      if (t.marks[i].type == M_START) {
        tx = t.marks[i].x;
        ty = t.marks[i].y;
        dir = t.marks[i].dir;
      }
  }
  autoWarp_ = MapMark{};
  autoWarp_.type = M_WARP;
  autoWarp_.target = (int8_t)map;
  autoWarp_.tx = (uint8_t)tx;
  autoWarp_.ty = (uint8_t)ty;
  autoWarp_.dir = dir;
  warp_ = &autoWarp_;
  audio_->play(SFX_CHECK);
  setState(S_FADE);
}

// ---------------------------------------------------------------- rendu
void Game::drawBanner(Band& b) {
  if (b.y0 > 34) return;
  const Episode& ep = episode(bannerEp_);
  char s[96];
  snprintf(s, sizeof s, "%d · %s", ep.episodeNum, ep.title);
  int w = textWidth(font_big, s) + 24;
  int x = (SCREEN_W - w) / 2, y = 8;
  b.fillRect(x, y, w, 24, 0x1926);
  b.fillRect(x, y + 22, w, 2, 0xFB00);
  b.text(font_big, x + 12, y + 2, s, 0xFFFF, 0x0000);
}

void Game::drawEnd(Band& b) {
  static const char* const PLACES[] = {"KOŠICE", "SUCHO", "SLOVENSKÝ RAJ"};
  static const char* const NEXT[] = {"Ďalšia kapitola : Sucho", "Ďalšia kapitola : Slovenský raj", "Pokračovanie nabudúce : Tatry"};
  b.fill(0x0841);
  b.fillRect(0, 50, SCREEN_W, 2, 0xFB00);
  b.fillRect(0, 188, SCREEN_W, 2, 0xFB00);
  centerText(b, font_small, 62, PLACES[chapter_], 0xFF2A, 0x0000);
  char s[64];
  snprintf(s, sizeof s, "Koniec %d. kapitoly", chapter_ + 1);
  centerText(b, font_title, 78, s, 0xFFFF, 0x0000);
  snprintf(s, sizeof s, "Tajné repliky : %d / %d", bonusFound(), bonusTotal());
  centerText(b, font_small, 118, s, 0xBDF7, -1);
  snprintf(s, sizeof s, "Slovník : %d / %d", wordsFound(), NUM_SIGN_WORDS);
  centerText(b, font_small, 134, s, 0xBDF7, -1);
  centerText(b, font_small, 154, NEXT[chapter_], 0xBDF7, -1);
  if (stateTime_ > 120 && (frame_ / 30) % 2 == 0) centerText(b, font_small, 170, "Stlač A", 0xFFFF, -1);
}
