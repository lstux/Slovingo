// Zajka Quest : aventure en vue de dessus. Logique a 60 Hz (step), rendu par bandes (renderBand).
// Ecran titre (continuer / nouvelle partie), carte de chapitre, le monde (world.h) et les
// dialogues (dialog.h). La partie est sauvegardee toute seule (save.h) en passant une porte et a
// la fin de chaque conversation.
#pragma once
#include <stdint.h>

#include "audio.h"
#include "dialog.h"
#include "gfx.h"
#include "input.h"
#include "save.h"
#include "world.h"

// S_FADE : passage d'une carte a l'autre (rideau noir qui se ferme puis se rouvre).
// S_DIALOG : une conversation, par-dessus le monde fige.
enum GameState : uint8_t { S_TITLE, S_CHAPTER, S_WORLD, S_FADE, S_DIALOG };

class Game {
 public:
  void begin(Audio* audio, Storage* storage = nullptr);
  void step(const Input& in);  // 1/60 s
  void renderBand(Band& b);
  uint32_t frameCount() const { return frame_; }
  GameState state() const { return state_; }
  const World& world() const { return world_; }
  Speaker lastTalk() const { return lastTalk_; }  // dernier PNJ a qui le renard a parle
  const Dialog& dialog() const { return dialog_; }
  int bonusFound() const { return bonusUsed_; }   // repliques secretes gagnees
  bool hasSave() const { return hasSave_; }

 private:
  void setState(GameState s);
  void drawBackdrop(Band& b);
  void drawTitle(Band& b);
  void drawChapter(Band& b);
  void drawFade(Band& b, int covered);
  void talkTo(Speaker who);
  void enteredMap();
  void say(const Line* lines, int n, bool withBonus);
  void stepTitle(const Input& in);
  void newGame();
  bool continueGame();
  void save();
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
  Dialog dialog_;
  Storage* storage_ = nullptr;
  bool hasSave_ = false;
  SaveData saved_;
  int menu_ = 0;           // ecran titre : 0 continuer, 1 nouvelle partie
  bool confirm_ = false;   // « effacer la partie ? »
  int savedShow_ = 0;      // images restantes du petit « Uloz(e)ne »
  // avancement du chapitre 1
  bool introDone_ = false;  // le narrateur a plante le decor du tram
  bool metAndrea_ = false;  // le dialogue de l'episode 1 a ete joue
  int bonusUsed_ = 0;
};
