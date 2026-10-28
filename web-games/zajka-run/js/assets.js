// Construit les objets de rendu (images, planches, themes, polices, niveaux) a partir des donnees
// exportees par web-games/tools/export_run.py (data/assets.js).
(function (root) {
  'use strict';
  const C = root.ZajkaCommon;

  function buildAssets(data) {
    const images = {}, A = { fonts: {}, themes: {}, levels: [] };
    for (const k in data.images) images[k] = C.makeImage8(data.images[k]);
    for (const k in data.sheets) A[k] = C.makeSheet(data.sheets[k], images);   // spr_fox, spr_medovnik, ...
    for (const k in data.themes) {
      const t = data.themes[k];
      A.themes[k] = { far: images[t.far], mid: images[t.mid], tiles: images[t.tiles], check: A[t.check],
        decor: A[t.decor], under: t.under, under2: t.under2 };
    }
    for (const k in data.fonts) A.fonts[k] = C.makeFont(data.fonts[k]);
    for (const l of data.levels) {
      A.levels.push({ name: l.name, subtitle: l.subtitle, hint: l.hint, theme: A.themes[l.theme], music: l.music,
        npc: l.npc ? A[l.npc] : null, weather: l.weather, next: l.next, w: l.w, h: l.h, rows: l.rows });
    }
    return A;
  }

  (root.ZajkaRun = root.ZajkaRun || {}).buildAssets = buildAssets;
})(globalThis);
