// Zajka Quest : aventure en vue de dessus. Logique a 60 Hz (step), rendu par bandes (renderBand).
// Ecran titre (continuer / nouvelle partie), carte de chapitre, le monde (world.h), les
// dialogues (dialog.h), le menu START (Batoh : objets, Slovnik : mots trouves, Ciel : objectif).
// La partie est sauvegardee toute seule (save.h) en passant une porte et apres chaque conversation.
// L'histoire de chaque chapitre est dans chapter1.cpp, chapter2.cpp (outils communs : story.cpp).
#pragma once
#include <stdint.h>

#include "audio.h"
#include "dialog.h"
#include "gfx.h"
#include "input.h"
#include "save.h"
#include "world.h"

// S_FADE : passage d'une carte a l'autre (rideau). S_DIALOG : une conversation, par-dessus le monde
// fige. S_NOTE : un petit message (« Nove slovo! », « Do batohu: ... »). S_MENU : le menu START.
// S_END : fin du chapitre.
enum GameState : uint8_t { S_TITLE, S_CHAPTER, S_WORLD, S_FADE, S_DIALOG, S_END, S_NOTE, S_MENU };

constexpr int NUM_CHAPTERS = 2;

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
  int bonusTotal(int chapter = -1) const;         // repliques secretes a gagner (-1 : tout le jeu)
  int chapter() const { return chapter_; }
  int stage() const { return stage_; }            // episodes du chapitre joues en entier
  int itemCount(int item) const { return item >= 0 && item < SAVE_ITEMS ? items_[item] : 0; }
  int wordsFound() const;
  bool hasSave() const { return hasSave_; }
  int goal() const;                               // objectif du moment (SC_CIEL_...)
  void debugChapter(int chapter) { startChapter(chapter); }  // simulateur : commencer a ce chapitre

 private:
  void setState(GameState s);
  void drawBackdrop(Band& b);
  void drawTitle(Band& b);
  void drawChapter(Band& b);
  void drawFade(Band& b, int covered);
  void drawBanner(Band& b);
  void drawEnd(Band& b);
  void drawNote(Band& b);
  void drawMenu(Band& b);
  void stepMenu(const Input& in);
  void drawVolume(Band& b);
  void centerText(Band& b, const Font& f, int y, const char* s, uint16_t c, int shadow = 0);
  void stepTitle(const Input& in);
  void newGame();
  bool continueGame();
  void startChapter(int chapter);
  void save();

  // ---- histoire : outils communs (story.cpp)
  enum After : uint8_t { AFTER_NOTHING, AFTER_EPISODE, AFTER_SLEEP, AFTER_EVENT };
  void storyEnteredMap();
  void storyTalk(int npc);
  void storyInspect(int tx, int ty);
  void storySign(const MapMark& m);
  void storyPickup(const MapMark& m);
  void storyZone(const MapMark& m);
  void storyDialogDone();
  bool storyWarp(const MapMark& m);  // false : l'histoire retient le renard (Andrea : « Najprv... »)
  void storyRefresh();  // qui est la, qui suit (apres un chargement de carte ou un changement)
  void playEpisode(int ep, int from, int to, After after, bool banner, int event = 0);
  void say(const Line* lines, int n, int bonusEpisode, After after, int event = 0);
  void sayScript(int script, After after = AFTER_NOTHING, int event = 0);
  void sayLast(int ep, Speaker who);  // la derniere replique de who dans l'episode ep
  void passerby(const Walker& w);     // « repliques en plus » d'un passant
  void goTo(int map, int tx, int ty, Dir dir);
  void give(int item, int count = 1, bool show = true);
  void note(const char* title, const char* sk, const char* fr, int icon = -1);
  bool flag(uint16_t f) const { return (flags_ & f) != 0; }
  void setFlag(uint16_t f) { flags_ |= f; }
  void endChapter();
  // ---- chapitre 1 : Kosice (chapter1.cpp)
  void ch1EnteredMap();
  void ch1Talk(const Walker& w);
  void ch1Inspect(int tx, int ty);
  void ch1Zone(const MapMark& m);
  void ch1Event(int event);
  void ch1EpisodeDone();
  void ch1Refresh();
  bool ch1Warp(const MapMark& m);
  int ch1Goal() const;
  // ---- chapitre 2 : Sucho (chapter2.cpp)
  void ch2EnteredMap();
  void ch2Talk(const Walker& w);
  void ch2Inspect(int tx, int ty);
  void ch2Zone(const MapMark& m);
  void ch2Event(int event);
  void ch2EpisodeDone();
  void ch2Refresh();
  bool ch2Warp(const MapMark& m);
  int ch2Goal() const;

  Audio* audio_ = nullptr;
  GameState state_ = S_TITLE;
  int stateTime_ = 0;
  uint32_t frame_ = 0;
  int volumeShow_ = 0;
  World world_;
  const MapMark* warp_ = nullptr;  // porte en cours de franchissement (S_FADE)
  MapMark autoWarp_ = {};          // passage impose par l'histoire
  Speaker lastTalk_ = SPK_NONE;
  int talks_ = 0;
  Dialog dialog_;
  Storage* storage_ = nullptr;
  bool hasSave_ = false;
  SaveData saved_;
  int menu_ = 0;           // ecran titre : 0 continuer, 1 nouvelle partie
  bool confirm_ = false;   // « effacer la partie ? »
  int savedShow_ = 0;      // images restantes du petit « Ulozene »
  // menu START
  int menuTab_ = 0, menuScroll_ = 0;
  // petit message (S_NOTE)
  const char* noteTitle_ = "";
  const char* noteSk_ = "";
  const char* noteFr_ = "";
  int noteIcon_ = -1;
  bool noteFr_on_ = false;
  // avancement
  int chapter_ = 0;
  int stage_ = 0;                  // episodes du chapitre joues en entier
  uint16_t flags_ = 0;             // drapeaux du chapitre
  uint8_t items_[SAVE_ITEMS] = {};
  uint8_t picked_[SAVE_BITS] = {};
  uint8_t words_[SAVE_BITS] = {};
  int bonus_[SAVE_EPISODES] = {};  // repliques secretes gagnees, par episode
  After after_ = AFTER_NOTHING;    // ce qui suit la conversation en cours
  int afterEvent_ = 0;             // AFTER_EVENT : quel evenement du chapitre
  int banner_ = 0, bannerEp_ = 0;  // titre de l'episode qui commence (images restantes)
};
