// Simulateur PC de Zajka Quest : meme code de jeu que sur l'Odroid Go, sans ecran.
// Rejoue un script d'entrees, ecrit des captures PPM et le son en WAV.
//
//   make -C sim                                        compile build/sim
//   build/sim --script sim/demo.txt --out /tmp/zq      rejoue un script (voir demo.txt)
//   build/sim --frames 600 --every 120 --out /tmp/zq   une capture toutes les 2 s
//   build/sim --course                                 resume du cours embarque, glyphes manquants
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "../src/game/course.h"
#include "../src/game/game.h"
#include "../src/game/gen/assets.h"

static uint16_t fb[SCREEN_W * SCREEN_H];

static void renderFrame(Game& g) {
  for (int y0 = 0; y0 < SCREEN_H; y0 += BAND_H) {
    Band b{fb + y0 * SCREEN_W, y0, BAND_H};
    g.renderBand(b);
  }
}

static void savePPM(const std::string& path) {
  FILE* f = fopen(path.c_str(), "wb");
  if (!f) return;
  fprintf(f, "P6\n%d %d\n255\n", SCREEN_W, SCREEN_H);
  for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
    uint16_t c = fb[i];
    uint8_t r = (c >> 11) << 3, g = ((c >> 5) & 63) << 2, b = (c & 31) << 3;
    r |= r >> 5; g |= g >> 6; b |= b >> 5;
    fputc(r, f); fputc(g, f); fputc(b, f);
  }
  fclose(f);
}

static void saveWav(const std::string& path, const std::vector<int16_t>& s) {
  FILE* f = fopen(path.c_str(), "wb");
  if (!f) return;
  uint32_t n = s.size() * 2, rate = AUDIO_RATE, br = AUDIO_RATE * 2;
  uint16_t one = 1, bits = 16, align = 2;
  uint32_t riff = 36 + n, fmt = 16;
  fwrite("RIFF", 1, 4, f); fwrite(&riff, 4, 1, f); fwrite("WAVEfmt ", 1, 8, f);
  fwrite(&fmt, 4, 1, f); fwrite(&one, 2, 1, f); fwrite(&one, 2, 1, f);
  fwrite(&rate, 4, 1, f); fwrite(&br, 4, 1, f); fwrite(&align, 2, 1, f); fwrite(&bits, 2, 1, f);
  fwrite("data", 1, 4, f); fwrite(&n, 4, 1, f); fwrite(s.data(), 2, s.size(), f);
  fclose(f);
}

struct Seg {
  int from, to;
  uint16_t btn;
};

// Lettres des boutons : L R U D (croix), A, B, S (START), E (SELECT), V (VOLUME).
static uint16_t parseButtons(const char* s) {
  uint16_t b = 0;
  if (strchr(s, 'L')) b |= BTN_LEFT;
  if (strchr(s, 'R')) b |= BTN_RIGHT;
  if (strchr(s, 'U')) b |= BTN_UP;
  if (strchr(s, 'D')) b |= BTN_DOWN;
  if (strchr(s, 'A')) b |= BTN_A;
  if (strchr(s, 'B')) b |= BTN_B;
  if (strchr(s, 'S')) b |= BTN_START;
  if (strchr(s, 'E')) b |= BTN_SELECT;
  if (strchr(s, 'V')) b |= BTN_VOLUME;
  return b;
}

// --course : resume du cours embarque et verification que la police sait tout dessiner.
static int missingGlyphs(const char* s, const char* where) {
  int bad = 0;
  for (const unsigned char* p = (const unsigned char*)s; s && *p;) {
    uint32_t cp = *p;
    int n = cp < 0x80 ? 1 : cp < 0xE0 ? 2 : cp < 0xF0 ? 3 : 4;
    if (n > 1) cp &= (0xFF >> (n + 1));
    for (int i = 1; i < n; i++) cp = (cp << 6) | (p[i] & 0x3F);
    p += n;
    if (cp == (uint32_t)NAME_MARK) continue;
    bool found = false;
    for (int i = 0; i < font_small.count && !found; i++) found = font_small.cp[i] == cp;
    if (!found) {
      printf("  glyphe absent U+%04X dans %s : %s\n", cp, where, s);
      bad++;
    }
  }
  return bad;
}

static int checkCourse() {
  int bad = 0, lines = 0;
  for (int e = 0; e < COURSE_NUM_EPISODES; e++) {
    const Episode& ep = *COURSE_EPISODES[e];
    printf("cours : %s, série %d « %s », épisode %d « %s » : %d répliques (+%d), %d mots, %d exercices\n",
           ep.id, ep.seriesNum, ep.series, ep.episodeNum, ep.title, ep.nDialog, ep.nExtra, ep.nWords,
           ep.nExercises);
    for (int pass = 0; pass < 2; pass++) {
      const Line* L = pass ? ep.extra : ep.dialog;
      int n = pass ? ep.nExtra : ep.nDialog;
      for (int i = 0; i < n; i++, lines++) {
        bad += missingGlyphs(L[i].sk, ep.id) + missingGlyphs(L[i].fr, ep.id);
        if (L[i].help) bad += missingGlyphs(L[i].help, ep.id);
        for (int g = 0; g < L[i].nGloss; g++)
          bad += missingGlyphs(L[i].gloss[g].sk, ep.id) + missingGlyphs(L[i].gloss[g].fr, ep.id);
      }
    }
    for (int i = 0; i < ep.nWords; i++) bad += missingGlyphs(ep.words[i].sk, ep.id) + missingGlyphs(ep.words[i].fr, ep.id);
  }
  printf("cours : %d épisode(s), %d répliques, nom par défaut « %s », %d glyphe(s) manquant(s)\n",
         COURSE_NUM_EPISODES, lines, COURSE_DEFAULT_NAME, bad);
  return bad == 0 && COURSE_NUM_EPISODES > 0 ? 0 : 1;
}

static const char* stateName(GameState s) {
  switch (s) {
    case S_TITLE: return "titre";
    case S_CHAPTER: return "chapitre";
  }
  return "?";
}

int main(int argc, char** argv) {
  std::string out = "/tmp/zajka-quest", script;
  std::vector<int> shots;
  std::vector<std::string> expect;  // etats attendus, dans l'ordre (--expect titre,chapitre)
  int frames = 0, every = 0;
  bool wav = false;
  for (int i = 1; i < argc; i++) {
    std::string a = argv[i];
    if (a == "--out") out = argv[++i];
    else if (a == "--script") script = argv[++i];
    else if (a == "--frames") frames = atoi(argv[++i]);
    else if (a == "--every") every = atoi(argv[++i]);
    else if (a == "--wav") wav = true;
    else if (a == "--shots") {
      char* s = argv[++i];
      for (char* t = strtok(s, ","); t; t = strtok(nullptr, ",")) shots.push_back(atoi(t));
    } else if (a == "--course") {
      return checkCourse();
    } else if (a == "--expect") {
      char* s = argv[++i];
      for (char* t = strtok(s, ","); t; t = strtok(nullptr, ",")) expect.push_back(t);
    }
  }
  std::vector<Seg> segs;
  if (!script.empty()) {
    FILE* f = fopen(script.c_str(), "r");
    if (!f) {
      fprintf(stderr, "script introuvable : %s\n", script.c_str());
      return 2;
    }
    char line[256];
    while (fgets(line, sizeof line, f)) {
      int a, b;
      char btn[32];
      if (line[0] == '#') continue;
      if (sscanf(line, "shot %d", &a) == 1) shots.push_back(a);
      else if (sscanf(line, "%d %d %31s", &a, &b, btn) == 3) segs.push_back({a, b, parseButtons(btn)});
    }
    fclose(f);
  }
  if (frames == 0) {
    for (int s : shots) frames = frames > s ? frames : s;
    for (auto& s : segs) frames = frames > s.to ? frames : s.to;
    if (frames == 0) frames = 600;
  }
  static Audio audio;
  static Game game;
  game.begin(&audio);
  Input in;
  std::vector<int16_t> pcm;
  int16_t abuf[AUDIO_RATE / 60 + 1];
  GameState last = game.state();
  std::vector<std::string> seen{stateName(last)};
  for (int f = 1; f <= frames; f++) {
    uint16_t raw = 0;
    for (auto& s : segs)
      if (f >= s.from && f <= s.to) raw |= s.btn;
    in.update(raw);
    game.step(in);
    if (wav) {
      audio.render(abuf, AUDIO_RATE / 60);
      pcm.insert(pcm.end(), abuf, abuf + AUDIO_RATE / 60);
    }
    if (game.state() != last) {
      printf("image %5d : %s -> %s\n", f, stateName(last), stateName(game.state()));
      last = game.state();
      seen.push_back(stateName(last));
    }
    bool shot = (every && f % every == 0);
    for (int s : shots) shot |= (s == f);
    if (shot) {
      renderFrame(game);
      char name[64];
      snprintf(name, sizeof name, "/frame_%05d.ppm", f);
      savePPM(out + name);
    }
  }
  if (wav) saveWav(out + "/audio.wav", pcm);
  printf("%d images, etat final : %s\n", frames, stateName(game.state()));
  if (!expect.empty()) {
    // les etats attendus doivent apparaitre dans cet ordre (d'autres peuvent s'intercaler)
    size_t k = 0;
    for (auto& s : seen)
      if (k < expect.size() && s == expect[k]) k++;
    if (k < expect.size()) {
      printf("ECHEC : etat « %s » jamais atteint dans l'ordre attendu\n", expect[k].c_str());
      return 1;
    }
    printf("parcours attendu : ok\n");
  }
  return 0;
}
