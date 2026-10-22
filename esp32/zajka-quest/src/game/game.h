// Zajka Quest : aventure en vue de dessus. Logique a 60 Hz (step), rendu par bandes (renderBand).
// Squelette : ecran titre et carte de chapitre ; le monde, les dialogues et la sauvegarde
// arrivent dans les etapes suivantes (voir le brief du projet).
#pragma once
#include <stdint.h>

#include "audio.h"
#include "gfx.h"
#include "input.h"

enum GameState : uint8_t { S_TITLE, S_CHAPTER };

class Game {
 public:
  void begin(Audio* audio);
  void step(const Input& in);  // 1/60 s
  void renderBand(Band& b);
  uint32_t frameCount() const { return frame_; }
  GameState state() const { return state_; }

 private:
  void setState(GameState s);
  void drawBackdrop(Band& b);
  void drawTitle(Band& b);
  void drawChapter(Band& b);
  void drawVolume(Band& b);
  void centerText(Band& b, const Font& f, int y, const char* s, uint16_t c, int shadow = 0);

  Audio* audio_ = nullptr;
  GameState state_ = S_TITLE;
  int stateTime_ = 0;
  uint32_t frame_ = 0;
  int volumeShow_ = 0;
};
