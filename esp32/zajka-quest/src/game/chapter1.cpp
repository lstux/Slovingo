// Chapitre 1 : Kosice (episodes 1 a 6 de la serie Kosice du cours, COURSE_EPISODES[0..5]).
//
//   rue Hlavna      on achete un ticket a la trafika (choix de replique), le conducteur ne laisse
//                   monter qu'avec un ticket ; des panneaux a lire (Slovnik), deux passants.
//   tram            le Narrateur plante le decor ; Andrea : episodes 1 et 2 ; puis un revizor monte :
//                   il faut avoir composte son ticket (le composteur jaune, pres de la porte) ;
//                   ensuite les episodes 3, 4, 5. A la fin du 5, on descend avec Andrea.
//   chez la tante   le soir : le Narrateur, l'episode 6, puis dormir sous le pommier. Fin.
//
// stage_ = episodes joues en entier (0 a 6) ; flags_ = F1_...
#include "game.h"
#include "gen/assets.h"
#include "gen/scripts.h"

namespace {
enum : uint16_t {
  F1_TRAM_INTRO = 1 << 0,  // le Narrateur a plante le decor du tram
  F1_TETA_INTRO = 1 << 1,  // ... et celui du jardin de la tante
  F1_SLEPT = 1 << 2,       // la nuit sous le pommier
  F1_TICKET = 1 << 3,      // ticket achete
  F1_DRIVER = 1 << 4,      // le conducteur nous a laisses monter
  F1_STAMPED = 1 << 5,     // ticket composte
  F1_CONTROL = 1 << 6,     // le revizor est monte
  F1_CONTROL_OK = 1 << 7,  // ... et il a vu le ticket composte
};
enum { EV1_TICKET = 1, EV1_DRIVER, EV1_STAMP, EV1_CONTROL_BOARD, EV1_CONTROL_OK };

constexpr int EP_FIRST = 0;   // index global du premier episode du chapitre
constexpr int EP_CONTROL = 2; // le revizor monte apres l'episode 2
constexpr int EP_TRAM_END = 5;
constexpr int EP_TETA = 5;    // l'episode 6 (index 5), chez la tante

const Episode& ep(int i) { return *COURSE_EPISODES[EP_FIRST + i]; }

int leadingNarrator(const Episode& e) {
  int n = 0;
  while (n < e.nDialog && e.dialog[n].speaker == SPK_NARRATOR) n++;
  return n;
}
int trailingNarrator(const Episode& e) {
  int n = 0;
  while (n < e.nDialog && e.dialog[e.nDialog - 1 - n].speaker == SPK_NARRATOR) n++;
  return n;
}
}  // namespace

int Game::ch1Goal() const {
  int m = world_.mapIndex();
  if (flag(F1_SLEPT)) return SC_CIEL_HOTOVO;
  if (m == MAP_KOSICE_TETA) return stage_ > EP_TETA ? SC_CIEL_JABLON : SC_CIEL_ANDREA;
  if (!flag(F1_TICKET)) return SC_CIEL_LISTOK;
  if (m == MAP_KOSICE_HLAVNA) return SC_CIEL_NASTUP;
  if (stage_ >= EP_CONTROL && !flag(F1_CONTROL_OK)) return SC_CIEL_REVIZOR;
  return SC_CIEL_ANDREA;
}

void Game::ch1Refresh() {
  int m = world_.mapIndex();
  if (m == MAP_KOSICE_HLAVNA) world_.setNpcHidden(TAG_VODIC, flag(F1_DRIVER));
  if (m == MAP_KOSICE_TRAM) world_.setNpcHidden(TAG_REVIZOR, !flag(F1_CONTROL));
}

void Game::ch1EnteredMap() {
  int m = world_.mapIndex();
  if (m == MAP_KOSICE_TRAM) {
    if (stage_ >= EP_TRAM_END) {  // episode 5 fini (partie reprise dans le tram) : chez la tante
      goTo(MAP_KOSICE_TETA, -1, -1, DIR_UP);
      return;
    }
    if (!flag(F1_TRAM_INTRO)) {
      setFlag(F1_TRAM_INTRO);
      playEpisode(EP_FIRST, 0, leadingNarrator(ep(0)), AFTER_NOTHING, true);
    }
  } else if (m == MAP_KOSICE_TETA && !flag(F1_TETA_INTRO)) {
    setFlag(F1_TETA_INTRO);
    playEpisode(EP_FIRST + EP_TETA, 0, leadingNarrator(ep(EP_TETA)), AFTER_NOTHING, true);
  }
}

void Game::ch1Talk(const Walker& w) {
  int m = world_.mapIndex();
  if (w.who == SPK_ANDREA) {
    if (m == MAP_KOSICE_TRAM && stage_ < EP_TRAM_END) {
      if (stage_ >= EP_CONTROL && !flag(F1_CONTROL_OK)) {
        sayScript(SC_ANDREA_POZOR);
        return;
      }
      const Episode& e = ep(stage_);
      // episode 1 : le debut du Narrateur a deja ete joue en entrant
      int from = (stage_ == 0 && flag(F1_TRAM_INTRO)) ? leadingNarrator(e) : 0;
      playEpisode(EP_FIRST + stage_, from, e.nDialog, AFTER_EPISODE, stage_ > 0);
    } else if (m == MAP_KOSICE_TETA && stage_ == EP_TETA) {
      const Episode& e = ep(EP_TETA);
      playEpisode(EP_FIRST + EP_TETA, leadingNarrator(e), e.nDialog - trailingNarrator(e), AFTER_EPISODE, false);
    } else {
      sayLast(EP_FIRST + (stage_ > 0 ? stage_ - 1 : 0), SPK_ANDREA);
    }
    return;
  }
  uint8_t tag = w.mark ? w.mark->tag : 0;
  if (tag == TAG_PREDAVAC) {
    if (!flag(F1_TICKET)) sayScript(SC_TRAFIKA, AFTER_EVENT, EV1_TICKET);
    else sayScript(SC_TRAFIKA_POTOM);
  } else if (tag == TAG_VODIC) {
    if (itemCount(ITEM_LISTOK)) sayScript(SC_VODIC_S_LISTKOM, AFTER_EVENT, EV1_DRIVER);
    else sayScript(SC_VODIC_BEZ_LISTKA);
  } else if (tag == TAG_REVIZOR) {
    if (flag(F1_CONTROL_OK)) sayScript(SC_REVIZOR_POTOM);
    else if (flag(F1_STAMPED)) sayScript(SC_REVIZOR_OK, AFTER_EVENT, EV1_CONTROL_OK);
    else sayScript(SC_REVIZOR_NEOZNACENY);
  } else {
    passerby(w);
  }
}

void Game::ch1Inspect(int tx, int ty) {
  int m = world_.mapIndex(), t = world_.tileAt(tx, ty);
  if (m == MAP_KOSICE_TRAM && t == TL_COMPOSTEUR) {
    if (flag(F1_STAMPED)) sayScript(SC_OZNACOVAC_ZNOVA);
    else if (itemCount(ITEM_LISTOK)) sayScript(SC_OZNACOVAC, AFTER_EVENT, EV1_STAMP);
    return;
  }
  if (m == MAP_KOSICE_TETA && t == TL_POMMIER && stage_ > EP_TETA) {
    // la nuit sous le pommier : la derniere replique du Narrateur, puis la fin du chapitre
    const Episode& e = ep(EP_TETA);
    int n = trailingNarrator(e);
    setFlag(F1_SLEPT);
    say(e.dialog + e.nDialog - n, n, -1, AFTER_SLEEP);
  }
}

void Game::ch1Zone(const MapMark&) {}
bool Game::ch1Warp(const MapMark&) { return true; }

void Game::ch1Event(int event) {
  switch (event) {
    case EV1_TICKET:
      setFlag(F1_TICKET);
      give(ITEM_LISTOK);
      break;
    case EV1_DRIVER:
      setFlag(F1_DRIVER);  // il s'ecarte (ch1Refresh le cache)
      break;
    case EV1_STAMP:
      setFlag(F1_STAMPED);
      break;
    case EV1_CONTROL_BOARD:
      setFlag(F1_CONTROL);
      break;
    case EV1_CONTROL_OK:
      setFlag(F1_CONTROL_OK);
      break;
  }
}

void Game::ch1EpisodeDone() {
  if (world_.mapIndex() != MAP_KOSICE_TRAM) return;
  if (stage_ == EP_CONTROL && !flag(F1_CONTROL)) {
    // le tram s'arrete, un revizor monte
    sayScript(SC_REVIZOR_NASTUPUJE, AFTER_EVENT, EV1_CONTROL_BOARD);
  } else if (stage_ == EP_TRAM_END) {
    // « To je moja zastavka. Pod! » : on descend avec Andrea, le soir, chez sa tante
    goTo(MAP_KOSICE_TETA, -1, -1, DIR_UP);
  }
}
