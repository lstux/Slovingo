#include "game.h"

#include <stdio.h>

#include "gen/assets.h"

namespace {
const uint16_t WHITE = 0xFFFF, BLACK = 0x0000;
const uint16_t YELLOW = 0xFF2A, ORANGE = 0xFB00, DARKBLUE = 0x1926, GREY = 0xBDF7;
const uint16_t SKY_TOP = 0x2A4E, SKY_LOW = 0x5C9B, STONE = 0x8C51, STONE_DARK = 0x6B4D;

// planches provisoires du renard et d'Andrea (32 x 48, reprises de Zajka Run) : ligne 0 = repos
constexpr int IDLE_ROW = 0, IDLE_FRAMES = 4;
}  // namespace

// ---------------------------------------------------------------- mise en route
void Game::begin(Audio* audio) {
  audio_ = audio;
  setState(S_TITLE);
  audio_->music(0);
}

void Game::setState(GameState s) {
  state_ = s;
  stateTime_ = 0;
}

// ---------------------------------------------------------------- boucle logique
void Game::step(const Input& in) {
  frame_++;
  stateTime_++;
  if (volumeShow_ > 0) volumeShow_--;
  if (in.hit(BTN_VOLUME)) {
    audio_->setVolume((audio_->volume() + 1) % 5);
    volumeShow_ = 90;
    audio_->play(SFX_COIN);
  }
  switch (state_) {
    case S_TITLE:
      if (in.hit(BTN_START) || in.hit(BTN_A)) {
        audio_->play(SFX_PAUSE);
        audio_->music(-1);
        setState(S_CHAPTER);
      }
      break;
    case S_CHAPTER:
      // plus tard : entree dans la carte de Kosice ; pour l'instant, retour au titre
      if (stateTime_ > 30 && (in.hit(BTN_START) || in.hit(BTN_B))) {
        audio_->play(SFX_PAUSE);
        begin(audio_);
      }
      break;
  }
}

// ---------------------------------------------------------------- rendu
void Game::renderBand(Band& b) {
  drawBackdrop(b);
  if (state_ == S_TITLE) drawTitle(b);
  else drawChapter(b);
  if (volumeShow_ > 0) drawVolume(b);
}

// Decor provisoire : ciel en degrade et paves, en attendant les tuiles de la rue Hlavna.
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
  b.frame(spr_fox, IDLE_ROW * spr_fox.cols + f, 120, 132);
  b.frame(spr_rabbit, IDLE_ROW * spr_rabbit.cols + (f + 2) % IDLE_FRAMES, 168, 132, true);
  b.fillRect(0, 26, SCREEN_W, 58, 0x18E6);
  b.fillRect(0, 26, SCREEN_W, 2, ORANGE);
  b.fillRect(0, 82, SCREEN_W, 2, ORANGE);
  centerText(b, font_title, 30, "ZAJKA QUEST", ORANGE, BLACK);
  centerText(b, font_small, 66, "Košice  →  Bratislava", WHITE, BLACK);
  if ((frame_ / 30) % 2 == 0) centerText(b, font_big, 194, "Stlač START", WHITE, BLACK);
  b.fillRect(0, SCREEN_H - 14, SCREEN_W, 14, 0x18E6);
  centerText(b, font_small, SCREEN_H - 13, "Slovingo · Zajka Friends", GREY, -1);
}

void Game::drawChapter(Band& b) {
  b.fillRect(0, 70, SCREEN_W, 96, DARKBLUE);
  b.fillRect(0, 70, SCREEN_W, 2, ORANGE);
  b.fillRect(0, 164, SCREEN_W, 2, ORANGE);
  centerText(b, font_small, 78, "KAPITOLA 1", YELLOW, BLACK);
  centerText(b, font_title, 92, "Košice", WHITE, BLACK);
  centerText(b, font_small, 128, "To si ty?", GREY, BLACK);
  if (stateTime_ > 30) centerText(b, font_small, 146, "Pokračovanie čoskoro…", GREY, -1);
}

void Game::drawVolume(Band& b) {
  b.fillRect(100, 200, 120, 24, DARKBLUE);
  b.text(font_small, 108, 206, "Hlasitosť", WHITE, -1);
  for (int i = 0; i < 4; i++) b.fillRect(170 + i * 10, 208, 7, 8, i < audio_->volume() ? YELLOW : 0x4208);
}

void Game::centerText(Band& b, const Font& f, int y, const char* s, uint16_t c, int shadow) {
  b.text(f, (SCREEN_W - textWidth(f, s)) / 2, y, s, c, shadow);
}
