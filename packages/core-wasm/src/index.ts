// @vroomchart/core-wasm — the framework-agnostic web core for the vroom chart.
//
// Exposes loadVroom(): Promise<VroomModule>, which yields a factory for chart
// instances bound to a <canvas>. It resolves to the Skia-WASM core bundled in
// this package; pass `wasm` to load it from your own URLs instead.

import type { VroomModule } from './handle';
import { loadWasmCore, type WasmConfig } from './wasm/loadWasm';

// URLs of the WASM assets bundled in this package. Defined here (one level under
// the package root, matching the built dist/index.js) so the relative paths
// resolve correctly both from source in dev and from the bundle in production.
// Static `new URL(..., import.meta.url)` so bundlers (Vite, webpack 5, …) emit
// and rewrite them automatically — no asset hosting needed by consumers.
function bundledWasmConfig(): WasmConfig {
  return {
    moduleUrl: new URL('../assets/vroom_core.mjs', import.meta.url).href,
    wasmUrl: new URL('../assets/vroom_core.wasm', import.meta.url).href,
    fontUrl: new URL('../assets/VroomSans-Regular.ttf', import.meta.url).href,
  };
}

export type {
  VroomModule,
  VroomChartHandle,
  AxisMetrics,
  CrosshairCandle,
  OverlaySpec,
} from './handle';
export { ColorKey } from './handle';
export { packCandles, unpackCandles, BYTES_PER_CANDLE } from './packCandles';
export { parseColor, applyTheme, argbToCss, COLOR_KEYS } from './color';
export type { WasmConfig } from './wasm/loadWasm';

/** Options for loading the core. */
export type LoadVroomOptions = {
  /**
   * Override where the Skia-WASM core is loaded from. By default the core uses
   * the WASM build bundled in this package (turnkey, no asset hosting needed);
   * pass `wasm` to load it from your own URLs instead.
   */
  wasm?: WasmConfig;
};

let modulePromise: Promise<VroomModule> | null = null;

/**
 * Load the chart core. Cached: repeated calls share one module instance.
 *
 * Resolves to the Skia-WASM core bundled in this package, or the core at the
 * URLs in `opts.wasm`. The returned promise rejects if the WASM module fails to
 * load (and the cache is cleared so a later call can retry).
 */
export function loadVroom(opts?: LoadVroomOptions): Promise<VroomModule> {
  if (modulePromise) return modulePromise;

  const cfg = opts?.wasm ?? bundledWasmConfig();
  modulePromise = loadWasmCore(cfg).catch((err) => {
    modulePromise = null; // allow a retry on the next call
    throw err;
  });
  return modulePromise;
}

/** Test/HMR helper: drop the cached module so the next loadVroom() re-creates it. */
export function resetVroomForTesting(): void {
  modulePromise = null;
}
