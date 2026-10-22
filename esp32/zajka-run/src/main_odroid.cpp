// Zajka Run sur ODROID-GO : ecran, boutons, son. Le jeu lui-meme est dans src/game/ (portable).
//
// Materiel (cf. retro-go, targets/odroid-go) :
//   ecran ILI9341 320x240 en SPI (MOSI 23, SCLK 18, CS 5, DC 21, retroeclairage 14)
//   croix : ADC, X sur GPIO34, Y sur GPIO35 ; A 32, B 33, SELECT 27, START 39, MENU 13, VOLUME 0
//   son : DAC interne sur GPIO25 + GPIO26 (sortie differentielle vers l'ampli du haut-parleur)
#include <Arduino.h>
#include <driver/i2s.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>

#define LGFX_AUTODETECT
#include <LovyanGFX.hpp>

#include "game/game.h"

static LGFX lcd;
static Audio audio;
static Game game;
static Input input;
static uint16_t* bands[2];

// ---------------------------------------------------------------- boutons
static constexpr int PIN_X = 34, PIN_Y = 35;
static constexpr int PIN_A = 32, PIN_B = 33, PIN_SELECT = 27, PIN_START = 39, PIN_MENU = 13, PIN_VOLUME = 0;

static uint16_t readButtons() {
  uint16_t b = 0;
  int x = analogRead(PIN_X), y = analogRead(PIN_Y);
  if (y > 3072) b |= BTN_UP;
  else if (y > 1024) b |= BTN_DOWN;
  if (x > 3072) b |= BTN_LEFT;
  else if (x > 1024) b |= BTN_RIGHT;
  if (!digitalRead(PIN_A)) b |= BTN_A;
  if (!digitalRead(PIN_B)) b |= BTN_B;
  if (!digitalRead(PIN_SELECT)) b |= BTN_SELECT;
  if (!digitalRead(PIN_START)) b |= BTN_START;
  if (!digitalRead(PIN_MENU)) b |= BTN_MENU;
  if (!digitalRead(PIN_VOLUME)) b |= BTN_VOLUME;
  return b;
}

// MENU maintenu une seconde : retour au lanceur du firmware d'origine (partition « factory »).
static void backToLauncher() {
  const esp_partition_t* factory =
      esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, nullptr);
  if (factory) esp_ota_set_boot_partition(factory);
  i2s_driver_uninstall(I2S_NUM_0);
  lcd.fillScreen(TFT_BLACK);
  esp_restart();
}

// ---------------------------------------------------------------- son
static constexpr int AUDIO_CHUNK = 256;

static void audioTask(void*) {
  static int16_t mono[AUDIO_CHUNK];
  static int16_t frames[AUDIO_CHUNK * 2];
  for (;;) {
    audio.render(mono, AUDIO_CHUNK);
    for (int i = 0; i < AUDIO_CHUNK; i++) {
      // sortie differentielle sur les deux DAC, comme retro-go (RG_AUDIO_USE_INT_DAC == 3)
      int sample = mono[i];
      int left, right;
      if (sample > 0x7F00) {
        left = 0x8000 + (sample - 0x7F00);
        right = -0x8000 + 0x7F00;
      } else if (sample < -0x7F00) {
        left = 0x8000 + (sample + 0x7F00);
        right = -0x8000 + -0x7F00;
      } else {
        left = 0x8000;
        right = -0x8000 + sample;
      }
      frames[i * 2] = (int16_t)left;
      frames[i * 2 + 1] = (int16_t)right;
    }
    size_t written;
    i2s_write(I2S_NUM_0, frames, sizeof(frames), &written, portMAX_DELAY);
  }
}

static void audioBegin() {
  i2s_config_t cfg = {};
  cfg.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN);
  cfg.sample_rate = AUDIO_RATE;
  cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  cfg.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  cfg.communication_format = I2S_COMM_FORMAT_STAND_MSB;
  cfg.intr_alloc_flags = 0;
  cfg.dma_buf_count = 4;
  cfg.dma_buf_len = AUDIO_CHUNK;
  cfg.use_apll = false;
  i2s_driver_install(I2S_NUM_0, &cfg, 0, nullptr);
  i2s_set_pin(I2S_NUM_0, nullptr);
  i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
  xTaskCreatePinnedToCore(audioTask, "audio", 4096, nullptr, 5, nullptr, 0);
}

// ---------------------------------------------------------------- ecran
static void renderFrame() {
  lcd.startWrite();
  int k = 0;
  for (int y0 = 0; y0 < SCREEN_H; y0 += BAND_H, k ^= 1) {
    uint16_t* buf = bands[k];
    Band b{buf, y0, BAND_H};
    game.renderBand(b);
    // l'ILI9341 attend du RGB565 poids fort en premier
    uint32_t* w = (uint32_t*)buf;
    for (int i = 0; i < SCREEN_W * BAND_H / 2; i++) {
      uint32_t v = w[i];
      w[i] = ((v & 0x00FF00FFu) << 8) | ((v >> 8) & 0x00FF00FFu);
    }
    lcd.pushImageDMA(0, y0, SCREEN_W, BAND_H, (const lgfx::swap565_t*)buf);
  }
  lcd.endWrite();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
  pinMode(PIN_SELECT, INPUT_PULLUP);
  pinMode(PIN_MENU, INPUT_PULLUP);
  pinMode(PIN_START, INPUT);   // GPIO39 : entree seule, resistance de tirage sur la carte
  pinMode(PIN_VOLUME, INPUT);  // GPIO0 : idem
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_X, ADC_11db);
  analogSetPinAttenuation(PIN_Y, ADC_11db);

  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(180);
  lcd.initDMA();
  lcd.fillScreen(TFT_BLACK);
  for (int i = 0; i < 2; i++)
    bands[i] = (uint16_t*)heap_caps_malloc(SCREEN_W * BAND_H * 2, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
  Serial.printf("Zajka Run : ecran %dx%d, tas libre %u\n", lcd.width(), lcd.height(), ESP.getFreeHeap());

  audioBegin();
  game.begin(&audio);
}

void loop() {
  // logique a 60 Hz fixes, rendu aussi souvent que possible (environ 30 images/s)
  static uint32_t last = micros();
  static uint32_t acc = 0;
  static uint32_t menuSince = 0;
  uint32_t now = micros();
  acc += now - last;
  last = now;
  int steps = 0;
  while (acc >= 16667 && steps < 4) {
    acc -= 16667;
    input.update(readButtons());
    game.step(input);
    steps++;
  }
  if (acc > 16667 * 4) acc = 0;  // trop de retard : on oublie
  if (input.down(BTN_MENU)) {
    if (!menuSince) menuSince = millis();
    else if (millis() - menuSince > 1000) backToLauncher();
  } else {
    menuSince = 0;
  }
  if (steps) renderFrame();
  else delay(1);
}
