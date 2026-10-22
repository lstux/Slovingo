// Rendu par bandes : l'ecran 320x240 est compose bande par bande (320 x BAND_H) dans un petit
// tampon RGB565, envoye a l'ecran par DMA pendant qu'on compose la bande suivante.
// Tout est portable (aucune dependance Arduino) : le simulateur PC utilise le meme code.
#pragma once
#include <stdint.h>

#ifndef PROGMEM
#define PROGMEM
#endif

constexpr int SCREEN_W = 320;
constexpr int SCREEN_H = 240;
constexpr int BAND_H = 40;           // 6 bandes ; 2 tampons de 25 Ko en RAM interne (DMA)

// Image 8 bits indexee ; pal[0] = transparent.
struct Image8 {
  uint16_t w, h;
  const uint16_t* pal;
  const uint8_t* px;
};

// Planche d'images d'animation de fw x fh, cols par ligne.
struct Sheet {
  const Image8* img;
  uint16_t fw, fh, cols;
};

// Police bitmap 1 bit, chasse variable, points de code UTF-8 tries.
struct Font {
  uint8_t height;
  uint16_t count;
  const uint16_t* cp;
  const uint8_t* w;
  const uint16_t* off;
  const uint8_t* bits;
};

struct Theme {
  const Image8* far;
  const Image8* mid;
  const Image8* tiles;
};

inline uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

// Une bande en cours de composition : buf couvre les lignes [y0, y0 + h) de l'ecran.
struct Band {
  uint16_t* buf;
  int y0, h;

  void fill(uint16_t c);
  void fillRect(int x, int y, int w, int h, uint16_t c);
  // Copie une zone (sx, sy, sw, sh) d'une image a (dx, dy) ecran ; flip horizontal possible.
  void blit(const Image8& img, int sx, int sy, int sw, int sh, int dx, int dy, bool flip = false,
            uint16_t tint = 0, bool solidTint = false);
  void frame(const Sheet& s, int index, int dx, int dy, bool flip = false, uint16_t tint = 0,
             bool solidTint = false);
  // Fond qui boucle a l'horizontale (parallaxe).
  void wrapRow(const Image8& img, int scrollX, int dy);
  int text(const Font& f, int x, int y, const char* utf8, uint16_t color, int shadow = -1);
};

int textWidth(const Font& f, const char* utf8);
