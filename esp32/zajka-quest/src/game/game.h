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
// S_DIALOG : une conversation, par-dessus le monde fige. S_END : fin du chapitre.
enum GameState : uint8_t { S_TITLE, S_CHAPTER, S_WORLD, S_FADE, S_DIALOG, S_END };

class Game {
 public:
  void begin(Audio* audio, Storage* storage = nullptr);
  void step(const Input& in);  // 1/60 s
  void renderBand(Band& b);
  uint32_t frameCount() const { return frame_; }
  GameState state() const { return state_; }
  const World& world() const { return world_; }
  Speaker lastTalk() const { return lastTalk_; }  // dernier PNJ a qui le renard a parle
  int talks() const { return talks_; }            // conversations engagees depuis le debut
  const Dialog& dialog() const { return dialog_; }
  int bonusFound() const;                         // repliques secretes gagnees (tous episodes)
  int bonusTotal() const;                         // repliques secretes a gagner
  int stage() const { return stage_; }            // episodes joues en entier
  bool hasSave() const { return hasSave_; }

 private:
  void setState(GameState s);
  void drawBackdrop(Band& b);
  void drawTitle(Band& b);
  void drawChapter(Band& b);
  void drawFade(Band& b, int covered);
  void drawBanner(Band& b);
  void drawEnd(Band& b);
  // histoire du chapitre 1 (story.cpp)
  enum After : uint8_t { AFTER_NOTHING, AFTER_EPISODE, AFTER_SLEEP };
  void storyEnteredMap();
  void storyTalk(int npc);
  void storyInspect(int tx, int ty);
  void storyDialogDone();
  void playEpisode(int ep, int from, int to, After after, bool banner);
  void say(const Line* lines, int n, int bonusEpisode, After after);
  void andreaRepeats();
  void goTo(int map, int tx, int ty, Dir dir);
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
  MapMark autoWarp_ = {};          // passage impose par l'histoire (fin de l'episode 5)
  Speaker lastTalk_ = SPK_NONE;
  int talks_ = 0;
  Dialog dialog_;
  Storage* storage_ = nullptr;
  bool hasSave_ = false;
  SaveData saved_;
  int menu_ = 0;           // ecran titre : 0 continuer, 1 nouvelle partie
  bool confirm_ = false;   // « effacer la partie ? »
  int savedShow_ = 0;      // images restantes du petit « Uloz(e)ne »
  // avancement du chapitre 1
  int stage_ = 0;           // episodes joues en entier (0 a 6)
  uint8_t flags_ = 0;       // SaveFlag
  int bonus_[SAVE_EPISODES] = {};  // repliques secretes gagnees, par episode
  After after_ = AFTER_NOTHING;    // ce qui suit la conversation en cours
  int banner_ = 0, bannerEp_ = 0;  // titre de l'episode qui commence (images restantes)
};
