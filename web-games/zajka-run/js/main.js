(function (root) {
  'use strict';
  const C = root.ZajkaCommon, R = root.ZajkaRun;
  const assets = R.buildAssets(root.ZR_DATA);
  const audio = new C.Audio(root.ZR_AUDIO, 2);
  const game = new R.Game(assets);
  game.begin(audio, new C.BrowserStorage('zajkarun'));
  C.boot({ game, audio, volumeKey: 'zajkarun.volume' });
})(globalThis);
