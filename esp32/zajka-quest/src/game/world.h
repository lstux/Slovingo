// Monde en vue de dessus : cartes en tuiles de 16 px, deplacement case par case (a la Pokemon),
// collisions, PNJ, portes vers d'autres cartes, camera. Portable (simulateur PC compris).
//
// Une carte (maps/*.txt, convertie par tools/build_assets.py) a deux calques : le sol et le decor
// (base), et ce qui passe devant les personnages (over : feuillage des arbres...).
#pragma once
#include <stdint.h>

#include "course.h"
#include "gfx.h"
#include "input.h"

constexpr int TILE = 16;
constexpr int CHAR_W = 16, CHAR_H = 24;  // personnages : pieds sur la case, tete sur celle du dessus

enum Dir : int8_t { DIR_DOWN, DIR_UP, DIR_LEFT, DIR_RIGHT };  // = ligne de la planche de marche

enum TileKind : uint8_t { TK_WALK, TK_SOLID, TK_OVER };

struct Tileset {
  const Image8* img;     // planche de tuiles 16 x 16, 16 par ligne ; la tuile 0 est le vide
  const uint8_t* kinds;  // TileKind de chaque tuile
  uint16_t count;
};

enum MarkType : uint8_t { M_START, M_WARP, M_NPC };

// Repere pose sur la carte (chiffre dans le fichier texte).
struct MapMark {
  MarkType type;
  uint8_t x, y;
  Dir dir;           // depart / PNJ : orientation ; porte : orientation a l'arrivee
  int8_t target;     // porte : index de la carte d'arrivee dans ALL_MAPS
  uint8_t tx, ty;    // porte : case d'arrivee
  Speaker npc;       // PNJ : qui c'est
};

struct MapDef {
  const char* id;    // nom du fichier, sans .txt
  const char* name;  // nom affiche
  const Tileset* tiles;
  uint8_t w, h;
  const uint8_t* base;
  const uint8_t* over;  // nullptr si rien ne passe devant
  const MapMark* marks;
  uint8_t nMarks;
  int8_t music;
};

constexpr int MAX_NPC = 16;

struct Walker {
  int tx = 0, ty = 0;  // case occupee (destination pendant un pas)
  int px = 0, py = 0;  // position en pixels du coin haut-gauche de la case des pieds
  Dir dir = DIR_DOWN;
  int left = 0;        // pixels restant a parcourir pour finir le pas
  int steps = 0;       // pas faits (alterne la jambe)
  const Sheet* sheet = nullptr;
  Speaker who = SPK_NONE;
};

enum WorldEvent : uint8_t { WE_NONE, WE_WARP, WE_TALK, WE_BUMP };

class World {
 public:
  void load(int mapIndex, int tx, int ty, Dir dir);  // carte + position du renard
  void loadStart(int mapIndex);                       // au repere de depart de la carte
  // 1/60 s. WE_WARP : le renard est sur une porte (warpTarget...) ; WE_TALK : A face a un PNJ (talkNpc).
  WorldEvent step(const Input& in);
  void render(Band& b);
  void snapCamera();

  const MapDef& map() const { return *map_; }
  int mapIndex() const { return mapIndex_; }
  const Walker& player() const { return p_; }
  Walker& npc(int i) { return npc_[i]; }
  const Walker& npc(int i) const { return npc_[i]; }
  int npcCount() const { return nnpc_; }
  const MapMark* pendingWarp() const { return warp_; }
  int talkNpc() const { return talk_; }

 private:
  bool walkable(int tx, int ty) const;
  int npcAt(int tx, int ty) const;
  void moveWalker(Walker& w, int speed);
  void updateCamera();
  void drawTiles(Band& b, const uint8_t* layer, bool skipEmpty);
  void drawWalker(Band& b, const Walker& w);

  const MapDef* map_ = nullptr;
  int mapIndex_ = 0;
  Walker p_;
  Walker npc_[MAX_NPC];
  int nnpc_ = 0;
  int camX_ = 0, camY_ = 0;
  int turn_ = 0;      // images avant de marcher apres avoir tourne sur place
  int bumpWait_ = 0;
  bool waitRelease_ = false;  // apres une porte : on attend que la croix soit relachee
  const MapMark* warp_ = nullptr;
  int talk_ = -1;
};

extern const MapDef* const ALL_MAPS[];
extern const int NUM_MAPS;
