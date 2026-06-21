// @vroom/core-wasm — the framework-agnostic web core for the vroom chart.
//
// Exposes loadVroom(): Promise<VroomModule>, which yields a factory for chart
// instances bound to a <canvas>. Today it resolves to a Canvas2D stub so the
// web stack is buildable/runnable before the Skia-WASM toolchain exists; the
// only change to ship the real core is to instantiate the WASM module inside
// loadVroom() — the VroomModule / VroomChartHandle contract stays identical.

import type { VroomModule } from './handle';
import { createStubModule } from './stub/stubModule';
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
   * pass `wasm` to load it from your own URLs instead. On load failure the
   * Canvas2D stub is used unless `fallbackToStub: false`.
   */
  wasm?: WasmConfig;
  /** When wasm loading fails, fall back to the stub (default true). */
  fallbackToStub?: boolean;
  /** Force the Canvas2D stub instead of the WASM core (tests / SSR). */
  forceStub?: boolean;
};

let modulePromise: Promise<VroomModule> | null = null;

/**
 * Load the chart core. Cached: repeated calls share one module instance.
 *
 * Defaults to the real Skia-WASM core bundled in this package. Pass
 * `forceStub` for the Canvas2D stub, or `wasm` to load the core from custom
 * URLs. WASM load failures fall back to the stub unless `fallbackToStub: false`.
 */
export function loadVroom(opts?: LoadVroomOptions): Promise<VroomModule> {
  if (modulePromise) return modulePromise;

  if (opts?.forceStub) {
    modulePromise = Promise.resolve(createStubModule());
  } else {
    const cfg = opts?.wasm ?? bundledWasmConfig();
    const fallback = opts?.fallbackToStub !== false;
    modulePromise = loadWasmCore(cfg).catch((err) => {
      if (!fallback) throw err;
      console.warn('[vroom] Skia-WASM core failed to load; using Canvas2D stub.', err);
      return createStubModule();
    });
  }
  return modulePromise;
}

/** Test/HMR helper: drop the cached module so the next loadVroom() re-creates it. */
export function resetVroomForTesting(): void {
  modulePromise = null;
}
