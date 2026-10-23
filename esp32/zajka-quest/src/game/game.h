// Zajka Quest : aventure en vue de dessus. Logique a 60 Hz (step), rendu par bandes (renderBand).
// Ecran titre, carte de chapitre, puis le monde (world.h). Les dialogues et la sauvegarde
// arrivent dans les etapes suivantes (voir le brief du projet).
#pragma once
#include <stdint.h>

#include "audio.h"
#include "gfx.h"
#include "input.h"
#include "world.h"

// S_FADE : passage d'une carte a l'autre (rideau noir qui se ferme puis se rouvre).
enum GameState : uint8_t { S_TITLE, S_CHAPTER, S_WORLD, S_FADE };

class Game {
 public:
  void begin(Audio* audio);
  void step(const Input& in);  // 1/60 s
  void renderBand(Band& b);
  uint32_t frameCount() const { return frame_; }
  GameState state() const { return state_; }
  const World& world() const { return world_; }
  Speaker lastTalk() const { return lastTalk_; }  // dernier PNJ a qui le renard a parle

 private:
  void setState(GameState s);
  void drawBackdrop(Band& b);
  void drawTitle(Band& b);
  void drawChapter(Band& b);
  void drawFade(Band& b, int covered);
  void drawVolume(Band& b);
  void centerText(Band& b, const Font& f, int y, const char* s, uint16_t c, int shadow = 0);

  Audio* audio_ = nullptr;
  GameState state_ = S_TITLE;
  int stateTime_ = 0;
  uint32_t frame_ = 0;
  int volumeShow_ = 0;
  World world_;
  const MapMark* warp_ = nullptr;  // porte en cours de franchissement (S_FADE)
  Speaker lastTalk_ = SPK_NONE;
};
