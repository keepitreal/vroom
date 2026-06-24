// Adapts the emscripten `WebChart` (embind class from web/vroom_web.cpp) to the
// VroomChartHandle / VroomModule contract that @vroomchart/react consumes. This is
// the only place that knows the WASM module's shape — swapping the stub for the
// real core changes nothing above this line.

import type {
  AxisMetrics,
  CrosshairCandle,
  CrosshairInfo,
  OverlaySpec,
  VroomChartHandle,
  VroomModule,
} from '../handle';

// The embind class instance. Methods mirror web/vroom_web.cpp; `delete()` frees
// the C++ object (embind requires explicit disposal).
interface WebChartInstance {
  setCandles(bytes: Uint8Array): void;
  setSize(width: number, height: number, dpr: number): void;
  setColor(key: number, argb: number): void;
  setVisibleRange(startMs: number, endMs: number): void;
  pan(dx: number, dy: number): void;
  translate(dx: number, dy: number): void;
  zoom(sx: number, sy: number, fx: number, fy: number): void;
  scalePriceAxis(dy: number): void;
  scaleTimeAxis(dx: number): void;
  getAxisMetrics(): AxisMetrics;
  setCrosshair(x: number, y: number): void;
  clearCrosshair(): void;
  getCrosshairCandle(): CrosshairCandle | null;
  getCrosshairInfo(): CrosshairInfo | null;
  setRSI(e: boolean, p: number, u: number, l: number, mae: boolean, map: number): void;
  setMACD(e: boolean, f: number, s: number, sig: number): void;
  setOverlays(overlays: OverlaySpec[]): void;
  setVWAP(e: boolean, reset: number, color: number, width: number): void;
  setTypeface(bytes: Uint8Array): void;
  isAnimating(): boolean;
  present(): void;
  delete(): void;
}

/** The shape of the instantiated emscripten module we depend on. */
export interface VroomWasmModule {
  WebChart: new (canvasSelector: string) => WebChartInstance;
}

let uid = 0;

class WasmHandle implements VroomChartHandle {
  constructor(private wc: WebChartInstance) {}

  setCandles(packed: ArrayBuffer): void {
    this.wc.setCandles(new Uint8Array(packed));
  }
  setSize(width: number, height: number, dpr: number): void {
    this.wc.setSize(width, height, dpr);
  }
  setColor(key: number, argb: number): void {
    this.wc.setColor(key, argb >>> 0);
  }
  setVisibleRange(startMs: number, endMs: number): void {
    this.wc.setVisibleRange(startMs, endMs);
  }
  pan(dx: number, dy: number): void {
    this.wc.pan(dx, dy);
  }
  translate(dx: number, dy: number): void {
    this.wc.translate(dx, dy);
  }
  zoom(scaleX: number, scaleY: number, fx: number, fy: number): void {
    this.wc.zoom(scaleX, scaleY, fx, fy);
  }
  scalePriceAxis(dy: number): void {
    this.wc.scalePriceAxis(dy);
  }
  scaleTimeAxis(dx: number): void {
    this.wc.scaleTimeAxis(dx);
  }
  getAxisMetrics(): AxisMetrics {
    return this.wc.getAxisMetrics();
  }
  setCrosshair(x: number, y: number): void {
    this.wc.setCrosshair(x, y);
  }
  clearCrosshair(): void {
    this.wc.clearCrosshair();
  }
  getCrosshairCandle(): CrosshairCandle | null {
    return this.wc.getCrosshairCandle();
  }
  getCrosshairInfo(): CrosshairInfo | null {
    return this.wc.getCrosshairInfo();
  }
  setRSI(
    enabled: boolean,
    period: number,
    upperBand: number,
    lowerBand: number,
    maEnabled: boolean,
    maPeriod: number,
  ): void {
    this.wc.setRSI(enabled, period, upperBand, lowerBand, maEnabled, maPeriod);
  }
  setMACD(enabled: boolean, fast: number, slow: number, signal: number): void {
    this.wc.setMACD(enabled, fast, slow, signal);
  }
  setOverlays(overlays: OverlaySpec[]): void {
    this.wc.setOverlays(overlays);
  }
  setVWAP(enabled: boolean, resetOffsetMin: number, color: number, width: number): void {
    this.wc.setVWAP(enabled, resetOffsetMin, color >>> 0, width);
  }
  isAnimating(): boolean {
    return this.wc.isAnimating();
  }
  present(): void {
    this.wc.present();
  }
  destroy(): void {
    this.wc.delete();
  }
}

/**
 * Wrap an instantiated WASM module as a VroomModule. `fontBytes` (a .ttf/.otf)
 * is installed as the axis typeface on each chart — required for text labels,
 * since the WASM sandbox has no system fonts.
 */
export function makeWasmModule(
  module: VroomWasmModule,
  fontBytes: Uint8Array | null,
): VroomModule {
  return {
    create(canvas: HTMLCanvasElement) {
      if (!canvas.id) canvas.id = `vroom-canvas-${uid++}`;
      const wc = new module.WebChart(`#${canvas.id}`);
      if (fontBytes) wc.setTypeface(fontBytes);
      return new WasmHandle(wc);
    },
  };
}
