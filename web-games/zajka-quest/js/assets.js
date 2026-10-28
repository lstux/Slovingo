// Construit les objets de rendu et les donnees de jeu (cartes, cours, scenes) a partir de ce que
// web-games/tools/export_quest.py ecrit dans data/assets.js et data/course.js.
(function (root) {
  'use strict';
  const C = root.ZajkaCommon;

  function buildAssets(data, course) {
    const K = data.consts;
    const images = {}, A = { fonts: {}, K, items: data.items, signWords: data.signWords, numPickups: data.numPickups };
    for (const k in data.images) images[k] = C.makeImage8(data.images[k]);
    for (const k in data.sheets) A[k] = C.makeSheet(data.sheets[k], images);   // spr_fox, walk_andrea, item_icons...
    for (const k in data.fonts) A.fonts[k] = C.makeFont(data.fonts[k]);
    A.portraits = data.portraits.map((p) => (p ? images[p] : null));
    const tilesets = {};
    for (const k in data.tilesets) {
      const t = data.tilesets[k];
      tilesets[k] = { img: images[t.img], kinds: Uint8Array.from(t.kinds), count: t.count };
    }
    A.maps = data.maps.map((m) => ({
      id: m.id, name: m.name, tiles: tilesets[m.tiles], w: m.w, h: m.h, base: Uint8Array.from(m.base),
      over: m.over ? Uint8Array.from(m.over) : null, marks: m.marks, nMarks: m.marks.length, music: m.music,
      night: m.night,
    }));
    A.course = course;
    return A;
  }

  const Q = root.ZajkaQuest = root.ZajkaQuest || {};
  Q.buildAssets = buildAssets;
  // effets sonores (ordre de SFX_TABLE dans audio.cpp)
  Object.assign(Q, { SFX_JUMP: 0, SFX_COIN: 1, SFX_STOMP: 2, SFX_HURT: 3, SFX_SPRING: 4, SFX_BUMP: 5, SFX_LIFE: 6,
    SFX_DIE: 7, SFX_WIN: 8, SFX_CHECK: 9, SFX_PAUSE: 10, SFX_BELL: 11, SFX_GAMEOVER: 12 });
})(globalThis);
