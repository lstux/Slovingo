// Synthe chiptune portable : 4 voix de musique (2 carres, triangle, bruit) + 2 voix d'effets.
// Le jeu appelle play()/music() ; la plateforme appelle render() depuis sa tache audio.
#pragma once
#include <stdint.h>

constexpr int AUDIO_RATE = 22050;

enum Sfx : uint8_t {
  SFX_JUMP, SFX_COIN, SFX_STOMP, SFX_HURT, SFX_SPRING, SFX_BUMP, SFX_LIFE,
  SFX_DIE, SFX_WIN, SFX_CHECK, SFX_PAUSE, SFX_BELL, SFX_GAMEOVER, SFX_THUNDER, SFX_CRUMBLE, SFX_COUNT
};

class Audio {
 public:
  void play(Sfx s);
  void music(int id);            // -1 : silence
  void setVolume(int v);         // 0..4
  int volume() const { return vol_; }
  // Remplit n echantillons mono signes (appele par la plateforme, autre coeur/tache).
  void render(int16_t* out, int n);

 private:
  struct Voice {
    uint32_t phase = 0, inc = 0, incEnd = 0;
    int32_t len = 0, pos = 0;
    uint8_t wave = 0;  // 0 carre 12,5 %, 1 carre 25 %, 2 carre 50 %, 3 triangle, 4 bruit
    int16_t vol = 0, volEnd = 0;
    uint16_t lfsr = 1;
    bool on = false;
    int16_t sample();
  };
  void startNote(Voice& v, int midi, int len, uint8_t wave, int vol);
  void startSfxStep(int ch);
  void tickMusic();

  Voice v_[6];
  volatile int pendingSfx_[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
  volatile uint8_t sfxHead_ = 0, sfxTail_ = 0;
  volatile int wantMusic_ = -1;
  int music_ = -2;
  int vol_ = 0;   // 0 = muet : on demarre en silence (VOLUME pour monter)
  // sequenceur de musique
  int32_t stepLeft_ = 0;
  int row_ = 0;
  int melIdx_ = 0, melLeft_ = 0;
  // effets : etape courante par voix d'effet
  int sfx_[2] = {-1, -1}, sfxStep_[2] = {0, 0};
};
