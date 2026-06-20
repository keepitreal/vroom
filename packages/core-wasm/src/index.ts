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
   * Load the real Skia-WASM core from these URLs instead of the Canvas2D stub.
   * Omit to use the stub. If WASM instantiation fails and `fallbackToStub` is
   * not false, the stub is used and a warning is logged.
   */
  wasm?: WasmConfig;
  /** When wasm loading fails, fall back to the stub (default true). */
  fallbackToStub?: boolean;
  /** Force the Canvas2D stub even when `wasm` is provided (tests / SSR). */
  forceStub?: boolean;
};

let modulePromise: Promise<VroomModule> | null = null;

/**
 * Load the chart core. Cached: repeated calls share one module instance.
 *
 * Default (no `wasm` config) resolves to the Canvas2D stub. Pass `wasm` (built
 * via packages/core CMake VROOM_WASM) to use the real Skia core; on failure it
 * falls back to the stub unless `fallbackToStub: false`.
 */
export function loadVroom(opts?: LoadVroomOptions): Promise<VroomModule> {
  if (modulePromise) return modulePromise;

  if (opts?.wasm && !opts.forceStub) {
    const cfg = opts.wasm;
    const fallback = opts.fallbackToStub !== false;
    modulePromise = loadWasmCore(cfg).catch((err) => {
      if (!fallback) throw err;
      console.warn('[vroom] Skia-WASM core failed to load; using Canvas2D stub.', err);
      return createStubModule();
    });
  } else {
    modulePromise = Promise.resolve(createStubModule());
  }
  return modulePromise;
}

/** Test/HMR helper: drop the cached module so the next loadVroom() re-creates it. */
export function resetVroomForTesting(): void {
  modulePromise = null;
}
