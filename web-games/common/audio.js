// Synthe chiptune : port de esp32/zajka-*/src/game/audio.{h,cpp}.
// 4 voix de musique (2 carres, triangle, bruit) + 2 voix d'effets, 22050 Hz mono, calcul en entiers
// comme sur la console. Les tables (effets, morceaux) viennent du jeu (audio_data.js) ; ce fichier
// contient le moteur, commun a Run et a Quest, et la sortie Web Audio.
(function (root) {
  'use strict';

  const AUDIO_RATE = 22050;
  const f32 = Math.fround;

  const freqInc = (hz) => (f32(f32(hz * 4294967296) / AUDIO_RATE)) >>> 0;
  const midiHz = (m) => f32(440 * f32(Math.pow(2, f32((m - 69) / 12))));
  const idiv = (a, b) => Math.trunc(a / b);

  class Voice {
    constructor() {
      this.phase = 0; this.inc = 0; this.incEnd = 0;
      this.len = 0; this.pos = 0;
      this.wave = 0;   // 0 carre 12,5 %, 1 carre 25 %, 2 carre 50 %, 3 triangle, 4 bruit
      this.vol = 0; this.volEnd = 0;
      this.lfsr = 1;
      this.on = false;
    }

    sample() {
      if (!this.on) return 0;
      // glissando et enveloppe lineaires sur la duree de la note
      let inc = this.inc, vv = this.vol;
      if (this.len > 0) {
        inc = (this.inc + (Math.trunc((this.incEnd - this.inc) * this.pos / this.len) >>> 0)) >>> 0;
        vv = this.vol + Math.trunc((this.volEnd - this.vol) * this.pos / this.len);
      }
      const old = this.phase;
      this.phase = (this.phase + inc) >>> 0;
      let s;
      switch (this.wave) {
        case 0: s = (this.phase >>> 29) === 0 ? 1 : -1; break;
        case 1: s = (this.phase >>> 30) === 0 ? 1 : -1; break;
        case 2: s = (this.phase >>> 31) ? 1 : -1; break;
        case 3: {   // triangle
          const t = this.phase >>> 16;
          s = t < 32768 ? (t - 16384) : (49152 - t);
          if (++this.pos >= this.len) this.on = false;
          return idiv(s * vv * 570, 16384);
        }
        default:    // bruit
          if (this.phase < old) this.lfsr = ((this.lfsr >> 1) ^ (-(this.lfsr & 1) & 0xB400)) & 0xFFFF;
          s = (this.lfsr & 1) ? 1 : -1;
      }
      if (++this.pos >= this.len) this.on = false;
      return s * vv * 380;
    }
  }

  class Audio {
    // data : { sfx: [[[wave, f0, f1, ms, v0, v1], ...], ...], songs: [{mel, bars, bpm, drums, melWave}, ...] }
    constructor(data, initialVolume) {
      this.sfx = data.sfx;
      this.songs = data.songs.map((s) => ({
        mel: s.mel, bars: s.bars, drums: s.drums, melWave: s.melWave,
        sixteenth: idiv(idiv(AUDIO_RATE * 60, s.bpm), 4),   // echantillons par double croche
      }));
      this.v_ = [];
      for (let i = 0; i < 6; i++) this.v_.push(new Voice());
      this.pendingSfx_ = [-1, -1, -1, -1, -1, -1, -1, -1];
      this.sfxHead_ = 0; this.sfxTail_ = 0;
      this.wantMusic_ = -1;
      this.music_ = -2;
      this.vol_ = initialVolume === undefined ? 2 : initialVolume;
      this.stepLeft_ = 0;
      this.row_ = 0;
      this.melIdx_ = 0; this.melLeft_ = 0;
      this.sfx_ = [-1, -1]; this.sfxStep_ = [0, 0];
    }

    play(s) {
      const h = this.sfxHead_, nh = (h + 1) & 7;
      if (nh === this.sfxTail_) return;   // file pleine
      this.pendingSfx_[h] = s;
      this.sfxHead_ = nh;
    }

    music(id) { this.wantMusic_ = id; }
    setVolume(v) { this.vol_ = v < 0 ? 0 : (v > 4 ? 4 : v); }
    volume() { return this.vol_; }

    startNote(v, midi, len, wave, vol) {
      v.wave = wave;
      v.inc = v.incEnd = freqInc(midiHz(midi));
      v.len = len;
      v.pos = 0;
      v.vol = vol;
      v.volEnd = idiv(vol, 3);
      v.on = true;
    }

    startSfxStep(ch) {
      const v = this.v_[4 + ch];
      const st = this.sfx[this.sfx_[ch]][this.sfxStep_[ch]];
      if (!st || st[3] === 0) {
        this.sfx_[ch] = -1;
        v.on = false;
        return;
      }
      const [wave, f0, f1, ms, v0, v1] = st;
      v.wave = wave;
      v.inc = freqInc(f0);
      v.incEnd = freqInc(f1);
      v.len = idiv(ms * AUDIO_RATE, 1000);
      v.pos = 0;
      v.vol = f0 ? v0 : 0;
      v.volEnd = f0 ? v1 : 0;
      v.on = true;
    }

    tickMusic() {
      if (this.music_ < 0) return;
      const song = this.songs[this.music_ % this.songs.length];
      const SIXTEENTH = song.sixteenth;
      const bar = ((this.row_ / 8) | 0) % 16, pos = this.row_ % 8;
      // melodie
      if (this.melLeft_ <= 0) {
        const n = song.mel[this.melIdx_];
        this.melLeft_ = n[1];
        if (n[0]) {
          this.startNote(this.v_[0], n[0], idiv(n[1] * SIXTEENTH * 9, 10), song.melWave, song.drums ? 6 : 5);
          this.v_[0].volEnd = 3;
        } else {
          this.v_[0].on = false;
        }
        this.melIdx_ = (this.melIdx_ + 1) % song.mel.length;
      }
      this.melLeft_--;
      const c = song.bars[bar];   // [racine, quinte, contretemps]
      if (pos === 0) this.startNote(this.v_[2], c[0], SIXTEENTH * 2, 3, 11);
      if (pos === 4) this.startNote(this.v_[2], c[1], SIXTEENTH * 2, 3, 11);
      if (pos === 2 || pos === 6) {
        this.startNote(this.v_[1], c[2], idiv(SIXTEENTH * 3, 2), 2, 3);
        if (song.drums) this.startNote(this.v_[3], 100, idiv(SIXTEENTH, 3), 4, 2);
        this.v_[3].inc = this.v_[3].incEnd = freqInc(9000);
      }
      this.row_ = (this.row_ + 1) % 128;
    }

    // Remplit n echantillons mono signes (out : Int16Array).
    render(out, n) {
      // changements demandes par le jeu
      const wm = this.wantMusic_;
      if (wm !== this.music_) {
        this.music_ = wm;
        this.row_ = 0;
        this.melIdx_ = 0;
        this.melLeft_ = 0;
        this.stepLeft_ = 0;
        for (let i = 0; i < 4; i++) this.v_[i].on = false;
      }
      while (this.sfxTail_ !== this.sfxHead_) {
        const s = this.pendingSfx_[this.sfxTail_];
        this.sfxTail_ = (this.sfxTail_ + 1) & 7;
        const ch = this.sfx_[0] < 0 ? 0 : (this.sfx_[1] < 0 ? 1 : 0);
        this.sfx_[ch] = s;
        this.sfxStep_[ch] = 0;
        this.startSfxStep(ch);
      }
      for (let i = 0; i < n; i++) {
        if (this.music_ >= 0 && --this.stepLeft_ <= 0) {
          this.stepLeft_ = this.songs[this.music_ % this.songs.length].sixteenth;
          this.tickMusic();
        }
        let mix = 0;
        for (let k = 0; k < 4; k++) mix += this.v_[k].sample();
        for (let ch = 0; ch < 2; ch++) {
          const v = this.v_[4 + ch];
          if (this.sfx_[ch] >= 0) {
            if (!v.on) {
              this.sfxStep_[ch]++;
              this.startSfxStep(ch);
            }
            mix += v.sample();
          }
        }
        mix = idiv(mix * this.vol_, 3);
        if (mix > 32767) mix = 32767;
        if (mix < -32768) mix = -32768;
        out[i] = mix;
      }
    }
  }

  // ---------------------------------------------------------------- sortie Web Audio
  // Le synthe tourne a 22050 Hz ; si le navigateur n'accepte pas ce taux, on reechantillonne.
  class WebAudioOut {
    constructor(audio, gain) {
      this.audio = audio;
      this.gain = gain === undefined ? 0.4 : gain;   // le DAC de la console est discret, pas un haut-parleur de PC
      this.ctx = null;
      this.started = false;
    }

    // a appeler depuis un geste de l'utilisateur (clavier, toucher, clic)
    start() {
      if (this.started) { if (this.ctx && this.ctx.state === 'suspended') this.ctx.resume(); return; }
      const AC = root.AudioContext || root.webkitAudioContext;
      if (!AC) return;
      this.started = true;
      try { this.ctx = new AC({ sampleRate: AUDIO_RATE, latencyHint: 'interactive' }); } catch (e) { this.ctx = new AC(); }
      const ctx = this.ctx;
      const ratio = AUDIO_RATE / ctx.sampleRate;
      const size = ctx.sampleRate > 30000 ? 2048 : 1024;
      const node = ctx.createScriptProcessor(size, 0, 1);
      const tmp = new Int16Array(size + 8);
      let frac = 0, prev = 0, cur = 0;
      const audio = this.audio, gain = this.gain / 32768;
      node.onaudioprocess = (ev) => {
        const out = ev.outputBuffer.getChannelData(0);
        if (ratio === 1) {
          audio.render(tmp, out.length);
          for (let i = 0; i < out.length; i++) out[i] = tmp[i] * gain;
          return;
        }
        // reechantillonnage lineaire : on tire les echantillons du synthe a la demande
        const need = Math.ceil(out.length * ratio) + 2;
        const buf = need <= tmp.length ? tmp : new Int16Array(need);
        audio.render(buf, need);
        let si = 0;
        for (let i = 0; i < out.length; i++) {
          out[i] = (buf[si] + (buf[si + 1] - buf[si]) * frac) * gain;
          frac += ratio;
          while (frac >= 1) { frac -= 1; si++; }
        }
      };
      node.connect(ctx.destination);
      this.node = node;
      if (ctx.state === 'suspended') ctx.resume();
    }
  }

  const ns = root.ZajkaCommon = root.ZajkaCommon || {};
  Object.assign(ns, { AUDIO_RATE, Audio, WebAudioOut });
})(globalThis);
