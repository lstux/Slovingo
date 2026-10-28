// Ouvre la page dans Chromium (file://), joue un peu au clavier, verifie l'absence d'erreurs et prend des captures.
const { chromium } = require('playwright');
const path = require('path');
(async () => {
  const page_ = process.argv[2] || 'zajka-run';
  const out = process.argv[3] || '/tmp/zajka_browser';
  require('fs').mkdirSync(out, { recursive: true });
  const b = await chromium.launch();
  const pg = await b.newPage({ viewport: { width: 960, height: 720 } });
  const errs = [];
  pg.on('pageerror', (e) => errs.push(String(e)));
  pg.on('console', (m) => { if (m.type() === 'error') errs.push(m.text()); });
  await pg.goto('file://' + path.resolve(__dirname, '..', page_, 'index.html'));
  await pg.waitForTimeout(500);
  await pg.screenshot({ path: out + '/0_start.png' });
  await pg.keyboard.press('Enter');
  await pg.waitForTimeout(600);
  await pg.screenshot({ path: out + '/1_title.png' });
  await pg.keyboard.press('Enter');
  await pg.waitForTimeout(500);
  await pg.keyboard.press('Space');
  await pg.keyboard.down('ArrowRight');
  await pg.waitForTimeout(2500);
  await pg.screenshot({ path: out + '/2_play.png' });
  await pg.keyboard.up('ArrowRight');
  console.log('erreurs :', errs.length ? errs : 'aucune');
  await b.close();
})();
