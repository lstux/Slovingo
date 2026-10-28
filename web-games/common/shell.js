// Habillage commun des pages de jeu : canvas ajuste, plein ecran, aide, "toucher pour jouer".
(function (root) {
  'use strict';
  const C = root.ZajkaCommon;

  // opts : { game, audio, storageKey, volumeKey }
  function boot(opts) {
    const $ = (id) => document.getElementById(id);
    const canvas = $('screen');
    const platform = new C.Platform({
      canvas, game: opts.game, audio: opts.audio, volumeKey: opts.volumeKey, pad: $('pad'),
    });
    C.fitCanvas(canvas, $('stage'));
    const start = $('start');
    const go = () => { start.classList.add('hidden'); platform.out.start(); canvas.focus(); };
    start.addEventListener('pointerdown', go);
    root.addEventListener('keydown', () => start.classList.add('hidden'), { once: true });
    $('fs').addEventListener('click', () => C.toggleFullscreen($('app')));
    $('helpbtn').addEventListener('click', () => $('help').classList.toggle('hidden'));
    platform.start();
    root.__platform = platform;   // pratique pour la console
    return platform;
  }
  C.boot = boot;
})(globalThis);
