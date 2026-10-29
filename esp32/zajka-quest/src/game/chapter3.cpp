// Chapitre 3 : Slovensky raj (episodes 1 a 6 de la serie Raj du cours, COURSE_EPISODES[12..17]).
//
//   gare        Spisska Nova Ves : l'episode 1 (« Kam teraz? »). Andrea suit le renard ensuite.
//   foret       trop silencieuse : l'episode 2 (« Pocujes? »). Une « oreille » en bas de l'ecran dit si
//               on s'approche du ruisseau. Au ruisseau : le debut de l'episode 3 (« Je tu voda! »), le
//               renard boit (A face a l'eau), puis Andrea : la fin de l'episode 3.
//   gorge       l'episode 4 (« Rebriky ») : Andrea passe devant, le renard traverse seul un champ de
//               planches fragiles (chacune cede quand il la quitte ; l'eau fait recommencer), puis une
//               echelle et un pont jusqu'a elle.
//   pan Orol    l'episode 5 (« Roklina ») : le garde du parc, une veste, direction les Tatry.
//   camp        l'episode 6 (« Pri potoku ») : la nuit au bord du ruisseau. Fin.
//
// stage_ = episodes joues en entier (0 a 6) ; flags_ = F3_...
#include "game.h"
#include "gen/assets.h"
#include "gen/scripts.h"

namespace {
enum : uint16_t {
  F3_INTRO = 1 << 0,   // l'episode 1 est joue (ou en cours)
  F3_LES1 = 1 << 1,    // la premiere moitie de l'episode 2 est jouee : on cherche le ruisseau
  F3_ASKED = 1 << 2,   // le debut de l'episode 3 est joue : « Najprv sa napi »
  F3_DRANK = 1 << 3,   // le renard a bu
  F3_ALONE = 1 << 4,   // Andrea attend de l'autre cote de la gorge
  F3_JOINED = 1 << 5,  // le renard l'a rejointe
  F3_CAMP = 1 << 6,    // la nuit au camp est jouee
};
enum { EV3_LES1 = 1, EV3_ASKED, EV3_DRANK, EV3_FALL, EV3_JOIN };

constexpr int EP_FIRST = 12;  // index global du premier episode du chapitre
constexpr int S2_SPLIT = 7;   // episode 2 : « Niekde blizko tecie voda » (la suite se joue au ruisseau)
constexpr int S3_SPLIT = 5;   // episode 3 : « Ty si zase sef » (apres avoir bu)
const uint16_t WHITE = 0xFFFF, BLACK = 0x0000, DARKBLUE = 0x1926;
constexpr int ENTRY_X = 7, ENTRY_Y = 8;  // la dalle de depart devant les planches

const Episode& ep(int i) { return *COURSE_EPISODES[EP_FIRST + i]; }

const Line* lastLine(const Episode& e, Speaker who) {
  for (int i = e.nDialog - 1; i >= 0; i--)
    if (e.dialog[i].speaker == who) return &e.dialog[i];
  return nullptr;
}

// 0 (silence) a 5 (le ruisseau est tout pres) : distance a la zone « potok »
int listenLevel(const World& w) {
  const MapDef& m = w.map();
  for (int i = 0; i < m.nMarks; i++) {
    const MapMark& k = m.marks[i];
    if (k.type != M_ZONE || k.tag != TAG_POTOK) continue;
    int dx = w.player().tx - k.x, dy = w.player().ty - k.y;
    int d = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
    return d <= 3 ? 5 : d <= 7 ? 4 : d <= 12 ? 3 : d <= 18 ? 2 : d <= 26 ? 1 : 0;
  }
  return 0;
}
}  // namespace

int Game::ch3Goal() const {
  int m = world_.mapIndex();
  if (flag(F3_CAMP)) return SC_CIEL_HOTOVO;
  switch (stage_) {
    case 0:
    case 1:
      if (m == MAP_RAJ_STANICA || !flag(F3_LES1)) return m == MAP_RAJ_STANICA ? SC_CIEL_LES : SC_CIEL_POTOK;
      return SC_CIEL_POTOK;
    case 2: return flag(F3_DRANK) ? SC_CIEL_ANDREA : SC_CIEL_NAPI;
    case 3: return SC_CIEL_ROKLINA;
    case 4:
      if (m == MAP_RAJ_OROL) return SC_CIEL_OROL;
      if (!flag(F3_JOINED)) return flag(F3_ALONE) && m == MAP_RAJ_ROKLINA ? SC_CIEL_DOSKY : SC_CIEL_ROKLINA;
      return SC_CIEL_OROL;
    case 5: return SC_CIEL_TABOR;
    default: return SC_CIEL_HOTOVO;
  }
}

void Game::ch3Refresh() {
  int m = world_.mapIndex();
  bool follows = stage_ >= 1 && !flag(F3_ALONE);
  if (follows && !world_.hasFollower()) world_.setFollower(SPK_ANDREA);
  if (!follows && world_.hasFollower()) world_.setFollower(SPK_NONE);
  if (m == MAP_RAJ_ROKLINA) world_.setNpcHidden(TAG_ANDREA_ROKLINA, !flag(F3_ALONE) || flag(F3_JOINED));
}

void Game::ch3EnteredMap() {
  int m = world_.mapIndex();
  plankX_ = plankY_ = -1;
  listen_ = (m == MAP_RAJ_LES && stage_ == 1 && flag(F3_LES1)) ? listenLevel(world_) : 0;
  if (m == MAP_RAJ_STANICA && stage_ == 0 && !flag(F3_INTRO)) {
    setFlag(F3_INTRO);
    playEpisode(EP_FIRST, 0, ep(0).nDialog, AFTER_EPISODE, true);
  } else if (m == MAP_RAJ_LES && stage_ == 2 && !flag(F3_ASKED)) {  // partie reprise au ruisseau
    playEpisode(EP_FIRST + 2, 0, S3_SPLIT, AFTER_EVENT, true, EV3_ASKED);
  } else if (m == MAP_RAJ_TABORISKO && stage_ == 5 && !flag(F3_CAMP)) {
    setFlag(F3_CAMP);
    playEpisode(EP_FIRST + 5, 0, ep(5).nDialog, AFTER_SLEEP, true);
  }
}

bool Game::ch3Warp(const MapMark& m) {
  int cur = world_.mapIndex();
  if (cur == MAP_RAJ_STANICA && stage_ < 1) return false;
  if (cur == MAP_RAJ_LES && m.target == MAP_RAJ_ROKLINA && stage_ < 3) {
    sayScript(stage_ < 2 ? SC_ANDREA_NAJPRV_POTOK : SC_ANDREA_NAJPRV_PIT);
    return false;
  }
  if (cur == MAP_RAJ_ROKLINA && m.target == MAP_RAJ_OROL && !flag(F3_JOINED)) {
    sayScript(flag(F3_ALONE) ? SC_ANDREA_POD_SEM : SC_ANDREA_DOSKY);
    return false;
  }
  if (cur == MAP_RAJ_OROL && m.target == MAP_RAJ_TABORISKO && stage_ < 5) {
    sayScript(SC_ANDREA_NAJPRV_OROL);
    return false;
  }
  return true;
}

void Game::ch3Zone(const MapMark& m) {
  if (m.tag == TAG_LES1 && stage_ == 1 && !flag(F3_LES1)) {
    playEpisode(EP_FIRST + 1, 0, S2_SPLIT, AFTER_EVENT, true, EV3_LES1);
  } else if (m.tag == TAG_POTOK && stage_ == 1 && flag(F3_LES1)) {
    playEpisode(EP_FIRST + 1, S2_SPLIT, ep(1).nDialog, AFTER_EPISODE, false);
  } else if (m.tag == TAG_ROKLINA && stage_ == 3) {
    playEpisode(EP_FIRST + 3, 0, ep(3).nDialog, AFTER_EPISODE, true);
  } else if (m.tag == TAG_OROL && stage_ == 4) {
    playEpisode(EP_FIRST + 4, 0, ep(4).nDialog, AFTER_EPISODE, true);
  }
}

void Game::ch3Talk(const Walker& w) {
  int m = world_.mapIndex();
  if (w.who == SPK_ANDREA) {
    if (stage_ == 2) {
      if (flag(F3_DRANK)) playEpisode(EP_FIRST + 2, S3_SPLIT, ep(2).nDialog, AFTER_EPISODE, false);
      else sayScript(SC_ANDREA_NAJPRV_PIT);
    } else if (stage_ == 1 && m != MAP_RAJ_STANICA) {
      sayScript(SC_ANDREA_NAJPRV_POTOK);
    } else if (m == MAP_RAJ_ROKLINA && flag(F3_ALONE) && !flag(F3_JOINED)) {
      const Line* l = lastLine(ep(3), SPK_ANDREA);  // « Zajacica. A silna. »
      if (l) say(l, 1, -1, AFTER_EVENT, EV3_JOIN);
    } else {
      sayLast(EP_FIRST + (stage_ > 0 ? (stage_ > 6 ? 5 : stage_ - 1) : 0), SPK_ANDREA);
    }
    return;
  }
  if (w.who == SPK_OROL) {
    if (stage_ == 4) playEpisode(EP_FIRST + 4, 0, ep(4).nDialog, AFTER_EPISODE, true);
    else sayLast(EP_FIRST + 4, SPK_OROL);
    return;
  }
  if (w.who == SPK_BIRD) {
    if (stage_ < 2) sayScript(SC_VTAK_TICHO);
    else passerby(w);
    return;
  }
  passerby(w);
}

void Game::ch3Inspect(int tx, int ty) {
  if (world_.mapIndex() == MAP_RAJ_LES && world_.tileAt(tx, ty) == TL_RUISSEAU && stage_ == 2 && flag(F3_ASKED) &&
      !flag(F3_DRANK))
    sayScript(SC_NAPIL_SI_SA, AFTER_EVENT, EV3_DRANK);
}

void Game::ch3Event(int event) {
  switch (event) {
    case EV3_LES1:
      setFlag(F3_LES1);
      listen_ = listenLevel(world_);
      break;
    case EV3_ASKED:
      setFlag(F3_ASKED);
      break;
    case EV3_DRANK:
      setFlag(F3_DRANK);
      give(ITEM_FLASA);
      break;
    case EV3_FALL:  // dans l'eau : on recommence devant les planches
      goTo(MAP_RAJ_ROKLINA, ENTRY_X, ENTRY_Y, DIR_RIGHT);
      break;
    case EV3_JOIN:
      setFlag(F3_JOINED);
      break;
  }
}

void Game::ch3EpisodeDone() {
  if (stage_ == 2) {
    // le ruisseau : « Potok je maly... Najprv sa napi. »
    playEpisode(EP_FIRST + 2, 0, S3_SPLIT, AFTER_EVENT, true, EV3_ASKED);
  } else if (stage_ == 4 && world_.mapIndex() == MAP_RAJ_ROKLINA) {
    // « Dobre. Idem. » : Andrea passe devant, le renard traversera seul
    setFlag(F3_ALONE);
    sayScript(SC_ANDREA_DOSKY);
  } else if (stage_ == 5) {
    give(ITEM_BUNDA);  // « A vezmite si bundu. »
  }
}

// Le renard vient d'arriver sur la case (tx, ty).
void Game::ch3Step(int tx, int ty) {
  int m = world_.mapIndex();
  if (m == MAP_RAJ_LES && stage_ == 1 && flag(F3_LES1)) {
    int lvl = listenLevel(world_);
    if (lvl > listen_) audio_->play(SFX_COIN);
    listen_ = lvl;
    return;
  }
  if (m != MAP_RAJ_ROKLINA) return;
  // une planche cede des que le renard la quitte
  if (plankX_ >= 0 && (plankX_ != tx || plankY_ != ty)) {
    world_.setTile(plankX_, plankY_, TL_EAU_GORGE);
    audio_->play(SFX_BUMP);
    plankX_ = plankY_ = -1;
  }
  int t = world_.tileAt(tx, ty);
  if (t == TL_PLANCHE) {
    plankX_ = tx;
    plankY_ = ty;
  } else if (t == TL_EAU_GORGE) {
    audio_->play(SFX_HURT);
    sayScript(SC_ANDREA_TOMBA, AFTER_EVENT, EV3_FALL);
  }
}

// L'oreille de la foret : cinq barres, de plus en plus hautes quand on approche du ruisseau.
void Game::ch3Hud(Band& b) {
  if (world_.mapIndex() != MAP_RAJ_LES || stage_ != 1 || !flag(F3_LES1)) return;
  int lvl = listenLevel(world_);
  const int x0 = 8, y0 = SCREEN_H - 30;
  if (b.y0 + b.h <= y0 - 4) return;
  b.fillRect(x0 - 4, y0 - 4, 84, 26, DARKBLUE);
  b.text(font_small, x0, y0 - 2, "Počuješ?", WHITE, -1);
  for (int i = 0; i < 5; i++) {
    int h = 3 + i * 2;
    b.fillRect(x0 + 48 + i * 6, y0 + 15 - h, 4, h, i < lvl ? WHITE : BLACK);
  }
}
