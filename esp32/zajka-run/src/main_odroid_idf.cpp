// Zajka Run sur ODROID-GO, version ESP-IDF (sans Arduino) : ecran, boutons, son, sauvegarde.
// Le jeu lui-meme est dans src/game/ (portable).
//
// Pourquoi ESP-IDF : le lanceur d'origine ecrit une table de partitions SANS somme MD5, et
// la bibliotheque Arduino precompilee refuse de demarrer dans ce cas (assert dans
// esp_ota_get_running_partition). Ici CONFIG_PARTITION_TABLE_MD5=n (cf. sdkconfig.defaults),
// comme retro-go.
//
// Materiel (cf. retro-go, targets/odroid-go) :
//   ecran ILI9341 320x240 en SPI (MOSI 23, MISO 19, SCLK 18, CS 5, DC 21, retroeclairage 14)
//   croix : ADC, X sur GPIO34, Y sur GPIO35 ; A 32, B 33, SELECT 27, START 39, MENU 13, VOLUME 0
//   son : DAC interne sur GPIO25 + GPIO26 (sortie differentielle vers l'ampli du haut-parleur)
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "driver/adc.h"
#include "driver/gpio.h"
#include "driver/i2s.h"
#include "esp_heap_caps.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <LovyanGFX.hpp>

#include "game/game.h"

// ---------------------------------------------------------------- ecran
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341 panel_;
  lgfx::Bus_SPI bus_;
  lgfx::Light_PWM light_;

 public:
  LGFX() {
    {
      auto cfg = bus_.config();
      cfg.spi_host = VSPI_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = true;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 18;
      cfg.pin_mosi = 23;
      cfg.pin_miso = 19;
      cfg.pin_dc = 21;
      bus_.config(cfg);
      panel_.setBus(&bus_);
    }
    {
      auto cfg = panel_.config();
      cfg.pin_cs = 5;
      cfg.pin_rst = -1;
      cfg.pin_busy = -1;
      cfg.panel_width = 240;
      cfg.panel_height = 320;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.readable = true;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      panel_.config(cfg);
    }
    {
      auto cfg = light_.config();
      cfg.pin_bl = 14;
      cfg.invert = false;
      cfg.freq = 44100;
      cfg.pwm_channel = 7;
      light_.config(cfg);
      panel_.setLight(&light_);
    }
    setPanel(&panel_);
  }
};

static LGFX lcd;
static Audio audio;
static Game game;
static Input input;
static uint16_t* bands[2];

// ---------------------------------------------------------------- sauvegarde (NVS)
// Partition « nvs » ajoutee par le .fw (cf. tools/mkfw.py) ; en cas d'erreur le jeu tourne
// quand meme, sans sauvegarde.
class NvsStorage : public Storage {
 public:
  bool load(SaveData& d) override {
    nvs_handle_t h;
    if (nvs_open("zajkarun", NVS_READONLY, &h) != ESP_OK) return false;  // pas encore de sauvegarde
    size_t len = sizeof d;
    esp_err_t e = nvs_get_blob(h, "save", &d, &len);
    nvs_close(h);
    return e == ESP_OK && len == sizeof d;
  }
  void save(const SaveData& d) override {
    nvs_handle_t h;
    if (nvs_open("zajkarun", NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_blob(h, "save", &d, sizeof d);
    nvs_commit(h);
    nvs_close(h);
  }
};
static NvsStorage storage;

// ---------------------------------------------------------------- boutons
static constexpr adc1_channel_t ADC_X = ADC1_CHANNEL_6;  // GPIO34
static constexpr adc1_channel_t ADC_Y = ADC1_CHANNEL_7;  // GPIO35
static constexpr int PIN_A = 32, PIN_B = 33, PIN_SELECT = 27, PIN_START = 39, PIN_MENU = 13, PIN_VOLUME = 0;

static void gpioInput(int pin, bool pullup) {
  gpio_config_t c = {};
  c.pin_bit_mask = 1ULL << pin;
  c.mode = GPIO_MODE_INPUT;
  c.pull_up_en = pullup ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
  c.pull_down_en = GPIO_PULLDOWN_DISABLE;
  c.intr_type = GPIO_INTR_DISABLE;
  gpio_config(&c);
}

static uint16_t readButtons() {
  uint16_t b = 0;
  int x = adc1_get_raw(ADC_X), y = adc1_get_raw(ADC_Y);
  if (y > 3072) b |= BTN_UP;
  else if (y > 1024) b |= BTN_DOWN;
  if (x > 3072) b |= BTN_LEFT;
  else if (x > 1024) b |= BTN_RIGHT;
  if (!gpio_get_level((gpio_num_t)PIN_A)) b |= BTN_A;
  if (!gpio_get_level((gpio_num_t)PIN_B)) b |= BTN_B;
  if (!gpio_get_level((gpio_num_t)PIN_SELECT)) b |= BTN_SELECT;
  if (!gpio_get_level((gpio_num_t)PIN_START)) b |= BTN_START;
  if (!gpio_get_level((gpio_num_t)PIN_MENU)) b |= BTN_MENU;
  if (!gpio_get_level((gpio_num_t)PIN_VOLUME)) b |= BTN_VOLUME;
  return b;
}

static uint32_t millis() { return (uint32_t)(esp_timer_get_time() / 1000); }

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

// ---------------------------------------------------------------- programme
static void gameTask(void*) {
  gpioInput(PIN_A, true);
  gpioInput(PIN_B, true);
  gpioInput(PIN_SELECT, true);
  gpioInput(PIN_MENU, true);
  gpioInput(PIN_START, false);   // GPIO39 : entree seule, resistance de tirage sur la carte
  gpioInput(PIN_VOLUME, false);  // GPIO0 : idem
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC_X, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC_Y, ADC_ATTEN_DB_11);

  esp_err_t e = nvs_flash_init();
  if (e == ESP_ERR_NVS_NO_FREE_PAGES || e == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    nvs_flash_erase();
    nvs_flash_init();
  }

  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(180);
  lcd.initDMA();
  lcd.fillScreen(TFT_BLACK);
  for (int i = 0; i < 2; i++)
    bands[i] = (uint16_t*)heap_caps_malloc(SCREEN_W * BAND_H * 2, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
  printf("Zajka Run (ESP-IDF) : ecran %dx%d, tas libre %u, nvs %s\n", (int)lcd.width(), (int)lcd.height(),
         (unsigned)esp_get_free_heap_size(), esp_err_to_name(e));

  audioBegin();
  game.begin(&audio, &storage);

  // logique a 60 Hz fixes, rendu aussi souvent que possible (environ 30 images/s)
  uint32_t last = (uint32_t)esp_timer_get_time();
  uint32_t acc = 0;
  uint32_t menuSince = 0;
  for (;;) {
    uint32_t now = (uint32_t)esp_timer_get_time();
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
    vTaskDelay(1);  // laisse respirer la tache d'inactivite (FREERTOS_HZ = 1000)
  }
}

extern "C" void app_main(void) {
  xTaskCreatePinnedToCore(gameTask, "game", 12288, nullptr, 5, nullptr, 1);
}
