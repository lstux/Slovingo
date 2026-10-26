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
  E_JET,        // jet de la fontaine : crachote, puis propulse vers le haut
  E_WIND,       // zone de vent (rafales vers la gauche)
  E_ROCKSPAWN,  // en haut d'une paroi : laisse tomber des pierres
  E_ROCK,       // pierre qui tombe
  E_HUT,        // decor : la chata (refuge) des Tatras
};

// Sauvegarde : niveaux debloques, meilleurs temps et medovniky par niveau.
struct SaveData {
  uint32_t magic = 0x5A52554E;  // "ZRUN"
  uint8_t version = 3, unlocked = 1, pad[2] = {0, 0};
  uint16_t bestTime[8] = {0};   // en secondes, 0 = jamais termine
  uint8_t bestHearts[8] = {0};
  uint8_t bestSecrets[8] = {0};   // passages secrets trouves par niveau
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

// Position et pose d'un personnage, pour le trace du compagnon (sans collisions)
struct Snap {
  float x = 0, y = 0;
  uint8_t anim = 0, frame = 0;
  bool left = false;
};

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
  void selectCharacter(bool rabbit) { rabbit_ = rabbit; }  // simulateur : Andrea (true) ou le renard (avant le debut)
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
  bool companionDown() const { return compDown_ > 0; }

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
  void updateCrumbles();
  void updateWeather();
  void postProcess(Band& b);
  void updateEntities();
  void interact();
  void hurtPlayer(float fromX);
  void killPlayer();
  void bumpTile(int tx, int ty);
  void revealSecrets();
  bool hiddenBySecret(const Entity& e) const;
  void collectHeart(float x, float y, bool popped);
  int spawn(EType t, float x, float y);
  void fx(EType t, float x, float y, const char* text = nullptr);
  void updateCamera(bool snap);
  bool solidAt(int px, int py) const;
  // rendu
  void drawWorld(Band& b);
  void drawEntity(Band& b, const Entity& e);
  void drawPlayer(Band& b);
  void drawFollower(Band& b);
  void drawChar(Band& b, bool rabbit, int row, int f, bool flip, float x, float y);
  // duo : a partir du niveau 2, les deux personnages avancent ensemble
  bool duo() const { return levelIndex_ >= 1; }
  void resetTrail();
  void recordTrail();
  Snap trailAt(int age) const;
  void switchChar(bool onDeath);
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
  bool rabbit_ = false;  // personnage actif : false = le renard, true = Andrea (rejointe a la fin du niveau 1)
  // compagnon : l'autre personnage suit le trace du personnage actif, avec un retard de TRAIL_DELAY images
  static constexpr int TRAIL_LEN = 32, TRAIL_DELAY = 10;
  static constexpr int REVIVE_FRAMES = 150;   // 2,5 s avant le retour du personnage tombe
  Snap trail_[TRAIL_LEN];
  int trailHead_ = 0;
  int compHp_ = 3;      // points de vie du compagnon (echanges avec ceux du personnage actif)
  int compDown_ = 0;    // images avant le retour du compagnon tombe (0 = debout)
  bool switched_ = false;  // un personnage vient de tomber : ne pas traiter le reste de la logique
  struct Crumble {
    int16_t tx, ty, t;
  };
  // passages secrets du niveau : faux murs qui disparaissent quand on y entre
  struct SecretRegion {
    bool revealed;
    int t0;
    int16_t ox, oy;
  };
  SecretRegion secrets_[16];
  int secretsFound_ = 0;
  Crumble crumbles_[24];
  int ncrumbles_ = 0;
  int weatherLevel_ = 0;  // 0 rien, 1 pluie, 2 pluie et nuit
  int windT_ = 0;         // dans une rafale (images restantes)
  int flash_ = 0, thunderIn_ = 0, nextBolt_ = 240;
  uint32_t rng_ = 12345;
  uint32_t rnd() { rng_ = rng_ * 1103515245u + 12345u; return rng_ >> 8; }
};
