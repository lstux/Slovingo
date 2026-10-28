#!/usr/bin/env node
// Simulateur en ligne de commande de la version web de Zajka Quest : meme interface et memes sorties
// que esp32/zajka-quest/sim/sim.cpp (scripts d'entrees, robot de dialogue, captures PPM, son WAV).
//
//   node web-games/tests/sim_quest.js --script esp32/zajka-quest/sim/chapter.txt --auto --out /tmp/zq_js
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const ROOT = path.join(__dirname, '..');
function load(...files) {
  for (const f of files) vm.runInThisContext(fs.readFileSync(path.join(ROOT, f), 'utf8'), { filename: f });
}
load('common/gfx.js', 'common/audio.js', 'common/input.js', 'common/platform.js',
  'zajka-quest/data/assets.js', 'zajka-quest/data/course.js', 'zajka-quest/data/audio_data.js',
  'zajka-quest/js/assets.js', 'zajka-quest/js/world.js', 'zajka-quest/js/dialog.js', 'zajka-quest/js/game.js');
const C = globalThis.ZajkaCommon, Q = globalThis.ZajkaQuest;

const args = process.argv.slice(2);
const opt = { out: '/tmp/zajka-quest-js', script: '', frames: 0, every: 0, wav: false, auto: false, peek: false, wrong: false,
  save: '', chapter: 0, shots: [], expect: [], volume: 2 };
for (let i = 0; i < args.length; i++) {
  const a = args[i];
  if (a === '--out') opt.out = args[++i];
  else if (a === '--script') opt.script = args[++i];
  else if (a === '--frames') opt.frames = parseInt(args[++i], 10);
  else if (a === '--every') opt.every = parseInt(args[++i], 10);
  else if (a === '--wav') opt.wav = true;
  else if (a === '--auto') opt.auto = true;
  else if (a === '--save') opt.save = args[++i];
  else if (a === '--chapter') opt.chapter = parseInt(args[++i], 10);
  else if (a === '--auto-peek') opt.auto = opt.peek = true;
  else if (a === '--auto-wrong') opt.auto = opt.wrong = true;
  else if (a === '--volume') opt.volume = parseInt(args[++i], 10);
  else if (a === '--shots') opt.shots.push(...args[++i].split(',').map((x) => parseInt(x, 10)));
  else if (a === '--expect') opt.expect.push(...args[++i].split(','));
}
fs.mkdirSync(opt.out, { recursive: true });

function parseButtons(s) {
  let b = 0;
  if (s.includes('L')) b |= C.BTN_LEFT;
  if (s.includes('R')) b |= C.BTN_RIGHT;
  if (s.includes('U')) b |= C.BTN_UP;
  if (s.includes('D')) b |= C.BTN_DOWN;
  if (s.includes('A')) b |= C.BTN_A;
  if (s.includes('B')) b |= C.BTN_B;
  if (s.includes('S')) b |= C.BTN_START;
  if (s.includes('E')) b |= C.BTN_SELECT;
  if (s.includes('V')) b |= C.BTN_VOLUME;
  return b;
}

const segs = [];
if (opt.script) {
  for (const line of fs.readFileSync(opt.script, 'utf8').split('\n')) {
    if (line.startsWith('#')) continue;
    let m = line.match(/^shot\s+(\d+)/);
    if (m) { opt.shots.push(+m[1]); continue; }
    m = line.match(/^\s*(\d+)\s+(\d+)\s+(\S+)/);
    if (m) segs.push({ from: +m[1], to: +m[2], btn: parseButtons(m[3]) });
  }
}
if (opt.frames === 0) {
  for (const s of opt.shots) opt.frames = Math.max(opt.frames, s);
  for (const s of segs) opt.frames = Math.max(opt.frames, s.to);
  if (opt.frames === 0) opt.frames = 600;
}

function savePPM(file, fb) {
  const W = C.SCREEN_W, H = C.SCREEN_H, buf = Buffer.alloc(W * H * 3);
  for (let i = 0; i < W * H; i++) {
    const c = fb[i];
    let r = (c >> 11) << 3, g = ((c >> 5) & 63) << 2, b = (c & 31) << 3;
    r |= r >> 5; g |= g >> 6; b |= b >> 5;
    buf[i * 3] = r; buf[i * 3 + 1] = g; buf[i * 3 + 2] = b;
  }
  fs.writeFileSync(file, Buffer.concat([Buffer.from(`P6\n${W} ${H}\n255\n`), buf]));
}

function saveWav(file, pcm) {
  const n = pcm.length * 2, h = Buffer.alloc(44);
  h.write('RIFF', 0); h.writeUInt32LE(36 + n, 4); h.write('WAVEfmt ', 8); h.writeUInt32LE(16, 16);
  h.writeUInt16LE(1, 20); h.writeUInt16LE(1, 22); h.writeUInt32LE(C.AUDIO_RATE, 24);
  h.writeUInt32LE(C.AUDIO_RATE * 2, 28); h.writeUInt16LE(2, 32); h.writeUInt16LE(16, 34);
  h.write('data', 36); h.writeUInt32LE(n, 40);
  fs.writeFileSync(file, Buffer.concat([h, Buffer.from(pcm.buffer, pcm.byteOffset, pcm.byteLength)]));
}

// stockage de la partie : en memoire, avec --save un fichier la garde d'une execution a l'autre
class FileStorage extends Q.MemoryByteStorage {
  constructor(p) { super(); this.path = p; if (p && fs.existsSync(p)) this.buf = Uint8Array.from(fs.readFileSync(p)); }
  write(u8) { const ok = super.write(u8); if (this.path) fs.writeFileSync(this.path, u8); return ok; }
  erase() { super.erase(); if (this.path && fs.existsSync(this.path)) fs.unlinkSync(this.path); }
}

const STATE_NAMES = ['titre', 'chapitre', 'monde', 'fondu', 'dialogue', 'fin', 'message', 'menu'];
const stateName = (s) => STATE_NAMES[s];
const log = (s) => console.log(s);
const pad = (n, w) => String(n).padStart(w, ' ');
const trace = !!process.env.ZQ_TRACE;

const A = Q.buildAssets(globalThis.ZQ_DATA, globalThis.ZQ_COURSE);
const audio = new C.Audio(globalThis.ZQ_AUDIO, opt.volume);
const storage = new FileStorage(opt.save);
const game = new Q.Game(A);
game.begin(audio, storage);
if (game.hasSave()) log('partie enregistree trouvee');
if (opt.chapter > 1) game.debugChapter(opt.chapter - 1);
const input = new C.Input();
const screen = new C.Screen();
const pcm = [];
const abuf = new Int16Array((C.AUDIO_RATE / 60 | 0) + 1);
let last = game.state();
const seen = [stateName(last)];
let lastMap = null, lastTalks = 0, lastWords = 0, lastTx = -1, lastTy = -1;
const lastItems = new Array(16).fill(0);
let t = 0, guard = 0, wasChoosing = false, peeked = false, erred = false, choiceLine = null;
for (let f = 1; t < opt.frames && guard < opt.frames * 20; f++, guard++) {
  let raw = 0;
  const talking = opt.auto && (game.state() === Q.S_DIALOG || game.state() === Q.S_NOTE);
  if (!talking) {
    t++;
    for (const s of segs) if (t >= s.from && t <= s.to) raw |= s.btn;
  } else if (game.state() === Q.S_NOTE) {
    if (f % 6 === 0 && input.held === 0) raw = C.BTN_A;   // le robot ferme les messages
  } else {
    const d = game.dialog();
    if (d.choosing()) {
      if (!wasChoosing || d.current() !== choiceLine) {
        peeked = erred = false;
        choiceLine = d.current();
        if (trace) log(`image ${pad(f, 5)} : choix « ${choiceLine.sk} » (erreurs jusqu'ici : ${d.mistakes()})`);
      }
      if (f % 4 === 0 && input.held === 0) {   // touches relachees : sinon la pression ne compte pas
        if (opt.peek && !peeked) { raw = C.BTN_SELECT; peeked = true; }
        else if (opt.wrong && !erred) {
          if (d.cursor() === d.correctOption()) raw = C.BTN_RIGHT;
          else {
            if (trace) log(`image ${pad(f, 5)} : erreur voulue (curseur ${d.cursor()}, bon ${d.correctOption()})`);
            raw = C.BTN_A;
            erred = true;
          }
        } else if (d.cursor() !== d.correctOption()) raw = C.BTN_RIGHT;
        else raw = C.BTN_A;
      }
    } else if (f % 6 === 0 && input.held === 0) {
      raw = C.BTN_A;
    }
    wasChoosing = d.choosing();
  }
  input.update(raw);
  game.step(input);
  if (opt.wav) {
    audio.render(abuf, C.AUDIO_RATE / 60 | 0);
    for (let i = 0; i < (C.AUDIO_RATE / 60 | 0); i++) pcm.push(abuf[i]);
  }
  if (game.state() !== last) {
    if (opt.auto) log(`image ${pad(f, 5)} (script ${t}) : ${stateName(last)} -> ${stateName(game.state())}`);
    else log(`image ${pad(f, 5)} : ${stateName(last)} -> ${stateName(game.state())}`);
    if (last === Q.S_TITLE && game.state() === Q.S_WORLD) {
      log(`image ${pad(f, 5)} : reprise de la partie, ${game.bonusFound()} replique(s) secrete(s)`);
      seen.push('reprise', 'bonus:' + game.bonusFound());
    }
    if (last === Q.S_DIALOG) {
      log(`image ${pad(f, 5)} : dialogue fini, ${game.bonusFound()} replique(s) secrete(s) en tout, ${game.dialog().mistakes()} erreur(s)`);
      seen.push('dialogue:fin', 'bonus:' + game.bonusFound());
    }
    last = game.state();
    seen.push(stateName(last));
  }
  if (game.wordsFound() !== lastWords) {
    lastWords = game.wordsFound();
    log(`image ${pad(f, 5)} : Slovnik ${lastWords} mot(s)`);
    seen.push('mots:' + lastWords);
  }
  for (let i = 0; i < A.items.length && i < 16; i++) {
    if (game.itemCount(i) !== lastItems[i]) {
      lastItems[i] = game.itemCount(i);
      log(`image ${pad(f, 5)} : objet ${A.items[i].sk} x${lastItems[i]}`);
      seen.push('objet:' + A.items[i].sk + (lastItems[i] > 1 ? 'x' + lastItems[i] : ''));
    }
  }
  if (game.talks() !== lastTalks) {
    lastTalks = game.talks();
    const sp = game.lastTalk();
    let who = sp === A.K.SPK_FOX ? 'renard' : A.course.speakerNames[sp];
    if (!who) who = 'passant';
    log(`image ${pad(f, 5)} : parle a ${who}`);
    seen.push('parle:' + who);
  }
  if (game.state() !== Q.S_TITLE && game.state() !== Q.S_CHAPTER && game.state() !== Q.S_END) {
    const w = game.world(), p = w.player();
    if (w.map().id !== lastMap) {
      lastMap = w.map().id;
      log(`image ${pad(f, 5)} : carte ${lastMap} (${p.tx}, ${p.ty})`);
      seen.push('carte:' + lastMap);
    }
    if (trace && (p.tx !== lastTx || p.ty !== lastTy)) log(`image ${pad(f, 5)} : case (${p.tx}, ${p.ty})`);
    lastTx = p.tx; lastTy = p.ty;
  }
  let shot = !!(opt.every && f % opt.every === 0);
  if (!talking) for (const s of opt.shots) shot = shot || s === t;
  if (shot) {
    screen.render(game);
    savePPM(path.join(opt.out, `frame_${String(f).padStart(5, '0')}.ppm`), screen.fb);
  }
}
if (opt.wav) saveWav(path.join(opt.out, 'audio.wav'), Int16Array.from(pcm));
log(`${opt.frames} images, etat final : ${stateName(game.state())}, ${storage.writes()} sauvegarde(s)`);
if (opt.expect.length) {
  let k = 0;
  for (const s of seen) if (k < opt.expect.length && s === opt.expect[k]) k++;
  if (k < opt.expect.length) { log(`ECHEC : etat « ${opt.expect[k]} » jamais atteint dans l'ordre attendu`); process.exit(1); }
  log('parcours attendu : ok');
}
