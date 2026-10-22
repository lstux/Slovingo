#include "gfx.h"

#include <string.h>

void Band::fill(uint16_t c) {
  int n = SCREEN_W * h;
  for (int i = 0; i < n; i++) buf[i] = c;
}

void Band::fillRect(int x, int y, int w, int hh, uint16_t c) {
  int x0 = x < 0 ? 0 : x, x1 = x + w > SCREEN_W ? SCREEN_W : x + w;
  int ya = y < y0 ? y0 : y, yb = y + hh > y0 + h ? y0 + h : y + hh;
  for (int yy = ya; yy < yb; yy++) {
    uint16_t* row = buf + (yy - y0) * SCREEN_W;
    for (int xx = x0; xx < x1; xx++) row[xx] = c;
  }
}

void Band::blit(const Image8& img, int sx, int sy, int sw, int sh, int dx, int dy, bool flip,
                uint16_t tint, bool solidTint) {
  // ecretage vertical sur la bande
  int ya = dy < y0 ? y0 : dy;
  int yb = dy + sh > y0 + h ? y0 + h : dy + sh;
  if (ya >= yb) return;
  int xa = dx < 0 ? 0 : dx;
  int xb = dx + sw > SCREEN_W ? SCREEN_W : dx + sw;
  if (xa >= xb) return;
  const uint16_t* pal = img.pal;
  for (int yy = ya; yy < yb; yy++) {
    const uint8_t* src = img.px + (sy + yy - dy) * img.w + sx;
    uint16_t* dst = buf + (yy - y0) * SCREEN_W;
    if (!flip) {
      const uint8_t* s = src + (xa - dx);
      for (int xx = xa; xx < xb; xx++) {
        uint8_t i = *s++;
        if (i) dst[xx] = solidTint ? tint : pal[i];
      }
    } else {
      // pixel x ecran -> colonne (sw - 1 - (x - dx)) de la source
      const uint8_t* s = src + (sw - 1 - (xa - dx));
      for (int xx = xa; xx < xb; xx++) {
        uint8_t i = *s--;
        if (i) dst[xx] = solidTint ? tint : pal[i];
      }
    }
  }
}

void Band::frame(const Sheet& s, int index, int dx, int dy, bool flip, uint16_t tint, bool solidTint) {
  int cx = index % s.cols, cy = index / s.cols;
  blit(*s.img, cx * s.fw, cy * s.fh, s.fw, s.fh, dx, dy, flip, tint, solidTint);
}

void Band::wrapRow(const Image8& img, int scrollX, int dy) {
  int ya = dy < y0 ? y0 : dy;
  int yb = dy + img.h > y0 + h ? y0 + h : dy + img.h;
  if (ya >= yb) return;
  int w = img.w;
  int ox = scrollX % w;
  if (ox < 0) ox += w;
  for (int yy = ya; yy < yb; yy++) {
    const uint8_t* src = img.px + (yy - dy) * w;
    uint16_t* dst = buf + (yy - y0) * SCREEN_W;
    int sx = ox;
    for (int x = 0; x < SCREEN_W; x++) {
      uint8_t i = src[sx];
      if (i) dst[x] = img.pal[i];
      if (++sx == w) sx = 0;
    }
  }
}

// ---------------------------------------------------------------- texte
static uint32_t nextCp(const char*& p) {
  uint8_t c = (uint8_t)*p++;
  if (c < 0x80) return c;
  if ((c & 0xE0) == 0xC0) {
    uint32_t v = (c & 0x1F) << 6;
    v |= (uint8_t)(*p++) & 0x3F;
    return v;
  }
  if ((c & 0xF0) == 0xE0) {
    uint32_t v = (c & 0x0F) << 12;
    v |= ((uint8_t)(*p++) & 0x3F) << 6;
    v |= (uint8_t)(*p++) & 0x3F;
    return v;
  }
  return '?';
}

static int findGlyph(const Font& f, uint32_t cp) {
  int lo = 0, hi = f.count - 1;
  while (lo <= hi) {
    int m = (lo + hi) / 2;
    if (f.cp[m] == cp) return m;
    if (f.cp[m] < cp) lo = m + 1;
    else hi = m - 1;
  }
  return -1;
}

int textWidth(const Font& f, const char* s) {
  int w = 0;
  while (*s) {
    int g = findGlyph(f, nextCp(s));
    if (g < 0) g = findGlyph(f, '?');
    if (g >= 0) w += f.w[g];
  }
  return w;
}

static void glyph(Band& b, const Font& f, int g, int x, int y, uint16_t c) {
  int gw = f.w[g], rb = (gw + 7) / 8;
  const uint8_t* bits = f.bits + f.off[g];
  for (int j = 0; j < f.height; j++) {
    int yy = y + j;
    if (yy < b.y0 || yy >= b.y0 + b.h) continue;
    uint16_t* row = b.buf + (yy - b.y0) * SCREEN_W;
    for (int i = 0; i < gw; i++) {
      int xx = x + i;
      if (xx < 0 || xx >= SCREEN_W) continue;
      if (bits[j * rb + (i >> 3)] & (0x80 >> (i & 7))) row[xx] = c;
    }
  }
}

int Band::text(const Font& f, int x, int y, const char* s, uint16_t color, int shadow) {
  if (y + f.height + 1 < y0 || y >= y0 + h) return x + textWidth(f, s);
  while (*s) {
    int g = findGlyph(f, nextCp(s));
    if (g < 0) g = findGlyph(f, '?');
    if (g < 0) continue;
    if (shadow >= 0) glyph(*this, f, g, x + 1, y + 1, (uint16_t)shadow);
    glyph(*this, f, g, x, y, color);
    x += f.w[g];
  }
  return x;
}
