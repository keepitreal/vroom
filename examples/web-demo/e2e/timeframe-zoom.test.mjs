// Timeframe-switch zoom regressions. A switch must keep each candle's pixel
// width (timeframeWindow preserves the visible slot count), so the candle
// pitch measured before and after must match. The broken states this guards
// against: candles crushed to a few px after a long->short switch, or blown
// up to ~8 giant bars after a short->long one.
//
// Run: pnpm --filter web-demo test:e2e   (starts its own Vite server)
//
// Pitch is read off the canvas: the volume bars (one per candle, colored by
// accentBull/accentBear) are themed a pure hue, and the median start-to-start
// distance between lit column runs is the px-per-slot.

import { after, before, describe, test } from 'node:test';
import assert from 'node:assert/strict';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { createServer } from 'vite';
import { chromium } from 'playwright';

const root = fileURLToPath(new URL('..', import.meta.url));
const PITCH_TOLERANCE_PX = 1.5;
const MIN_FILL = 0.4;
// Long enough for WASM init, the 300ms interval morph, and a simulated fetch
// (FETCH_MS) plus the loading hand-off.
const BOOT_MS = 2500;
const SETTLE_MS = 900;
const FETCH_SETTLE_MS = 2000;

let server;
let browser;
let baseUrl;

before(async () => {
  server = await createServer({ root, logLevel: 'error', server: { port: 0, strictPort: false } });
  await server.listen();
  baseUrl = server.resolvedUrls.local[0];
  browser = await chromium.launch();
});

after(async () => {
  await browser?.close();
  await server?.close();
});

async function openDemo(query = '') {
  const page = await browser.newPage({ viewport: { width: 1400, height: 900 } });
  await page.addInitScript(() => {
    localStorage.setItem(
      'vroom-theme',
      JSON.stringify({ accentBull: '#00ff00', accentBear: '#ff00ff' }),
    );
    localStorage.setItem('vroom-transition-ms', '300');
    localStorage.setItem('vroom-candle-width', '8');
  });
  await page.goto(`${baseUrl}${query}`);
  await page.waitForTimeout(BOOT_MS);
  return page;
}

async function measure(page) {
  const png = await page.locator('canvas').first().screenshot();
  return page.evaluate(async (b64) => {
    const img = new Image();
    img.src = `data:image/png;base64,${b64}`;
    await img.decode();
    const c = document.createElement('canvas');
    c.width = img.width;
    c.height = img.height;
    const ctx = c.getContext('2d');
    ctx.drawImage(img, 0, 0);
    const { data, width, height } = ctx.getImageData(0, 0, c.width, c.height);
    // The volume band, above the x-axis labels.
    const y0 = Math.floor(height * 0.75);
    const y1 = Math.floor(height * 0.95);
    // A column counts once it has MIN_RUN vertically contiguous hue pixels, so
    // the dotted current-price line (same accent, 1-2px tall) crossing the
    // band doesn't read as a row of thin bars.
    const MIN_RUN = 4;
    const lit = new Array(width).fill(false);
    for (let x = 0; x < width; x++) {
      let run = 0;
      for (let y = y0; y < y1 && !lit[x]; y++) {
        const i = (y * width + x) * 4;
        const [r, g, b] = [data[i], data[i + 1], data[i + 2]];
        run = (g > 90 && r < 50 && b < 50) || (r > 90 && b > 90 && g < 50) ? run + 1 : 0;
        lit[x] = run >= MIN_RUN;
      }
    }
    const runs = [];
    for (let x = 0; x < width; x++) {
      if (lit[x] && !lit[x - 1]) runs.push({ start: x, width: 0 });
      if (lit[x]) runs[runs.length - 1].width++;
    }
    const median = (a) => [...a].sort((p, q) => p - q)[Math.floor(a.length / 2)] ?? 0;
    return {
      bars: runs.length,
      pitch: median(runs.slice(1).map((r, k) => r.start - runs[k].start)),
      barWidth: median(runs.map((r) => r.width)),
    };
  }, png.toString('base64'));
}

const clickTf = (page, label) =>
  page.getByRole('button', { name: label, exact: true }).first().click();

async function pan(page, dx) {
  const box = await page.locator('canvas').first().boundingBox();
  const y = box.y + box.height * 0.4;
  const x = box.x + box.width * 0.5;
  await page.mouse.move(x, y);
  await page.mouse.down();
  await page.mouse.move(x + dx / 2, y, { steps: 5 });
  await page.mouse.move(x + dx, y, { steps: 5 });
  await page.mouse.up();
  await page.waitForTimeout(300);
}

const slug = (query) => query.replace(/\W+/g, '-').replace(/^-|-$/g, '') || 'plain';

const describeBars = (m) => `pitch ${m.pitch}px, bar width ${m.barWidth}px, ${m.bars} bars`;

// Pitch alone comes from timestamps over the window; the drawn width comes from
// the core's candle duration. Bars must fill their slot without overlapping
// their neighbours.
function assertSaneBars(m, label) {
  assert.ok(
    m.bars > 10 && m.barWidth >= m.pitch * MIN_FILL && m.barWidth < m.pitch,
    `${label}: ${describeBars(m)}`,
  );
}

// Mount at `from`, switch to `to`, and assert the candle pitch held.
async function expectPitchHeld(query, from, to, { settleMs = SETTLE_MS, prepare } = {}) {
  const sep = query ? '&' : '?';
  const page = await openDemo(`${query}${sep}tf=${from}`);
  try {
    if (prepare) await prepare(page);
    const before = await measure(page);
    assertSaneBars(before, `at ${from}, before switching`);
    await clickTf(page, to);
    await page.waitForTimeout(settleMs);
    const afterSwitch = await measure(page);
    const shot = join(tmpdir(), `vroom-tf-${slug(query)}-${from}-${to}.png`);
    await page.screenshot({ path: shot });
    const detail =
      `${from}->${to}: ${describeBars(before)} -> ${describeBars(afterSwitch)} (screenshot: ${shot})`;
    assert.ok(Math.abs(afterSwitch.pitch - before.pitch) <= PITCH_TOLERANCE_PX, detail);
    assertSaneBars(afterSwitch, `${from}->${to}, after switching (screenshot: ${shot})`);
  } finally {
    await page.close();
  }
}

const SWITCHES = [
  ['1h', '5m'],
  ['5m', '1h'],
  ['1h', '1m'],
  ['1m', '1h'],
];

function suite(name, query, opts = {}) {
  describe(name, () => {
    for (const [from, to] of SWITCHES) {
      test(`${from} -> ${to} keeps candle width`, () => expectPitchHeld(query, from, to, opts));
    }
  });
}

// Control: the synchronous switch the demo does by default.
suite('plain switch', '');

// Data withheld (candles=[]) while the new interval "fetches". Expected to
// pass: an empty push holds the last frame and keeps the old series as the
// reference for classifying the next one.
suite('fetch on switch, empty candles', '?fetch=empty', { settleMs: FETCH_SETTLE_MS });

// Same fetch with the `loading` prop raised. The loading branch clears the
// core's candles and prevDataRef, so the new interval lands as 'initial' on a
// core whose window is still in the old interval's milliseconds: nothing
// re-frames it (setDefaultCandleWidth hits an empty buffer, set_candles only
// frames a 0/0 window).
suite('fetch on switch, loading line', '?fetch=loading', { settleMs: FETCH_SETTLE_MS });

// Host round-trips onViewportChange into visibleRange (persisting the user's
// zoom). After a pan the range is explicit, the switch is classified 'stream',
// and the old interval's milliseconds are re-applied to the new one.
suite('controlled visibleRange after a pan', '?controlled=1', {
  prepare: (page) => pan(page, 120),
});

// The core infers candle_duration_ms from candles[1] - candles[0] only (the TS
// layer uses the median of the first 8 intervals). A leading gap doubles the
// drawn slot; a duplicate first timestamp (d = 0) keeps the previous
// interval's duration entirely.
suite('first interval is a gap', '?first=gap');
suite('first interval is a duplicate timestamp', '?first=dup');
