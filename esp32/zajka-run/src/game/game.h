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
  E_PLATFORM,   // passerelle mobile (state 0 horizontale, 1 verticale)
  E_BOTTLE,     // bouteille d'eau : rend un coeur
  E_SQUIRREL,   // ecureuil qui bondit vers le renard
};

// Sauvegarde : niveaux debloques, meilleurs temps et medovniky par niveau.
struct SaveData {
  uint32_t magic = 0x5A52554E;  // "ZRUN"
  uint8_t version = 1, unlocked = 1, pad[2] = {0, 0};
  uint16_t bestTime[8] = {0};   // en secondes, 0 = jamais termine
  uint8_t bestHearts[8] = {0};
};

// Stockage de la sauvegarde, fourni par la plateforme (NVS sur la console, fichier sur PC).
class Storage {
 public:
  virtual ~Storage() {}
  virtual bool load(SaveData& d) = 0;
  virtual void save(const SaveData& d) = 0;
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
  int riding = -1;  // index de l'entite qui le porte (tram, passerelle)
  int squash = 0;
  bool climbing = false;  // sur une echelle
  float climbDist = 0;
};

class Game {
 public:
  static constexpr int PW = 14, PH = 38;    // boite de collision du renard
  static constexpr int MAX_ENT = 200;

  void begin(Audio* audio, Storage* storage = nullptr);
  void selectLevel(int i) { sel_ = i; }  // simulateur : niveau choisi sur l'ecran titre
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
  void debugTeleport(float x, float feetY = -1);  // simulateur : place le renard (et le point de passage)

 private:
  // logique
  void updatePlay(const Input& in);
  void updatePlayer(const Input& in);
  void movePlayerX(float dx);
  void movePlayerY(float dy, float prevBottom);
  bool tryStartClimb(const Input& in);
  void updateClimb(const Input& in);
  float platformTop(const Entity& e) const;
  void levelCleared();
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
  Storage* storage_ = nullptr;
  SaveData save_;
  int sel_ = 0;          // niveau choisi sur l'ecran titre
};
