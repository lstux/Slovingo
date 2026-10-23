// Histoire du chapitre 1 : Kosice (episodes 1 a 6 de la serie Kosice du cours).
//
//   rue Hlavna      on monte dans le tram a l'arret ; deux passants disent des « repliques en plus »
//   tram            en entrant, le Narrateur plante le decor (debut de l'episode 1) ; chaque fois
//                   qu'on parle a Andrea, l'episode suivant : 1 « To si ty? » ... 5 « V elektricke ».
//                   A la fin du 5 (« To je moja zastavka. Pod! »), on descend avec elle.
//   chez la tante   le soir, devant la maison : le Narrateur, puis l'episode 6 « Dobru noc » avec
//                   Andrea ; sa derniere replique (la nuit sous le pommier) se joue quand on va
//                   au pommier. Fin du chapitre.
//
// stage_ = nombre d'episodes joues en entier ; flags_ = SaveFlag. Tout est sauvegarde.
#include <stdio.h>

#include "game.h"
#include "gen/assets.h"

namespace {
constexpr int EP_TRAM_LAST = 4;  // index des episodes du tram : 0 a 4
constexpr int EP_TETA = 5;       // l'episode 6, chez la tante
constexpr int BANNER_TIME = 150;

const Episode& episode(int i) { return *COURSE_EPISODES[i < COURSE_NUM_EPISODES ? i : COURSE_NUM_EPISODES - 1]; }

// repliques du Narrateur au debut d'un episode
int leadingNarrator(const Episode& ep) {
  int n = 0;
  while (n < ep.nDialog && ep.dialog[n].speaker == SPK_NARRATOR) n++;
  return n;
}
// ... et a la fin (episode 6 : la nuit sous le pommier)
int trailingNarrator(const Episode& ep) {
  int n = 0;
  while (n < ep.nDialog && ep.dialog[ep.nDialog - 1 - n].speaker == SPK_NARRATOR) n++;
  return n;
}
}  // namespace

int Game::bonusFound() const {
  int n = 0;
  for (int b : bonus_) n += b;
  return n;
}

int Game::bonusTotal() const {
  int n = 0;
  for (int i = 0; i < COURSE_NUM_EPISODES && i < SAVE_EPISODES; i++) n += COURSE_EPISODES[i]->nBonus;
  return n;
}

// Une conversation ; bonusEpisode >= 0 : ses choix peuvent gagner les repliques secretes de cet episode.
void Game::say(const Line* lines, int n, int bonusEpisode, After after) {
  if (n <= 0) {
    after_ = after;
    storyDialogDone();
    return;
  }
  if (bonusEpisode >= 0 && bonusEpisode < SAVE_EPISODES) {
    const Episode& ep = episode(bonusEpisode);
    dialog_.start(lines, n, COURSE_DEFAULT_NAME, ep.bonus, ep.nBonus, &bonus_[bonusEpisode]);
  } else {
    dialog_.start(lines, n, COURSE_DEFAULT_NAME);
  }
  after_ = after;
  setState(S_DIALOG);
}

// Repliques [from, to) du dialogue de l'episode ep ; banner : affiche son titre.
void Game::playEpisode(int ep, int from, int to, After after, bool banner) {
  if (banner) {
    bannerEp_ = ep;
    banner_ = BANNER_TIME;
  }
  say(episode(ep).dialog + from, to - from, ep, after);
}

// Andrea redit sa derniere replique de l'episode qu'on vient de jouer avec elle.
void Game::andreaRepeats() {
  const Episode& ep = episode(stage_ > 0 ? stage_ - 1 : 0);
  for (int i = ep.nDialog - 1; i >= 0; i--)
    if (ep.dialog[i].speaker == SPK_ANDREA) {
      say(&ep.dialog[i], 1, -1, AFTER_NOTHING);
      return;
    }
}

// Passage impose par l'histoire : le meme rideau qu'une porte. tx < 0 : au depart de la carte.
void Game::goTo(int map, int tx, int ty, Dir dir) {
  if (tx < 0) {
    const MapDef& t = *ALL_MAPS[map];
    tx = t.w / 2;
    ty = t.h / 2;
    for (int i = 0; i < t.nMarks; i++)
      if (t.marks[i].type == M_START) {
        tx = t.marks[i].x;
        ty = t.marks[i].y;
        dir = t.marks[i].dir;
      }
  }
  autoWarp_ = MapMark{};
  autoWarp_.type = M_WARP;
  autoWarp_.target = (int8_t)map;
  autoWarp_.tx = (uint8_t)tx;
  autoWarp_.ty = (uint8_t)ty;
  autoWarp_.dir = dir;
  warp_ = &autoWarp_;
  audio_->play(SFX_CHECK);
  setState(S_FADE);
}

void Game::storyEnteredMap() {
  int m = world_.mapIndex();
  if (m == MAP_KOSICE_TRAM) {
    if (stage_ > EP_TRAM_LAST) {
      // episode 5 fini (partie reprise dans le tram) : on descend chez la tante
      goTo(MAP_KOSICE_TETA, -1, -1, DIR_UP);
      return;
    }
    if (!(flags_ & SF_TRAM_INTRO)) {
      flags_ |= SF_TRAM_INTRO;
      playEpisode(0, 0, leadingNarrator(episode(0)), AFTER_NOTHING, true);
    }
  } else if (m == MAP_KOSICE_TETA && !(flags_ & SF_TETA_INTRO)) {
    flags_ |= SF_TETA_INTRO;
    playEpisode(EP_TETA, 0, leadingNarrator(episode(EP_TETA)), AFTER_NOTHING, true);
  }
}

void Game::storyTalk(int npc) {
  const Walker& w = world_.npc(npc);
  int m = world_.mapIndex();
  if (w.who == SPK_ANDREA) {
    if (m == MAP_KOSICE_TRAM && stage_ <= EP_TRAM_LAST) {
      const Episode& ep = episode(stage_);
      // episode 1 : le debut du Narrateur a deja ete joue en entrant
      int from = (stage_ == 0 && (flags_ & SF_TRAM_INTRO)) ? leadingNarrator(ep) : 0;
      playEpisode(stage_, from, ep.nDialog, AFTER_EPISODE, stage_ > 0);
    } else if (m == MAP_KOSICE_TETA && stage_ == EP_TETA) {
      const Episode& ep = episode(EP_TETA);
      playEpisode(EP_TETA, leadingNarrator(ep), ep.nDialog - trailingNarrator(ep), AFTER_EPISODE, false);
    } else {
      andreaRepeats();
    }
    return;
  }
  // passant ou voyageur : ses « repliques en plus »
  const MapMark* mk = w.mark;
  if (mk && mk->sayCount && mk->sayEp >= 1 && mk->sayEp <= COURSE_NUM_EPISODES) {
    const Episode& ep = *COURSE_EPISODES[mk->sayEp - 1];
    int from = mk->sayFrom, n = mk->sayCount;
    if (from + n > ep.nExtra) n = ep.nExtra - from;
    if (n > 0) {
      say(ep.extra + from, n, -1, AFTER_NOTHING);
      return;
    }
  }
  audio_->play(SFX_BUMP);
}

void Game::storyInspect(int tx, int ty) {
  if (world_.mapIndex() == MAP_KOSICE_TETA && world_.tileAt(tx, ty) == TL_POMMIER && stage_ > EP_TETA) {
    // la nuit sous le pommier : la derniere replique du Narrateur, puis la fin du chapitre
    const Episode& ep = episode(EP_TETA);
    int n = trailingNarrator(ep);
    say(ep.dialog + ep.nDialog - n, n, -1, AFTER_SLEEP);
  }
}

void Game::storyDialogDone() {
  After a = after_;
  after_ = AFTER_NOTHING;
  if (a == AFTER_EPISODE) {
    stage_++;
    if (stage_ == EP_TRAM_LAST + 1 && world_.mapIndex() == MAP_KOSICE_TRAM) {
      // « To je moja zastavka. Pod! » : on descend avec Andrea, le soir, chez sa tante
      goTo(MAP_KOSICE_TETA, -1, -1, DIR_UP);
    }
  } else if (a == AFTER_SLEEP) {
    flags_ |= SF_SLEPT;
    audio_->play(SFX_WIN);
    setState(S_END);
  }
}

// ---------------------------------------------------------------- rendu
void Game::drawBanner(Band& b) {
  if (b.y0 > 34) return;
  const Episode& ep = episode(bannerEp_);
  char s[96];
  snprintf(s, sizeof s, "%d · %s", ep.episodeNum, ep.title);
  int w = textWidth(font_big, s) + 24;
  int x = (SCREEN_W - w) / 2, y = 8;
  b.fillRect(x, y, w, 24, 0x1926);
  b.fillRect(x, y + 22, w, 2, 0xFB00);
  b.text(font_big, x + 12, y + 2, s, 0xFFFF, 0x0000);
}

void Game::drawEnd(Band& b) {
  b.fill(0x0841);
  b.fillRect(0, 50, SCREEN_W, 2, 0xFB00);
  b.fillRect(0, 188, SCREEN_W, 2, 0xFB00);
  centerText(b, font_small, 62, "KOŠICE", 0xFF2A, 0x0000);
  centerText(b, font_title, 78, "Koniec 1. kapitoly", 0xFFFF, 0x0000);
  char s[64];
  snprintf(s, sizeof s, "Tajné repliky : %d / %d", bonusFound(), bonusTotal());
  centerText(b, font_small, 124, s, 0xBDF7, -1);
  centerText(b, font_small, 146, "Ďalšia kapitola : Sucho", 0xBDF7, -1);
  if (stateTime_ > 120 && (frame_ / 30) % 2 == 0) centerText(b, font_small, 166, "Stlač A", 0xFFFF, -1);
}
