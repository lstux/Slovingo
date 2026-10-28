#!/usr/bin/env node
// Simulateur en ligne de commande de la version web de Zajka Run : meme interface que
// esp32/zajka-run/sim/sim.cpp (scripts d'entrees, robot, captures PPM, son WAV), pour comparer les
// deux versions image par image.
//
//   node web-games/tests/sim_run.js --bot --level 2 --shots 300,900 --out /tmp/zr_js
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const ROOT = path.join(__dirname, '..');
function load(...files) {
  for (const f of files) vm.runInThisContext(fs.readFileSync(path.join(ROOT, f), 'utf8'), { filename: f });
}
load('common/gfx.js', 'common/audio.js', 'common/input.js', 'common/platform.js', 'zajka-run/data/assets.js', 'zajka-run/data/audio_data.js',
  'zajka-run/js/level.js', 'zajka-run/js/assets.js', 'zajka-run/js/game.js');
const C = globalThis.ZajkaCommon, R = globalThis.ZajkaRun;

const args = process.argv.slice(2);
const opt = { out: '/tmp/zajka_js', script: '', bot: false, frames: 0, every: 0, wav: false, startX: -1, startY: -1,
  level: 0, afterEnd: 0, andrea: false, shots: [], volume: 2, save: '' };
for (let i = 0; i < args.length; i++) {
  const a = args[i];
  if (a === '--out') opt.out = args[++i];
  else if (a === '--script') opt.script = args[++i];
  else if (a === '--bot') opt.bot = true;
  else if (a === '--frames') opt.frames = parseInt(args[++i], 10);
  else if (a === '--every') opt.every = parseInt(args[++i], 10);
  else if (a === '--wav') opt.wav = true;
  else if (a === '--start-x') opt.startX = parseFloat(args[++i]);
  else if (a === '--start-y') opt.startY = parseFloat(args[++i]);
  else if (a === '--level') opt.level = parseInt(args[++i], 10) - 1;
  else if (a === '--andrea') opt.andrea = true;
  else if (a === '--after') opt.afterEnd = parseInt(args[++i], 10);
  else if (a === '--volume') opt.volume = parseInt(args[++i], 10);
  else if (a === '--save') opt.save = args[++i];
  else if (a === '--shots') opt.shots = args[++i].split(',').map((x) => parseInt(x, 10));
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
  return b;
}

const segs = [];
if (opt.script) {
  for (const line of fs.readFileSync(opt.script, 'utf8').split('\n')) {
    if (line.startsWith('#')) continue;
    let m = line.match(/^\s*(\d+)\s+(\d+)\s+(\S+)/);
    if (m) { segs.push({ from: +m[1], to: +m[2], btn: parseButtons(m[3]) }); continue; }
    m = line.match(/^shot\s+(\d+)/);
    if (m) opt.shots.push(+m[1]);
  }
}
if (opt.frames === 0) {
  for (const s of opt.shots) opt.frames = Math.max(opt.frames, s);
  for (const s of segs) opt.frames = Math.max(opt.frames, s.to);
  if (opt.bot && opt.frames < 6000) opt.frames = 6000;
}

// robot : meme logique que sim.cpp
function bot(g, st) {
  let b = C.BTN_RIGHT;
  const s = g.state();
  if (s === R.S_TITLE || s === R.S_INTRO || s === R.S_CLEAR || s === R.S_END || s === R.S_GAMEOVER)
    return (g.frameCount() % 20) < 2 ? (C.BTN_START | C.BTN_A) : 0;
  const p = g.player(), L = g.level();
  const PW = R.Game.PW, PH = R.Game.PH;
  const fx = Math.trunc(p.x + PW + 6 + p.vx * 6) >> 4;
  const feet = Math.trunc(p.y + PH - 1) >> 4;
  const wall = L.kindAt(fx, feet) === R.K_SOLID || L.kindAt(fx, feet - 1) === R.K_SOLID;
  let hole = true;   // pas de sol dans les 4 cases sous le bord avant
  for (let dy = 1; dy <= 4 && hole; dy++)
    for (let dx = 0; dx <= 1; dx++)
      if (L.kindAt(fx + dx, feet + dy) !== R.K_AIR) hole = false;
  const spike = L.kindAt(fx, feet) === R.K_HAZARD || L.kindAt(fx + 1, feet) === R.K_HAZARD;
  let enemy = false, tram = false;
  const ents = g.entities(), n = g.nent_;
  for (let i = 0; i < n; i++) {
    const e = ents[i];
    if ((e.type === R.E_PIGEON || e.type === R.E_WASP) && e.state === 0) {
      const dx = e.x - (p.x + PW);
      if (dx > 0 && dx < 34 && Math.abs(e.y - p.y) < 60) enemy = true;
    }
    if (e.type === R.E_TRAM) {
      const dx = e.x - (p.x + PW);
      if (dx > -10 && dx < 40 && p.riding < 0) tram = true;
    }
  }
  if (hole || process.env.ZR_RUN) b |= C.BTN_B;
  if (p.onGround && (hole || spike || tram)) st.jumpHold = 22;
  else if (p.onGround && (wall || enemy)) st.jumpHold = 12;
  if (st.jumpHold > 0) { b |= C.BTN_A; st.jumpHold--; }
  return b;
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

// printf("%.1f") de la glibc : arrondi exact, les egalites exactes vont au pair (211.25 -> 211.2)
const f1 = (v) => {
  const t = v * 10, fl = Math.floor(t);
  const r = t - fl === 0.5 ? (fl % 2 === 0 ? fl : fl + 1) : Math.round(t);
  return (r / 10).toFixed(1);
};
const assets = R.buildAssets(globalThis.ZR_DATA);
const audio = new C.Audio(globalThis.ZR_AUDIO, opt.volume);
const storage = new C.MemoryStorage();
if (opt.save && fs.existsSync(opt.save)) storage.data = JSON.parse(fs.readFileSync(opt.save, 'utf8'));
const realSave = storage.save.bind(storage);
storage.save = (d) => { realSave(d); if (opt.save) fs.writeFileSync(opt.save, JSON.stringify(d)); };
const game = new R.Game(assets);
game.selectLevel(opt.level);
game.selectCharacter(opt.andrea);
game.begin(audio, storage);
game.selectLevel(opt.level);   // meme si la sauvegarde ne l'a pas encore debloque
const input = new C.Input();
const screen = new C.Screen();
const pcm = [];
const abuf = new Int16Array((C.AUDIO_RATE / 60 | 0) + 1);
const st = { jumpHold: 0 };
let last = game.state(), lastHp = 3, startX = opt.startX, endAt = 0;
for (let f = 1; f <= opt.frames; f++) {
  let raw = 0;
  for (const s of segs) if (f >= s.from && f <= s.to) raw |= s.btn;
  if (opt.bot) raw |= bot(game, st);
  input.update(raw);
  game.step(input);
  if (startX >= 0 && game.state() === R.S_INTRO) { game.debugTeleport(startX, opt.startY); startX = -1; }
  if (opt.wav) {
    audio.render(abuf, C.AUDIO_RATE / 60 | 0);
    for (let i = 0; i < (C.AUDIO_RATE / 60 | 0); i++) pcm.push(abuf[i]);
  }
  if (process.env.ZR_TRACE) {
    const p = game.player();
    console.log(`${f} x=${f1(p.x)} y=${f1(p.y)} vx=${p.vx.toFixed(2)} vy=${p.vy.toFixed(2)} g=${p.onGround ? 1 : 0} raw=${raw.toString(16)}`);
  }
  const p = game.player();
  if (p.hp !== lastHp) {
    if (p.hp < lastHp) console.log(`image ${String(f).padStart(5)} : touche ! pv=${p.hp} x=${p.x.toFixed(0)} y=${p.y.toFixed(0)}`);
    lastHp = p.hp;
  }
  if (game.state() !== last) {
    console.log(`image ${String(f).padStart(5)} : etat ${last} -> ${game.state()}  (x=${p.x.toFixed(0)}, medovniky=${game.hearts()}, vies=${game.lives()})`);
    last = game.state();
  }
  let shot = opt.every && f % opt.every === 0;
  for (const s of opt.shots) shot = shot || s === f;
  if (shot) {
    screen.render(game);
    savePPM(path.join(opt.out, `frame_${String(f).padStart(5, '0')}.ppm`), screen.fb);
  }
  if (opt.bot && (game.state() === R.S_END || game.state() === R.S_CLEAR)) {
    if (!endAt) { endAt = f; console.log(`fin atteinte a l'image ${f}`); }
    if (f >= endAt + opt.afterEnd) break;
  }
}
if (opt.wav) saveWav(path.join(opt.out, 'audio.wav'), Int16Array.from(pcm));
const p = game.player();
console.log(`x=${f1(p.x)} y=${f1(p.y)} etat=${game.state()} medovniky=${game.hearts()} vies=${game.lives()}`);
