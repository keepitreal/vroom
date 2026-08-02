// @vroomchart/core-wasm — the framework-agnostic web core for the vroom chart.
//
// Exposes loadVroom(): Promise<VroomModule>, which yields a factory for chart
// instances bound to a <canvas>. It resolves to the Skia-WASM core bundled in
// this package; pass `wasm` to load it from your own URLs instead.

import type { VroomModule } from './handle';
import { loadWasmCore, type ResolvedWasmConfig, type WasmConfig } from './wasm/loadWasm';

// URLs of the WASM assets bundled in this package. Defined here (one level under
// the package root, matching the built dist/index.js) so the relative paths
// resolve correctly both from source in dev and from the bundle in production.
// Static `new URL(..., import.meta.url)` so bundlers (Vite, webpack 5, …) emit
// and rewrite them automatically — no asset hosting needed by consumers.
function bundledWasmConfig(): ResolvedWasmConfig {
  return {
    moduleUrl: new URL('../assets/vroom_core.mjs', import.meta.url).href,
    wasmUrl: new URL('../assets/vroom_core.wasm', import.meta.url).href,
    fontUrl: new URL('../assets/VroomSans-Regular.ttf', import.meta.url).href,
  };
}

// Fill in whatever the caller didn't specify. Supplying `moduleUrl` means
// self-hosting, so the other URLs are the caller's to provide too (an absent
// wasmUrl resolves next to the module) — we must not point a self-hosted core at
// this package's bundled .wasm. Everything else (notably `importModule`) layers
// onto the bundled defaults, so overriding just the importer is a one-liner.
function resolveWasmConfig(cfg?: WasmConfig): ResolvedWasmConfig {
  if (cfg?.moduleUrl) return { ...cfg, moduleUrl: cfg.moduleUrl };
  return { ...bundledWasmConfig(), ...cfg, moduleUrl: bundledWasmConfig().moduleUrl };
}

export type {
  VroomModule,
  VroomChartHandle,
  AxisMetrics,
  CrosshairCandle,
  CrosshairInfo,
  OverlaySpec,
  BollingerSpec,
  DrawingSpec,
  BandSpec,
  LiquiditySpec,
  Coord,
} from './handle';
export { ColorKey, FloatKey } from './handle';
export { packCandles, unpackCandles, BYTES_PER_CANDLE } from './packCandles';
export { parseColor, applyTheme, argbToCss, COLOR_KEYS, FLOAT_KEYS, BOOL_KEYS } from './color';
export type { WasmConfig } from './wasm/loadWasm';

/** Options for loading the core. */
export type LoadVroomOptions = {
  /**
   * Override how the Skia-WASM core is loaded. By default it comes from the
   * WASM build bundled in this package (turnkey, no asset hosting needed);
   * set `moduleUrl` to self-host it, or `importModule` to keep the bundled
   * assets but control the dynamic import your bundler emits.
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

  modulePromise = loadWasmCore(resolveWasmConfig(opts?.wasm)).catch((err) => {
    modulePromise = null; // allow a retry on the next call
    throw err;
  });
  return modulePromise;
}

/** Test/HMR helper: drop the cached module so the next loadVroom() re-creates it. */
export function resetVroomForTesting(): void {
  modulePromise = null;
}
