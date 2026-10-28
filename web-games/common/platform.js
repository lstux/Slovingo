// Plateforme navigateur : equivalent de main_odroid*.cpp. Boucle a 60 Hz fixes, affichage du
// tampon RGB565 sur un canvas 320x240, clavier / manette / boutons tactiles, son, sauvegarde.
(function (root) {
  'use strict';

  const C = root.ZajkaCommon;
  const { SCREEN_W, SCREEN_H, Band, makePaletteLUT, Input, InputSources } = C;
  const STEP_MS = 1000 / 60;

  // Sauvegarde dans le navigateur (a la place de la NVS de l'ESP32)
  class BrowserStorage {
    constructor(key) { this.key = key; }
    load() {
      try { const s = root.localStorage.getItem(this.key); return s ? JSON.parse(s) : null; } catch (e) { return null; }
    }
    save(d) { try { root.localStorage.setItem(this.key, JSON.stringify(d)); } catch (e) { /* stockage refuse */ } }
    clear() { try { root.localStorage.removeItem(this.key); } catch (e) { /* ignore */ } }
  }

  class MemoryStorage {
    constructor() { this.data = null; }
    load() { return this.data ? JSON.parse(JSON.stringify(this.data)) : null; }
    save(d) { this.data = JSON.parse(JSON.stringify(d)); }
  }

  // Convertit le tampon RGB565 (Uint16Array) en RGBA pour le canvas
  const BAND_H = 40;
  class Screen {
    constructor() {
      this.fb = new Uint16Array(SCREEN_W * SCREEN_H);
      // comme le firmware : l'image est dessinee par bandes de 40 lignes (le decoupage influe sur
      // quelques bords de sprites, on le garde pour obtenir exactement les memes pixels)
      this.bands = [];
      for (let y0 = 0; y0 < SCREEN_H; y0 += BAND_H)
        this.bands.push(new Band(this.fb.subarray(y0 * SCREEN_W, (y0 + BAND_H) * SCREEN_W), y0, BAND_H));
      this.lut = makePaletteLUT();
      this.rgba = new Uint32Array(SCREEN_W * SCREEN_H);
    }
    render(game) {
      for (const b of this.bands) game.renderBand(b);
      const fb = this.fb, lut = this.lut, out = this.rgba;
      for (let i = 0; i < fb.length; i++) out[i] = lut[fb[i]];
      return out;
    }
  }

  class Platform {
    // opts : { canvas, game, audio, storage, volumeKey, pad (element des boutons tactiles) }
    constructor(opts) {
      this.canvas = opts.canvas;
      this.game = opts.game;
      this.audio = opts.audio;
      this.input = new Input();
      this.sources = new InputSources();
      this.screen = new Screen();
      this.out = new C.WebAudioOut(opts.audio);
      this.volumeKey = opts.volumeKey || 'zajka.volume';
      this.ctx = this.canvas.getContext('2d', { alpha: false });
      this.image = this.ctx.createImageData(SCREEN_W, SCREEN_H);
      this.image32 = new Uint32Array(this.image.data.buffer);
      this.acc = 0;
      this.last = 0;
      this.running = false;
      this.dirty = true;
      try {   // le volume est sauvegarde (la console a un bouton physique)
        const v = root.localStorage.getItem(this.volumeKey);
        if (v !== null && !isNaN(parseInt(v, 10))) this.audio.setVolume(parseInt(v, 10));
      } catch (e) { /* ignore */ }
      this.lastVolume = this.audio.volume();
      this.bindEvents(opts.pad);
    }

    bindEvents(pad) {
      const wake = () => this.out.start();
      root.addEventListener('keydown', (e) => {
        if (e.ctrlKey || e.metaKey || e.altKey) return;
        wake();
        if (this.sources.keyDown(e.code)) e.preventDefault();
      });
      root.addEventListener('keyup', (e) => { if (this.sources.keyUp(e.code)) e.preventDefault(); });
      root.addEventListener('blur', () => this.sources.releaseAll());
      root.addEventListener('pointerdown', wake, { passive: true });
      root.addEventListener('gamepadconnected', wake);
      document.addEventListener('visibilitychange', () => {
        const ctx = this.out.ctx;
        if (!ctx) return;
        if (document.hidden) ctx.suspend(); else ctx.resume();
        this.sources.releaseAll();
      });
      if (pad) this.bindPad(pad);
    }

    // boutons tactiles : <button data-btn="A"> ... et une croix <div data-dpad>
    bindPad(pad) {
      const names = { UP: C.BTN_UP, DOWN: C.BTN_DOWN, LEFT: C.BTN_LEFT, RIGHT: C.BTN_RIGHT, A: C.BTN_A, B: C.BTN_B,
        START: C.BTN_START, SELECT: C.BTN_SELECT, VOLUME: C.BTN_VOLUME };
      const held = new Map();   // pointerId -> masque
      const apply = () => {
        let m = 0;
        for (const v of held.values()) m |= v;
        this.sources.setTouch(m);
      };
      const maskFor = (el, ev) => {
        if (el.dataset.dpad !== undefined) {
          const r = el.getBoundingClientRect();
          const dx = (ev.clientX - (r.left + r.width / 2)) / (r.width / 2);
          const dy = (ev.clientY - (r.top + r.height / 2)) / (r.height / 2);
          let m = 0;
          if (dx < -0.33) m |= C.BTN_LEFT;
          if (dx > 0.33) m |= C.BTN_RIGHT;
          if (dy < -0.33) m |= C.BTN_UP;
          if (dy > 0.33) m |= C.BTN_DOWN;
          return m;
        }
        return names[el.dataset.btn] || 0;
      };
      const on = (ev) => {
        const el = ev.target.closest('[data-btn],[data-dpad]');
        if (!el || !pad.contains(el)) return;
        ev.preventDefault();
        try { el.setPointerCapture(ev.pointerId); } catch (e) { /* ignore */ }
        held.set(ev.pointerId, maskFor(el, ev));
        el.classList.add('on');
        apply();
      };
      const move = (ev) => {
        if (!held.has(ev.pointerId)) return;
        const el = ev.target.closest('[data-btn],[data-dpad]');
        if (el && el.dataset.dpad !== undefined) { held.set(ev.pointerId, maskFor(el, ev)); apply(); }
      };
      const off = (ev) => {
        if (!held.has(ev.pointerId)) return;
        held.delete(ev.pointerId);
        const el = ev.target.closest('[data-btn],[data-dpad]');
        if (el) el.classList.remove('on');
        apply();
      };
      pad.addEventListener('pointerdown', on);
      pad.addEventListener('pointermove', move);
      pad.addEventListener('pointerup', off);
      pad.addEventListener('pointercancel', off);
      pad.addEventListener('contextmenu', (e) => e.preventDefault());
    }

    // un pas de logique (60 Hz)
    stepOnce() {
      this.input.update(this.sources.read());
      this.game.step(this.input);
      if (this.audio.volume() !== this.lastVolume) {
        this.lastVolume = this.audio.volume();
        try { root.localStorage.setItem(this.volumeKey, String(this.lastVolume)); } catch (e) { /* ignore */ }
      }
    }

    draw() {
      this.image32.set(this.screen.render(this.game));
      this.ctx.putImageData(this.image, 0, 0);
    }

    start() {
      if (this.running) return;
      this.running = true;
      this.last = root.performance.now();
      const loop = (now) => {
        if (!this.running) return;
        this.acc += now - this.last;
        this.last = now;
        let steps = 0;
        while (this.acc >= STEP_MS && steps < 4) {
          this.acc -= STEP_MS;
          this.stepOnce();
          steps++;
        }
        if (this.acc > STEP_MS * 4) this.acc = 0;   // trop de retard : on oublie
        if (steps) this.draw();
        root.requestAnimationFrame(loop);
      };
      this.draw();
      root.requestAnimationFrame(loop);
    }

    stop() { this.running = false; }
  }

  // met le canvas a la plus grande taille entiere possible dans son conteneur (pixels nets)
  function fitCanvas(canvas, box) {
    const apply = () => {
      const w = box.clientWidth, h = box.clientHeight;
      if (!w || !h) return;
      let s = Math.min(w / SCREEN_W, h / SCREEN_H);
      if (s >= 2) s = Math.floor(s);
      canvas.style.width = Math.floor(SCREEN_W * s) + 'px';
      canvas.style.height = Math.floor(SCREEN_H * s) + 'px';
    };
    apply();
    root.addEventListener('resize', apply);
    if (root.ResizeObserver) new ResizeObserver(apply).observe(box);
  }

  function toggleFullscreen(el) {
    if (document.fullscreenElement) document.exitFullscreen();
    else if (el.requestFullscreen) el.requestFullscreen().catch(() => {});
    else if (el.webkitRequestFullscreen) el.webkitRequestFullscreen();
  }

  Object.assign(C, { BrowserStorage, MemoryStorage, Screen, Platform, fitCanvas, toggleFullscreen });
})(globalThis);
