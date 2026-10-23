// Chapitre 2 : Sucho (episodes 1 a 6 de la serie Sucho du cours, COURSE_EPISODES[6..11]).
//
//   jardin          le matin, sous le pommier : l'episode 1 (« Je horuco »). Andrea donne un peu
//                   d'eau, puis elle suit le renard partout.
//   rue             en chemin vers le parc : l'episode 2 (« Vela alebo velmi? »). Panneaux, passants.
//   parc            au bord de l'etang presque a sec : l'episode 3 (« Chyba mi voda »). Les animaux
//                   ont soif : trouver cinq gouttes d'eau (parc, rue) et les leur donner. Puis Andrea :
//                   l'episode 4 (« Co budeme robit? ») ; elle a un sac a dos et une carte.
//   centre          la fontaine vide et le jardinier : l'episode 5 (« Fontana »).
//   gare            l'episode 6 (« Na stanici ») : aider Andrea a ecrire un mot a sa tante (morceaux a
//                   remettre dans l'ordre), les billets au guichet, puis le train. Fin.
//
// Andrea retient le renard tant que l'etape d'avant n'est pas faite (« Najprv do parku... »).
// stage_ = episodes joues en entier (0 a 6) ; flags_ = F2_...
#include "game.h"
#include "gen/assets.h"
#include "gen/scripts.h"

namespace {
enum : uint16_t {
  F2_GIVEN = 1 << 0,          // l'eau donnee aux animaux
  F2_STATION_INTRO = 1 << 1,  // arrivee a la gare jouee
  F2_NOTE = 1 << 2,           // le mot a la tante est ecrit
  F2_TICKETS = 1 << 3,        // billets achetes
};
enum { EV2_STATION = 1, EV2_NOTE, EV2_NOTE_READ, EV2_DROPS, EV2_TICKETS, EV2_BOARD };

constexpr int EP_FIRST = 6;  // index global du premier episode du chapitre
constexpr int DROPS_NEEDED = 5;

const Episode& ep(int i) { return *COURSE_EPISODES[EP_FIRST + i]; }

// la gare : arrivee (0 a 4), le mot ecrit (5), le guichet (6 a 10), le train (11)
constexpr int S6_NOTE = 5, S6_DESK = 6, S6_TRAIN = 11;
}  // namespace

int Game::ch2Goal() const {
  switch (stage_) {
    case 0:
    case 1:
    case 2: return SC_CIEL_PARK;
    case 3: return flag(F2_GIVEN) ? SC_CIEL_ANDREA : SC_CIEL_KVAPKY;
    case 4: return SC_CIEL_FONTANA;
    case 5:
      if (world_.mapIndex() != MAP_SUCHO_STANICA) return SC_CIEL_STANICA;
      return flag(F2_TICKETS) ? SC_CIEL_VLAK : SC_CIEL_OKIENKO;
    default: return SC_CIEL_HOTOVO;
  }
}

void Game::ch2Refresh() {
  // a partir de l'episode 2, Andrea suit le renard (celle posee sur la carte du jardin disparait)
  if (stage_ >= 1 && !world_.hasFollower()) world_.setFollower(SPK_ANDREA);
}

void Game::ch2EnteredMap() {
  int m = world_.mapIndex();
  if (m == MAP_SUCHO_ZAHRADA && stage_ == 0) {
    playEpisode(EP_FIRST, 0, ep(0).nDialog, AFTER_EPISODE, true);
  } else if (m == MAP_SUCHO_STANICA && stage_ == 5 && !flag(F2_STATION_INTRO)) {
    setFlag(F2_STATION_INTRO);
    playEpisode(EP_FIRST + 5, 0, S6_NOTE, AFTER_EVENT, true, EV2_STATION);
  } else if (m == MAP_SUCHO_STANICA && stage_ == 5 && !flag(F2_NOTE)) {
    sayScript(SC_ANDREA_TETA_LISTOK, AFTER_EVENT, EV2_NOTE);  // partie reprise avant le mot
  }
}

bool Game::ch2Warp(const MapMark& m) {
  if (world_.mapIndex() != MAP_SUCHO_CESTA) return true;
  if (m.target == MAP_SUCHO_HLAVNA && stage_ < 4) {
    sayScript(SC_ANDREA_NAJPRV_PARK);
    return false;
  }
  if (m.target == MAP_SUCHO_STANICA && stage_ < 5) {
    sayScript(stage_ < 4 ? SC_ANDREA_NAJPRV_PARK : SC_ANDREA_FONTANA);
    return false;
  }
  return true;
}

void Game::ch2Zone(const MapMark& m) {
  if (m.tag == TAG_SUCHO2 && stage_ == 1) playEpisode(EP_FIRST + 1, 0, ep(1).nDialog, AFTER_EPISODE, true);
  else if (m.tag == TAG_SUCHO3 && stage_ == 2) playEpisode(EP_FIRST + 2, 0, ep(2).nDialog, AFTER_EPISODE, true);
  else if (m.tag == TAG_SUCHO5 && stage_ == 4) playEpisode(EP_FIRST + 4, 0, ep(4).nDialog, AFTER_EPISODE, true);
  else if (m.tag == TAG_VLAK && stage_ == 5) {
    if (flag(F2_TICKETS)) sayScript(SC_SPRIEVODCA, AFTER_EVENT, EV2_BOARD);
    else sayScript(SC_ANDREA_NAJPRV_LISTKY);
  }
}

void Game::ch2Talk(const Walker& w) {
  uint8_t tag = w.mark ? w.mark->tag : 0;
  if (w.who == SPK_ANDREA) {
    if (stage_ == 3 && flag(F2_GIVEN)) playEpisode(EP_FIRST + 3, 0, ep(3).nDialog, AFTER_EPISODE, true);
    else if (stage_ == 3) sayScript(SC_ANDREA_KVAPKY);
    else if (stage_ < 3) sayScript(SC_ANDREA_NAJPRV_PARK);
    else if (stage_ == 4) sayScript(SC_ANDREA_FONTANA);
    else if (stage_ == 5 && !flag(F2_TICKETS) && world_.mapIndex() == MAP_SUCHO_STANICA) sayScript(SC_ANDREA_NAJPRV_LISTKY);
    else sayLast(EP_FIRST + (stage_ > 0 ? stage_ - 1 : 0), SPK_ANDREA);
    return;
  }
  if (w.who == SPK_MOUSE || w.who == SPK_CHICK) {
    if (flag(F2_GIVEN)) sayScript(SC_ZVIERATA_POTOM);
    else if (stage_ >= 3 && itemCount(ITEM_KVAPKA) >= DROPS_NEEDED) sayScript(SC_ZVIERATA_DAKUJEME, AFTER_EVENT, EV2_DROPS);
    else if (itemCount(ITEM_KVAPKA) > 0) sayScript(SC_ZVIERATA_MALO);
    else sayScript(SC_ZVIERATA_SMAD);
    return;
  }
  if (tag == TAG_POKLADNA) {
    if (stage_ == 5 && !flag(F2_TICKETS)) playEpisode(EP_FIRST + 5, S6_DESK, S6_TRAIN, AFTER_EVENT, false, EV2_TICKETS);
    else sayLast(EP_FIRST + 5, SPK_HUMAN);
    return;
  }
  if (tag == TAG_ZAHRADNIK) {
    if (stage_ >= 5) sayLast(EP_FIRST + 4, SPK_HUMAN);  // « Vela stastia. »
    else audio_->play(SFX_BUMP);
    return;
  }
  passerby(w);
}

void Game::ch2Inspect(int, int) {}

void Game::ch2Event(int event) {
  switch (event) {
    case EV2_STATION:  // « Pockaj. Musim napisat tete listok. » : on l'aide
      sayScript(SC_ANDREA_TETA_LISTOK, AFTER_EVENT, EV2_NOTE);
      break;
    case EV2_NOTE:
      setFlag(F2_NOTE);
      say(ep(5).dialog + S6_NOTE, 1, -1, AFTER_NOTHING);  // « Andrea pise: ... »
      break;
    case EV2_DROPS:
      setFlag(F2_GIVEN);
      give(ITEM_KVAPKA, -DROPS_NEEDED);
      break;
    case EV2_TICKETS:
      setFlag(F2_TICKETS);
      give(ITEM_LISTKY);
      break;
    case EV2_BOARD:  // « Vlak ide na zapad. »
      say(ep(5).dialog + S6_TRAIN, 1, -1, AFTER_SLEEP);
      break;
  }
}

void Game::ch2EpisodeDone() {
  if (stage_ == 1) {
    // « Tu mas vodu. Len trochu. » ; ensuite Andrea suit le renard
    give(ITEM_VODA);
    world_.setFollower(SPK_ANDREA);
  } else if (stage_ == 4) {
    // « Mam batoh a mapu. » (Andrea part avec, le renard les porte)
    give(ITEM_MAPA, 1, false);
    give(ITEM_BATOH);
  }
}
