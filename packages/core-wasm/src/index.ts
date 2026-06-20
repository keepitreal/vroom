// @vroom/core-wasm — the framework-agnostic web core for the vroom chart.
//
// Exposes loadVroom(): Promise<VroomModule>, which yields a factory for chart
// instances bound to a <canvas>. Today it resolves to a Canvas2D stub so the
// web stack is buildable/runnable before the Skia-WASM toolchain exists; the
// only change to ship the real core is to instantiate the WASM module inside
// loadVroom() — the VroomModule / VroomChartHandle contract stays identical.

import type { VroomModule } from './handle';
import { createStubModule } from './stub/stubModule';

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

/** Options for loading the core. (WASM URL / font URL land here later.) */
export type LoadVroomOptions = {
  /**
   * Force the Canvas2D stub even once the real WASM core is wired in. Useful
   * for tests / SSR fallbacks. Defaults to false.
   */
  forceStub?: boolean;
};

let modulePromise: Promise<VroomModule> | null = null;

/**
 * Load the chart core. Cached: repeated calls share one module instance.
 *
 * Currently always resolves to the Canvas2D stub. When the Skia-WASM build is
 * available, instantiate it here (guarded by `!opts?.forceStub`) and fall back
 * to the stub on failure.
 */
export function loadVroom(opts?: LoadVroomOptions): Promise<VroomModule> {
  void opts;
  if (!modulePromise) {
    // TODO(web-wasm): instantiate the Skia-WASM module here when available.
    modulePromise = Promise.resolve(createStubModule());
  }
  return modulePromise;
}

/** Test/HMR helper: drop the cached module so the next loadVroom() re-creates it. */
export function resetVroomForTesting(): void {
  modulePromise = null;
}
