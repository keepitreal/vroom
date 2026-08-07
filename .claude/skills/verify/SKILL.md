---
name: verify
description: Build, run, and visually verify vroom chart changes end-to-end via the web demo driven by headless Playwright.
---

# Verifying vroom changes

## Build chain (order matters)

1. C++ core changed? Rebuild WASM first — the demo loads the committed assets:
   ```bash
   source ~/emsdk/emsdk_env.sh
   SKIA_DIR=/Users/darion/skia pnpm --filter @vroomchart/core-wasm build:wasm
   ```
   Outputs to `packages/core-wasm/assets/`. Core unit tests: `pnpm test:core`
   (compiles only Skia-free TUs — facade/labels compile errors surface only in
   the WASM build).
2. TS packages resolve each other via built `dist/`, so after editing
   `core-wasm` or `react` sources: `pnpm --filter <pkg> build` before
   typechecking dependents.
3. React unit tests: `pnpm --filter @vroomchart/react test` (vitest).

## Run + drive the demo

```bash
pnpm --filter web-demo dev --port 5199 --strictPort   # background
```

Drive with Playwright (install `playwright` + chromium in a scratchpad dir if
missing). Headless chromium logs Skia "Shader compilation error" console
errors — harmless, rendering still works; filter them out. Wait ~2.5s after
load for WASM init, ~700ms after each interaction before screenshotting.

Useful selectors (demo top bar): view buttons `Repro`/`Demo`, asset buttons
`BTC`/`SOL`, timeframe buttons `1m`/`5m`/`15m`/`1h`, `seriesKey` checkbox
(first `input[type=checkbox]`). Axis labels are drawn into the canvas — verify
by reading screenshots, not the DOM. The crosshair OHLC readout in the top bar
IS DOM text if you need programmatic values.

Gestures on the canvas (`page.locator('canvas').last()`):
- pan: mouse drag in the chart body
- price-axis drag (manual y): vertical drag at `box.width - 20` from the left
- streaming: Repro view, `Add candle` / `Update last candle` buttons

## Flows worth driving after viewport/data changes

- 1m→15m: candle pixel width constant, right edge anchored on last close
  badge, y refit
- BTC→SOL (seriesKey on and off): full reset, y-axis ~80 not ~55k
- pan in auto-y → y follows; price-axis drag → y freezes across next pan
- manual y then asset switch → freeze must clear
- manual y then timeframe switch → freeze must SURVIVE, rescaled so the candle
  high-low envelope keeps its pixel height (see preservePriceEnvelope). Volume
  bars and the price indicator share the candle colors by default, so to measure
  the envelope from a screenshot first override `accentBull`/`accentBear` to a
  distinct hue (the demo persists its theme under localStorage `vroom-theme`).
  Compare the manual/auto envelope ratio at each interval, not absolute pixels —
  the extreme high/low is a 1px antialiased wick tip.
- Repro view appends → x-window must not move
