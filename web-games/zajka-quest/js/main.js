(function (root) {
  'use strict';
  const C = root.ZajkaCommon, Q = root.ZajkaQuest;
  const A = Q.buildAssets(root.ZQ_DATA, root.ZQ_COURSE);
  const audio = new C.Audio(root.ZQ_AUDIO, 2);
  const game = new Q.Game(A);
  game.begin(audio, new Q.BrowserByteStorage('zajkaquest'));
  C.boot({ game, audio, volumeKey: 'zajkaquest.volume' });
})(globalThis);
