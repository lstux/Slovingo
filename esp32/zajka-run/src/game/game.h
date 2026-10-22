// Zajka Run : plateformer. Logique a 60 Hz (step), rendu par bandes (renderBand).
#pragma once
#include <stdint.h>

#include "audio.h"
#include "gfx.h"
#include "input.h"
#include "level.h"

enum EType : uint8_t {
  E_NONE, E_HEART, E_PIGEON, E_WASP, E_SPRING, E_CHECK, E_GOAL, E_TRAM,
  E_BENCH, E_FLOWERS, E_POP, E_DUST, E_SPARK, E_TEXT,
};

struct Entity {
  EType type = E_NONE;
  float x = 0, y = 0, vx = 0, vy = 0;  // x, y : coin haut-gauche de la boite de collision
  int16_t w = 0, h = 0;
  int16_t timer = 0;
  uint8_t state = 0;
  int8_t dir = 1;
  float baseX = 0, baseY = 0;
  const char* text = nullptr;
};

enum GameState : uint8_t { S_TITLE, S_INTRO, S_PLAY, S_PAUSE, S_DYING, S_CLEAR, S_GAMEOVER, S_END };

struct Player {
  float x = 0, y = 0, vx = 0, vy = 0;
  bool onGround = false;
  int coyote = 0, jumpBuf = 0;
  bool facingLeft = false;
  int invuln = 0;
  int hp = 3;
  int anim = 0, animTime = 0, animFrame = 0;
  int riding = -1;  // index de l'entite qui le porte (tram)
  int squash = 0;
};

class Game {
 public:
  static constexpr int PW = 14, PH = 38;    // boite de collision du renard
  static constexpr int MAX_ENT = 200;

  void begin(Audio* audio);
  void step(const Input& in);               // 1/60 s
  void renderBand(Band& b);
  uint32_t frameCount() const { return frame_; }
  GameState state() const { return state_; }
  // pour le simulateur et les tests
  const Player& player() const { return p_; }
  int hearts() const { return hearts_; }
  int lives() const { return lives_; }
  void startLevel(int index, bool fresh);
  const Level& level() const { return lvl_; }
  const Entity* entities(int& n) const { n = nent_; return ent_; }
  float camX() const { return camX_; }
  void debugTeleport(float x);  // simulateur : place le renard (et le point de passage) en x

 private:
  // logique
  void updatePlay(const Input& in);
  void updatePlayer(const Input& in);
  void movePlayerX(float dx);
  void movePlayerY(float dy, float prevBottom);
  void updateEntities();
  void interact();
  void hurtPlayer(float fromX);
  void killPlayer();
  void bumpTile(int tx, int ty);
  void collectHeart(float x, float y, bool popped);
  int spawn(EType t, float x, float y);
  void fx(EType t, float x, float y, const char* text = nullptr);
  void updateCamera(bool snap);
  bool solidAt(int px, int py) const;
  // rendu
  void drawWorld(Band& b);
  void drawEntity(Band& b, const Entity& e);
  void drawPlayer(Band& b);
  void drawHud(Band& b);
  void drawTitle(Band& b);
  void drawOverlay(Band& b);
  void centerText(Band& b, const Font& f, int y, const char* s, uint16_t c, int shadow = 0);

  Audio* audio_ = nullptr;
  Level lvl_;
  int levelIndex_ = 0;
  Player p_;
  Entity ent_[MAX_ENT];
  int nent_ = 0;
  GameState state_ = S_TITLE;
  int stateTime_ = 0;
  uint32_t frame_ = 0;
  float camX_ = 0, camY_ = 0, lookAhead_ = 0;
  int hearts_ = 0, lives_ = 3, levelHearts_ = 0;
  float checkX_ = 0, checkY_ = 0;
  int levelTime_ = 0;
  int bumpTx_ = -1, bumpTy_ = -1, bumpT_ = 0;
  int shake_ = 0;
  int volumeShow_ = 0;
  bool holdA_ = false;
  bool respawn_ = false;
};
