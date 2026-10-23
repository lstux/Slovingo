#include "game.h"

#include <stdio.h>

#include "gen/assets.h"

namespace {
const uint16_t WHITE = 0xFFFF, BLACK = 0x0000;
const uint16_t YELLOW = 0xFF2A, ORANGE = 0xFB00, DARKBLUE = 0x1926, GREY = 0xBDF7;
const uint16_t SKY_TOP = 0x2A4E, SKY_LOW = 0x5C9B, STONE = 0x8C51, STONE_DARK = 0x6B4D;

// planches provisoires du renard et d'Andrea (32 x 48, reprises de Zajka Run) : ligne 0 = repos
constexpr int IDLE_ROW = 0, IDLE_FRAMES = 4;
constexpr int FADE_HALF = 14;  // images pour fermer le rideau, autant pour le rouvrir

// Episode 1 « To si ty? » : les premieres repliques du narrateur plantent le decor en entrant dans
// le tram, le reste se joue en parlant a Andrea.
const Episode& episode1() { return *COURSE_EPISODES[0]; }
int introLength(const Episode& ep) {
  int n = 0;
  while (n < ep.nDialog && ep.dialog[n].speaker == SPK_NARRATOR) n++;
  return n;
}
}  // namespace

// ---------------------------------------------------------------- mise en route
void Game::begin(Audio* audio, Storage* storage) {
  audio_ = audio;
  storage_ = storage;
  introDone_ = metAndrea_ = false;
  bonusUsed_ = 0;
  lastTalk_ = SPK_NONE;
  hasSave_ = false;
  if (storage_ && storage_->read(&saved_, sizeof saved_) && saved_.valid() && saved_.map < NUM_MAPS) {
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
  d.flags = (introDone_ ? SF_INTRO_DONE : 0) | (metAndrea_ ? SF_MET_ANDREA : 0);
  d.bonus = (uint8_t)bonusUsed_;
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
  introDone_ = metAndrea_ = false;
  bonusUsed_ = 0;
  audio_->play(SFX_PAUSE);
  audio_->music(-1);
  setState(S_CHAPTER);
}

bool Game::continueGame() {
  if (!hasSave_) return false;
  introDone_ = saved_.flags & SF_INTRO_DONE;
  metAndrea_ = saved_.flags & SF_MET_ANDREA;
  bonusUsed_ = saved_.bonus;
  world_.load(saved_.map, saved_.tx, saved_.ty, (Dir)saved_.dir);
  audio_->play(SFX_PAUSE);
  audio_->music(world_.map().music);
  setState(S_WORLD);
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

// ---------------------------------------------------------------- boucle logique
void Game::step(const Input& in) {
  frame_++;
  stateTime_++;
  if (volumeShow_ > 0) volumeShow_--;
  if (savedShow_ > 0) savedShow_--;
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
        world_.loadStart(MAP_KOSICE_HLAVNA);
        audio_->music(world_.map().music);
        setState(S_WORLD);
      }
      break;
    case S_WORLD:
      switch (world_.step(in)) {
        case WE_WARP:
          warp_ = world_.pendingWarp();
          audio_->play(SFX_CHECK);
          setState(S_FADE);
          break;
        case WE_BUMP:
          audio_->play(SFX_BUMP);
          break;
        case WE_TALK:
          lastTalk_ = world_.npc(world_.talkNpc()).who;
          talkTo(lastTalk_);
          break;
        default:
          break;
      }
      break;
    case S_FADE:
      if (stateTime_ == FADE_HALF && warp_) {
        int from = world_.mapIndex();
        world_.load(warp_->target, warp_->tx, warp_->ty, warp_->dir);
        if (world_.map().music != ALL_MAPS[from]->music) audio_->music(world_.map().music);
        warp_ = nullptr;
      }
      if (stateTime_ >= 2 * FADE_HALF) {
        setState(S_WORLD);
        save();
        enteredMap();
      }
      break;
    case S_DIALOG:
      dialog_.step(in, audio_);
      if (!dialog_.active()) {
        setState(S_WORLD);
        save();
      }
      break;
  }
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
      break;
    case S_DIALOG:
      world_.render(b);
      dialog_.render(b);
      break;
    case S_FADE: {
      world_.render(b);
      int t = stateTime_ <= FADE_HALF ? stateTime_ : 2 * FADE_HALF - stateTime_;
      drawFade(b, t * (SCREEN_H / 2) / FADE_HALF);
      break;
    }
  }
  if (savedShow_ > 0 && (state_ == S_WORLD || state_ == S_DIALOG) && b.y0 < 20) {
    // petit rappel en haut a droite : la partie vient d'etre enregistree
    int w = textWidth(font_small, "Uložené") + 12;
    b.fillRect(SCREEN_W - w - 4, 4, w, 15, DARKBLUE);
    b.text(font_small, SCREEN_W - w + 2, 5, "Uložené", WHITE, -1);
  }
  if (volumeShow_ > 0) drawVolume(b);
}

// ---------------------------------------------------------------- histoire du chapitre 1
void Game::say(const Line* lines, int n, bool withBonus) {
  if (n <= 0) return;
  const Episode& ep = episode1();
  if (withBonus) dialog_.start(lines, n, COURSE_DEFAULT_NAME, ep.bonus, ep.nBonus, &bonusUsed_);
  else dialog_.start(lines, n, COURSE_DEFAULT_NAME);
  setState(S_DIALOG);
}

// En arrivant sur une carte : le narrateur plante le decor du tram, la premiere fois.
void Game::enteredMap() {
  const Episode& ep = episode1();
  if (world_.mapIndex() == MAP_KOSICE_TRAM && !introDone_) {
    introDone_ = true;
    say(ep.dialog, introLength(ep), false);
  }
}

void Game::talkTo(Speaker who) {
  const Episode& ep = episode1();
  if (who == SPK_ANDREA) {
    if (!metAndrea_) {
      metAndrea_ = true;
      int k = introLength(ep);
      say(ep.dialog + k, ep.nDialog - k, true);
    } else {
      // ensuite, Andrea redit sa derniere replique de l'episode
      for (int i = ep.nDialog - 1; i >= 0; i--)
        if (ep.dialog[i].speaker == SPK_ANDREA) {
          say(&ep.dialog[i], 1, false);
          break;
        }
    }
    return;
  }
  audio_->play(SFX_BUMP);  // personne d'autre n'a encore de repliques
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
  b.frame(spr_fox, IDLE_ROW * spr_fox.cols + f, 120, 132);
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
  centerText(b, font_small, 78, "KAPITOLA 1", YELLOW, BLACK);
  centerText(b, font_title, 92, "Košice", WHITE, BLACK);
  centerText(b, font_small, 128, "To si ty?", GREY, BLACK);
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
