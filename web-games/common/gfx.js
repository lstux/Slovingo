// Rendu 320x240 en RGB565 : port de esp32/zajka-*/src/game/gfx.{h,cpp}.
// Meme principe que le firmware : images 8 bits indexees (palette[0] = transparent), polices
// bitmap 1 bit, composition dans un tampon RGB565. La plateforme convertit ensuite en RGBA.
(function (root) {
  'use strict';

  const SCREEN_W = 320, SCREEN_H = 240, BAND_H = 40;

  function rgb(r, g, b) { return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3); }

  // ---------------------------------------------------------------- decodage des donnees exportees
  function decodeB64(s) {
    if (typeof atob === 'function') {
      const bin = atob(s), a = new Uint8Array(bin.length);
      for (let i = 0; i < bin.length; i++) a[i] = bin.charCodeAt(i);
      return a;
    }
    return new Uint8Array(Buffer.from(s, 'base64'));
  }

  // PackBits : 0..127 = copier n+1 octets, 129..255 = repeter l'octet suivant 257-n fois
  function unpackBits(blob) {
    let size = 0;
    for (let i = 0; i < blob.length;) {
      const n = blob[i++];
      if (n < 128) { size += n + 1; i += n + 1; } else if (n > 128) { size += 257 - n; i++; }
    }
    const out = new Uint8Array(size);
    let o = 0;
    for (let i = 0; i < blob.length;) {
      const n = blob[i++];
      if (n < 128) {
        for (let k = 0; k <= n; k++) out[o++] = blob[i++];
      } else if (n > 128) {
        const v = blob[i++];
        for (let k = 0, c = 257 - n; k < c; k++) out[o++] = v;
      }
    }
    return out;
  }

  const unpackB64 = (s) => unpackBits(decodeB64(s));

  // Image8 : { w, h, pal: Uint16Array (RGB565, pal[0] ignore), px: Uint8Array (indices) }
  function makeImage8(o) { return { w: o.w, h: o.h, pal: Uint16Array.from(o.pal), px: unpackB64(o.px) }; }

  // Sheet : { img, fw, fh, cols }
  function makeSheet(o, images) { return { img: images[o.img], fw: o.fw, fh: o.fh, cols: o.cols }; }

  // Font : { height, count, cp, w, off, bits }
  function makeFont(o) {
    return {
      height: o.height, count: o.cp.length, cp: Uint16Array.from(o.cp), w: Uint8Array.from(o.w),
      off: Uint16Array.from(o.off), bits: unpackB64(o.bits),
    };
  }

  // ---------------------------------------------------------------- texte
  function findGlyph(f, cp) {
    let lo = 0, hi = f.count - 1;
    while (lo <= hi) {
      const m = (lo + hi) >> 1;
      if (f.cp[m] === cp) return m;
      if (f.cp[m] < cp) lo = m + 1; else hi = m - 1;
    }
    return -1;
  }

  function glyphOf(f, ch) {
    let g = findGlyph(f, ch.codePointAt(0));
    if (g < 0) g = findGlyph(f, 63);   // '?'
    return g;
  }

  function textWidth(f, s) {
    let w = 0;
    for (const ch of s) {
      const g = glyphOf(f, ch);
      if (g >= 0) w += f.w[g];
    }
    return w;
  }

  function glyph(b, f, g, x, y, c) {
    const gw = f.w[g], rb = (gw + 7) >> 3, bits = f.bits, base = f.off[g];
    for (let j = 0; j < f.height; j++) {
      const yy = y + j;
      if (yy < b.y0 || yy >= b.y0 + b.h) continue;
      const row = (yy - b.y0) * SCREEN_W;
      for (let i = 0; i < gw; i++) {
        const xx = x + i;
        if (xx < 0 || xx >= SCREEN_W) continue;
        if (bits[base + j * rb + (i >> 3)] & (0x80 >> (i & 7))) b.buf[row + xx] = c;
      }
    }
  }

  // Une bande en cours de composition : buf couvre les lignes [y0, y0 + h) de l'ecran.
  class Band {
    constructor(buf, y0, h) { this.buf = buf; this.y0 = y0; this.h = h; }

    fill(c) { this.buf.fill(c, 0, SCREEN_W * this.h); }

    fillRect(x, y, w, hh, c) {
      const x0 = x < 0 ? 0 : x, x1 = x + w > SCREEN_W ? SCREEN_W : x + w;
      const ya = y < this.y0 ? this.y0 : y, yb = y + hh > this.y0 + this.h ? this.y0 + this.h : y + hh;
      for (let yy = ya; yy < yb; yy++) {
        const row = (yy - this.y0) * SCREEN_W;
        for (let xx = x0; xx < x1; xx++) this.buf[row + xx] = c;
      }
    }

    // Copie une zone (sx, sy, sw, sh) d'une image a (dx, dy) ecran ; flip horizontal possible.
    blit(img, sx, sy, sw, sh, dx, dy, flip, tint, solidTint) {
      const ya = dy < this.y0 ? this.y0 : dy;
      const yb = dy + sh > this.y0 + this.h ? this.y0 + this.h : dy + sh;
      if (ya >= yb) return;
      const xa = dx < 0 ? 0 : dx;
      const xb = dx + sw > SCREEN_W ? SCREEN_W : dx + sw;
      if (xa >= xb) return;
      const pal = img.pal, px = img.px, buf = this.buf;
      for (let yy = ya; yy < yb; yy++) {
        const src = (sy + yy - dy) * img.w + sx;
        const dst = (yy - this.y0) * SCREEN_W;
        if (!flip) {
          let s = src + (xa - dx);
          for (let xx = xa; xx < xb; xx++) {
            const i = px[s++];
            if (i) buf[dst + xx] = solidTint ? tint : pal[i];
          }
        } else {
          let s = src + (sw - 1 - (xa - dx));
          for (let xx = xa; xx < xb; xx++) {
            const i = px[s--];
            if (i) buf[dst + xx] = solidTint ? tint : pal[i];
          }
        }
      }
    }

    frame(s, index, dx, dy, flip, tint, solidTint) {
      const cx = index % s.cols, cy = (index / s.cols) | 0;
      this.blit(s.img, cx * s.fw, cy * s.fh, s.fw, s.fh, dx, dy, flip, tint, solidTint);
    }

    // Fond qui boucle a l'horizontale (parallaxe).
    wrapRow(img, scrollX, dy) {
      const ya = dy < this.y0 ? this.y0 : dy;
      const yb = dy + img.h > this.y0 + this.h ? this.y0 + this.h : dy + img.h;
      if (ya >= yb) return;
      const w = img.w;
      let ox = scrollX % w;
      if (ox < 0) ox += w;
      for (let yy = ya; yy < yb; yy++) {
        const src = (yy - dy) * w;
        const dst = (yy - this.y0) * SCREEN_W;
        let sx = ox;
        for (let x = 0; x < SCREEN_W; x++) {
          const i = img.px[src + sx];
          if (i) this.buf[dst + x] = img.pal[i];
          if (++sx === w) sx = 0;
        }
      }
    }

    text(f, x, y, s, color, shadow) {
      if (shadow === undefined) shadow = -1;
      if (y + f.height + 1 < this.y0 || y >= this.y0 + this.h) return x + textWidth(f, s);
      for (const ch of s) {
        const g = glyphOf(f, ch);
        if (g < 0) continue;
        if (shadow >= 0) glyph(this, f, g, x + 1, y + 1, shadow);
        glyph(this, f, g, x, y, color);
        x += f.w[g];
      }
      return x;
    }
  }

  // RGB565 -> RGBA (Uint32 little-endian : 0xAABBGGRR), avec les memes bits de poids faible repliques
  // que l'ILI9341 (le bit de poids fort est recopie dans les bits manquants).
  function makePaletteLUT() {
    const lut = new Uint32Array(65536);
    for (let c = 0; c < 65536; c++) {
      let r = (c >> 11) << 3, g = ((c >> 5) & 63) << 2, b = (c & 31) << 3;
      r |= r >> 5; g |= g >> 6; b |= b >> 5;
      lut[c] = (0xFF000000 | (b << 16) | (g << 8) | r) >>> 0;
    }
    return lut;
  }

  const ns = root.ZajkaCommon = root.ZajkaCommon || {};
  Object.assign(ns, {
    SCREEN_W, SCREEN_H, BAND_H, rgb, decodeB64, unpackBits, unpackB64,
    makeImage8, makeSheet, makeFont, textWidth, Band, makePaletteLUT,
  });
})(globalThis);
