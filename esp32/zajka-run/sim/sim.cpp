// Simulateur PC de Zajka Run : meme code de jeu que sur l'Odroid Go, sans ecran.
// Rejoue des entrees (script ou robot), ecrit des captures PPM et le son en WAV.
//
//   make -C sim                       compile build/sim
//   build/sim --bot --shots 300,900 --out /tmp/zr      joue tout seul, captures aux images 300 et 900
//   build/sim --script sim/demo.txt --out /tmp/zr      rejoue un script (voir demo.txt)
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "../src/game/game.h"

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

// Sauvegarde dans un fichier (--save fichier) : meme format que la NVS de la console.
class FileStorage : public Storage {
 public:
  explicit FileStorage(const std::string& p) : path_(p) {}
  bool load(SaveData& d) override {
    FILE* f = fopen(path_.c_str(), "rb");
    if (!f) return false;
    bool ok = fread(&d, sizeof d, 1, f) == 1;
    fclose(f);
    return ok;
  }
  void save(const SaveData& d) override {
    FILE* f = fopen(path_.c_str(), "wb");
    if (!f) return;
    fwrite(&d, sizeof d, 1, f);
    fclose(f);
  }

 private:
  std::string path_;
};

struct Seg {
  int from, to;
  uint16_t btn;
};

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
  return b;
}

// Robot tres simple : court a droite, saute devant un obstacle, un trou ou un ennemi.
static uint16_t bot(const Game& g, int& jumpHold) {
  uint16_t b = BTN_RIGHT;
  if (g.state() == S_TITLE || g.state() == S_INTRO || g.state() == S_CLEAR || g.state() == S_END ||
      g.state() == S_GAMEOVER)
    return (g.frameCount() % 20) < 2 ? (BTN_START | BTN_A) : 0;
  const Player& p = g.player();
  const Level& L = g.level();
  int fx = (int)(p.x + Game::PW + 6 + p.vx * 6) >> 4;
  int feet = (int)(p.y + Game::PH - 1) >> 4;
  bool wall = L.kindAt(fx, feet) == K_SOLID || L.kindAt(fx, feet - 1) == K_SOLID;
  bool hole = true;  // pas de sol dans les 4 cases sous le bord avant
  for (int dy = 1; dy <= 4 && hole; dy++)
    for (int dx = 0; dx <= 1; dx++)
      if (L.kindAt(fx + dx, feet + dy) != K_AIR) hole = false;
  bool spike = L.kindAt(fx, feet) == K_HAZARD || L.kindAt(fx + 1, feet) == K_HAZARD;
  int n;
  const Entity* e = g.entities(n);
  bool enemy = false, tram = false;
  for (int i = 0; i < n; i++) {
    if ((e[i].type == E_PIGEON || e[i].type == E_WASP) && e[i].state == 0) {
      float dx = e[i].x - (p.x + Game::PW);
      if (dx > 0 && dx < 34 && fabsf(e[i].y - p.y) < 60) enemy = true;
    }
    if (e[i].type == E_TRAM) {
      float dx = e[i].x - (p.x + Game::PW);
      if (dx > -10 && dx < 40 && p.riding < 0) tram = true;
    }
  }
  if (hole || getenv("ZR_RUN")) b |= BTN_B;
  if (p.onGround && (hole || spike || tram)) jumpHold = 22;
  else if (p.onGround && (wall || enemy)) jumpHold = 12;
  if (jumpHold > 0) {
    b |= BTN_A;
    jumpHold--;
  }
  return b;
}

int main(int argc, char** argv) {
  std::string out = "/tmp/zajka", script;
  std::vector<int> shots;
  int frames = 0, every = 0;
  bool useBot = false, wav = false;
  float startX = -1, startY = -1;
  int level = 0;
  bool rabbit = false;
  std::string savePath;
  for (int i = 1; i < argc; i++) {
    std::string a = argv[i];
    if (a == "--out") out = argv[++i];
    else if (a == "--script") script = argv[++i];
    else if (a == "--bot") useBot = true;
    else if (a == "--frames") frames = atoi(argv[++i]);
    else if (a == "--every") every = atoi(argv[++i]);
    else if (a == "--wav") wav = true;
    else if (a == "--start-x") startX = atof(argv[++i]);
    else if (a == "--start-y") startY = atof(argv[++i]);
    else if (a == "--level") level = atoi(argv[++i]) - 1;
    else if (a == "--andrea") rabbit = true;
    else if (a == "--save") savePath = argv[++i];
    else if (a == "--shots") {
      char* s = argv[++i];
      for (char* t = strtok(s, ","); t; t = strtok(nullptr, ",")) shots.push_back(atoi(t));
    }
  }
  std::vector<Seg> segs;
  if (!script.empty()) {
    FILE* f = fopen(script.c_str(), "r");
    char line[256];
    while (f && fgets(line, sizeof line, f)) {
      int a, b;
      char btn[32];
      if (line[0] == '#') continue;
      if (sscanf(line, "%d %d %31s", &a, &b, btn) == 3) segs.push_back({a, b, parseButtons(btn)});
      else if (sscanf(line, "shot %d", &a) == 1) shots.push_back(a);
    }
    if (f) fclose(f);
  }
  if (frames == 0) {
    for (int s : shots) frames = frames > s ? frames : s;
    for (auto& s : segs) frames = frames > s.to ? frames : s.to;
    if (useBot && frames < 6000) frames = 6000;
  }
  static Audio audio;
  static Game game;
  static FileStorage* storage = savePath.empty() ? nullptr : new FileStorage(savePath);
  game.selectLevel(level);
  game.selectCharacter(rabbit);
  game.begin(&audio, storage);
  game.selectLevel(level);  // meme si la sauvegarde ne l'a pas encore debloque
  Input in;
  std::vector<int16_t> pcm;
  int16_t abuf[AUDIO_RATE / 60 + 1];
  int jumpHold = 0;
  GameState last = game.state();
  for (int f = 1; f <= frames; f++) {
    uint16_t raw = 0;
    for (auto& s : segs)
      if (f >= s.from && f <= s.to) raw |= s.btn;
    if (useBot) raw |= bot(game, jumpHold);
    in.update(raw);
    game.step(in);
    if (startX >= 0 && game.state() == S_INTRO) {
      game.debugTeleport(startX, startY);
      startX = -1;
    }
    if (wav) {
      audio.render(abuf, AUDIO_RATE / 60);
      pcm.insert(pcm.end(), abuf, abuf + AUDIO_RATE / 60);
    }
    if (getenv("ZR_TRACE")) {
      const Player& p = game.player();
      printf("%d x=%.1f y=%.1f vx=%.2f vy=%.2f g=%d raw=%x\n", f, p.x, p.y, p.vx, p.vy, p.onGround, raw);
    }
    static int lastHp = 3;
    if (game.player().hp != lastHp) {
      if (game.player().hp < lastHp)
        printf("image %5d : touche ! pv=%d x=%.0f y=%.0f\n", f, game.player().hp, game.player().x, game.player().y);
      lastHp = game.player().hp;
    }
    if (game.state() != last) {
      printf("image %5d : etat %d -> %d  (x=%.0f, medovniky=%d, vies=%d)\n", f, last, game.state(),
             game.player().x, game.hearts(), game.lives());
      last = game.state();
    }
    bool shot = (every && f % every == 0);
    for (int s : shots) shot |= (s == f);
    if (shot) {
      renderFrame(game);
      char name[64];
      snprintf(name, sizeof name, "/frame_%05d.ppm", f);
      savePPM(out + name);
    }
    if (useBot && (game.state() == S_END || game.state() == S_CLEAR)) {
      printf("fin atteinte a l'image %d\n", f);
      break;
    }
  }
  if (wav) saveWav(out + "/audio.wav", pcm);
  printf("x=%.1f y=%.1f etat=%d medovniky=%d vies=%d\n", game.player().x, game.player().y, game.state(),
         game.hearts(), game.lives());
  return 0;
}
