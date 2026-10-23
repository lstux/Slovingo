#include "audio.h"

#include <math.h>

// ---------------------------------------------------------------- effets sonores
struct Step {
  uint8_t wave;
  uint16_t f0, f1;    // Hz (0 = silence)
  uint16_t ms;
  uint8_t v0, v1;     // volume 0..15
};

static const Step S_JUMP[] = {{1, 300, 760, 110, 9, 3}, {0, 0, 0, 0, 0, 0}};
static const Step S_COIN[] = {{2, 988, 988, 55, 8, 8}, {2, 1319, 1319, 200, 8, 0}, {0, 0, 0, 0, 0, 0}};
static const Step S_STOMP[] = {{4, 1800, 400, 50, 11, 0}, {1, 420, 160, 90, 9, 0}, {0, 0, 0, 0, 0, 0}};
static const Step S_HURT[] = {{0, 520, 110, 280, 11, 0}, {0, 0, 0, 0, 0, 0}};
static const Step S_SPRING[] = {{1, 180, 1250, 240, 10, 2}, {0, 0, 0, 0, 0, 0}};
static const Step S_BUMP[] = {{2, 170, 110, 80, 11, 0}, {0, 0, 0, 0, 0, 0}};
static const Step S_LIFE[] = {{2, 659, 659, 70, 8, 8}, {2, 784, 784, 70, 8, 8}, {2, 1319, 1319, 70, 8, 8},
                              {2, 1047, 1047, 70, 8, 8}, {2, 1175, 1175, 70, 8, 8}, {2, 1568, 1568, 160, 8, 0},
                              {0, 0, 0, 0, 0, 0}};
static const Step S_DIE[] = {{2, 0, 0, 120, 0, 0}, {2, 740, 90, 800, 10, 0}, {0, 0, 0, 0, 0, 0}};
static const Step S_WIN[] = {{1, 587, 587, 110, 9, 9}, {1, 740, 740, 110, 9, 9}, {1, 880, 880, 110, 9, 9},
                             {1, 1175, 1175, 220, 9, 9}, {1, 880, 880, 110, 9, 9}, {1, 1175, 1175, 500, 10, 0},
                             {0, 0, 0, 0, 0, 0}};
static const Step S_CHECK[] = {{2, 880, 880, 80, 8, 8}, {2, 1175, 1175, 200, 8, 0}, {0, 0, 0, 0, 0, 0}};
static const Step S_PAUSE[] = {{2, 1047, 1047, 60, 7, 7}, {2, 0, 0, 40, 0, 0}, {2, 1568, 1568, 90, 7, 0},
                               {0, 0, 0, 0, 0, 0}};
// la cloche de l'electricka : « cink cink »
static const Step S_BELL[] = {{2, 2093, 2093, 90, 7, 0}, {2, 0, 0, 50, 0, 0}, {2, 2093, 2093, 160, 7, 0},
                              {0, 0, 0, 0, 0, 0}};
static const Step S_GAMEOVER[] = {{1, 523, 523, 220, 9, 7}, {1, 392, 392, 220, 9, 7}, {1, 330, 330, 220, 9, 7},
                                  {1, 262, 247, 700, 9, 0}, {0, 0, 0, 0, 0, 0}};

static const Step* const SFX_TABLE[SFX_COUNT] = {S_JUMP, S_COIN,  S_STOMP, S_HURT,  S_SPRING, S_BUMP,    S_LIFE,
                                                 S_DIE,  S_WIN,   S_CHECK, S_PAUSE, S_BELL,   S_GAMEOVER};

// ---------------------------------------------------------------- musique
// « Hlavná » : air original, re majeur avec le 4e degre hausse (couleur lydienne, frequente
// dans la musique populaire slovaque), rythme de polka a 2/4. Une mesure = 8 doubles croches.
struct Note {
  uint8_t midi, len;  // midi 0 = silence ; len en doubles croches
};
static const Note MELODY[] = {
    // A
    {69, 2}, {74, 2}, {78, 2}, {81, 2},            // D
    {80, 1}, {81, 1}, {83, 2}, {81, 2}, {78, 2},   // D
    {76, 2}, {73, 2}, {76, 2}, {81, 2},            // A
    {78, 4}, {74, 4},                              // D
    {83, 2}, {79, 2}, {83, 2}, {86, 2},            // G
    {81, 2}, {78, 2}, {81, 2}, {86, 2},            // D
    {85, 1}, {83, 1}, {81, 2}, {79, 2}, {76, 2},   // A7
    {74, 4}, {0, 2}, {69, 2},                      // D
    // B
    {78, 2}, {83, 2}, {86, 2}, {83, 2},            // Bm
    {85, 2}, {81, 2}, {78, 2}, {81, 2},            // F#m
    {83, 1}, {81, 1}, {79, 2}, {83, 2}, {86, 2},   // G
    {81, 4}, {78, 4},                              // D
    {79, 2}, {83, 2}, {76, 2}, {79, 2},            // Em
    {81, 2}, {80, 2}, {76, 2}, {73, 2},            // A
    {74, 2}, {78, 2}, {81, 2}, {78, 2},            // D
    {74, 4}, {0, 4},                               // D
};
struct Chord {
  uint8_t root, fifth, pah;
};
static const Chord CH_D = {50, 57, 66}, CH_A = {45, 52, 61}, CH_A7 = {45, 52, 67}, CH_G = {43, 50, 59},
                   CH_BM = {47, 54, 62}, CH_FSM = {42, 49, 57}, CH_EM = {40, 47, 55};
static const Chord* const BARS[16] = {&CH_D, &CH_D,   &CH_A, &CH_D, &CH_G,  &CH_D, &CH_A7, &CH_D,
                                      &CH_BM, &CH_FSM, &CH_G, &CH_D, &CH_EM, &CH_A, &CH_D,  &CH_D};

// « Les » (Slovensky raj) : air original, mi dorien (do diese), plus lent, sans charleston.
static const Note MELODY_LES[] = {
    {76, 2}, {79, 2}, {83, 4},                     // Em
    {81, 2}, {79, 1}, {78, 1}, {76, 4},            // Em
    {76, 2}, {81, 2}, {85, 2}, {83, 2},            // A
    {79, 4}, {76, 4},                              // Em
    {79, 2}, {83, 2}, {86, 2}, {83, 2},            // G
    {81, 2}, {78, 2}, {74, 4},                     // D
    {76, 2}, {78, 2}, {81, 2}, {85, 2},            // A
    {83, 6}, {0, 2},                               // Em
    {86, 2}, {83, 2}, {79, 2}, {83, 2},            // G
    {81, 4}, {78, 4},                              // D
    {79, 2}, {81, 1}, {79, 1}, {78, 2}, {76, 2},   // Em
    {78, 4}, {74, 2}, {71, 2},                     // Bm
    {79, 2}, {83, 2}, {86, 2}, {88, 2},            // G
    {85, 4}, {81, 4},                              // A
    {83, 2}, {81, 2}, {79, 2}, {78, 2},            // Em
    {76, 6}, {0, 2},                               // Em
};
static const Chord LS_EM = {40, 47, 67}, LS_A = {45, 52, 61}, LS_D = {38, 45, 66}, LS_G = {43, 50, 59},
                   LS_BM = {47, 54, 62};
static const Chord* const BARS_LES[16] = {&LS_EM, &LS_EM, &LS_A, &LS_EM, &LS_G, &LS_D, &LS_A, &LS_EM,
                                          &LS_G,  &LS_D,  &LS_EM, &LS_BM, &LS_G, &LS_A, &LS_EM, &LS_EM};

struct Song {
  const Note* mel;
  int n;
  const Chord* const* bars;
  int sixteenth;   // echantillons par double croche
  bool drums;
  uint8_t melWave;
};
static const Song SONGS[] = {
    {MELODY, (int)(sizeof(MELODY) / sizeof(MELODY[0])), BARS, AUDIO_RATE * 60 / 144 / 4, true, 1},          // 0 Hlavna
    {MELODY_LES, (int)(sizeof(MELODY_LES) / sizeof(MELODY_LES[0])), BARS_LES, AUDIO_RATE * 60 / 112 / 4, false, 2},  // 1 Les
};
static const int NUM_SONGS = sizeof(SONGS) / sizeof(SONGS[0]);

static uint32_t freqInc(float hz) { return (uint32_t)(hz * 4294967296.0f / AUDIO_RATE); }
static float midiHz(int m) { return 440.0f * powf(2.0f, (m - 69) / 12.0f); }

int16_t Audio::Voice::sample() {
  if (!on) return 0;
  // glissando et enveloppe lineaires sur la duree de la note
  uint32_t inc_ = inc;
  int32_t vv = vol;
  if (len > 0) {
    inc_ = inc + (uint32_t)(((int64_t)incEnd - (int64_t)inc) * pos / len);
    vv = vol + (volEnd - vol) * pos / len;
  }
  uint32_t old = phase;
  phase += inc_;
  int32_t s;
  switch (wave) {
    case 0: s = (phase >> 29) == 0 ? 1 : -1; break;            // 12,5 %
    case 1: s = (phase >> 30) == 0 ? 1 : -1; break;            // 25 %
    case 2: s = (phase >> 31) ? 1 : -1; break;                 // 50 %
    case 3: {                                                  // triangle
      int32_t t = (int32_t)(phase >> 16);                      // 0..65535
      s = t < 32768 ? (t - 16384) : (49152 - t);
      if (++pos >= len) on = false;
      return (int16_t)((int32_t)s * vv * 570 / 16384);
    }
    default:                                                   // bruit
      if (phase < old) lfsr = (lfsr >> 1) ^ (-(lfsr & 1) & 0xB400u);
      s = (lfsr & 1) ? 1 : -1;
  }
  if (++pos >= len) on = false;
  return (int16_t)(s * vv * 380);
}

void Audio::startNote(Voice& v, int midi, int len, uint8_t wave, int vol) {
  v.wave = wave;
  v.inc = v.incEnd = freqInc(midiHz(midi));
  v.len = len;
  v.pos = 0;
  v.vol = vol;
  v.volEnd = vol / 3;
  v.on = true;
}

void Audio::play(Sfx s) {
  uint8_t h = sfxHead_;
  uint8_t nh = (h + 1) & 7;
  if (nh == sfxTail_) return;  // file pleine
  pendingSfx_[h] = s;
  sfxHead_ = nh;
}

void Audio::music(int id) { wantMusic_ = id; }

void Audio::setVolume(int v) { vol_ = v < 0 ? 0 : (v > 4 ? 4 : v); }

void Audio::startSfxStep(int ch) {
  Voice& v = v_[4 + ch];
  const Step* st = &SFX_TABLE[sfx_[ch]][sfxStep_[ch]];
  if (st->ms == 0) {
    sfx_[ch] = -1;
    v.on = false;
    return;
  }
  v.wave = st->wave;
  v.inc = freqInc(st->f0);
  v.incEnd = freqInc(st->f1);
  v.len = (int32_t)st->ms * AUDIO_RATE / 1000;
  v.pos = 0;
  v.vol = st->f0 ? st->v0 : 0;
  v.volEnd = st->f0 ? st->v1 : 0;
  v.on = true;
}

void Audio::tickMusic() {
  if (music_ < 0) return;
  const Song& song = SONGS[music_ % NUM_SONGS];
  const int SIXTEENTH = song.sixteenth;
  int bar = (row_ / 8) % 16, pos = row_ % 8;
  // melodie
  if (melLeft_ <= 0) {
    const Note& n = song.mel[melIdx_];
    melLeft_ = n.len;
    if (n.midi) {
      startNote(v_[0], n.midi, n.len * SIXTEENTH * 9 / 10, song.melWave, song.drums ? 6 : 5);
      v_[0].volEnd = 3;
    } else {
      v_[0].on = false;
    }
    melIdx_ = (melIdx_ + 1) % song.n;
  }
  melLeft_--;
  const Chord& c = *song.bars[bar];
  if (pos == 0) startNote(v_[2], c.root, SIXTEENTH * 2, 3, 11);
  if (pos == 4) startNote(v_[2], c.fifth, SIXTEENTH * 2, 3, 11);
  if (pos == 2 || pos == 6) {
    startNote(v_[1], c.pah, SIXTEENTH * 3 / 2, 2, 3);
    if (song.drums) startNote(v_[3], 100, SIXTEENTH / 3, 4, 2);
    v_[3].inc = v_[3].incEnd = freqInc(9000);
  }
  row_ = (row_ + 1) % 128;
}

void Audio::render(int16_t* out, int n) {
  // changements demandes par le jeu
  int wm = wantMusic_;
  if (wm != music_) {
    music_ = wm;
    row_ = 0;
    melIdx_ = 0;
    melLeft_ = 0;
    stepLeft_ = 0;
    for (int i = 0; i < 4; i++) v_[i].on = false;
  }
  while (sfxTail_ != sfxHead_) {
    int s = pendingSfx_[sfxTail_];
    sfxTail_ = (sfxTail_ + 1) & 7;
    int ch = sfx_[0] < 0 ? 0 : (sfx_[1] < 0 ? 1 : 0);
    sfx_[ch] = s;
    sfxStep_[ch] = 0;
    startSfxStep(ch);
  }
  for (int i = 0; i < n; i++) {
    if (music_ >= 0 && --stepLeft_ <= 0) {
      stepLeft_ = SONGS[music_ % NUM_SONGS].sixteenth;
      tickMusic();
    }
    int32_t mix = 0;
    for (int k = 0; k < 4; k++) mix += v_[k].sample();
    for (int ch = 0; ch < 2; ch++) {
      Voice& v = v_[4 + ch];
      if (sfx_[ch] >= 0) {
        if (!v.on) {
          sfxStep_[ch]++;
          startSfxStep(ch);
        }
        mix += v.sample();
      }
    }
    mix = mix * vol_ / 3;
    if (mix > 32767) mix = 32767;
    if (mix < -32768) mix = -32768;
    out[i] = (int16_t)mix;
  }
}
