// Adapts the emscripten `WebChart` (embind class from web/vroom_web.cpp) to the
// VroomChartHandle / VroomModule contract that @vroomchart/react consumes. This is
// the only place that knows the WASM module's shape — swapping the stub for the
// real core changes nothing above this line.

import type {
  AxisMetrics,
  BollingerSpec,
  Coord,
  CrosshairCandle,
  CrosshairInfo,
  DrawingSpec,
  LiquiditySpec,
  MACDSpec,
  OverlaySpec,
  PriceLinesSpec,
  VolumeSpec,
  VroomChartHandle,
  VroomModule,
} from '../handle';

// The embind class instance. Methods mirror web/vroom_web.cpp; `delete()` frees
// the C++ object (embind requires explicit disposal).
//
// This interface — and WasmHandle's forwarding below — mirrors VroomChartHandle
// argument-for-argument, so it MUST be updated whenever a handle signature
// changes. TypeScript will NOT flag a dropped trailing parameter: a method with
// fewer params still satisfies an interface requiring more, so the argument is
// silently lost at runtime instead of failing to compile.
interface WebChartInstance {
  setCandles(bytes: Uint8Array): void;
  setSize(width: number, height: number, dpr: number): void;
  setColor(key: number, argb: number): void;
  setFloat(key: number, value: number): void;
  setVisibleRange(startMs: number, endMs: number): void;
  setDefaultCandleWidth(px: number): void;
  setChartType(mode: number): void;
  setMorph(collapse: number, fade: number): void;
  getVisibleRange(): { startMs: number; endMs: number };
  resetView(): void;
  resetPriceScale(): void;
  getVisiblePriceEnvelope(): { low: number; high: number } | null;
  preservePriceEnvelope(prevLow: number, prevHigh: number): void;
  beginIntervalMorph(): void;
  setIntervalMorph(t: number): void;
  pan(dx: number, dy: number): void;
  translate(dx: number, dy: number): void;
  zoom(sx: number, sy: number, fx: number, fy: number): void;
  scalePriceAxis(dy: number): void;
  scaleTimeAxis(dx: number): void;
  resizeIndicatorPane(dy: number): void;
  scaleIndicatorAxis(y: number, dy: number): void;
  getAxisMetrics(): AxisMetrics;
  setCrosshair(x: number, y: number): void;
  setCrosshairData(timeMs: number, price: number): void;
  clearCrosshair(): void;
  getCrosshairCandle(): CrosshairCandle | null;
  getCrosshairInfo(): CrosshairInfo | null;
  setRSI(e: boolean, p: number, u: number, l: number, mae: boolean, map: number): void;
  setMACD(spec: MACDSpec): void;
  setOverlays(overlays: OverlaySpec[]): void;
  setVWAP(e: boolean, reset: number, color: number, width: number): void;
  setBollinger(spec: BollingerSpec): void;
  setVolume(spec: VolumeSpec): void;
  setVolumeCollapse(t: number, easing: number): void;
  setDrawings(drawings: DrawingSpec[]): void;
  setLiquidity(liquidity: LiquiditySpec): void;
  setPriceLines(priceLines: PriceLinesSpec): void;
  hitTestPriceLine(x: number, y: number): { index: number; part: number } | null;
  setPriceLineHover(index: number, part: number): void;
  setPriceLineDrag(index: number, price: number): void;
  setDraft(
    aTime: number,
    aPrice: number,
    hasB: boolean,
    bTime: number,
    bPrice: number,
    guide: boolean,
    color: number,
    width: number,
    kind: number,
  ): void;
  startDraftStroke(color: number, width: number): void;
  appendDraftPoint(timeMs: number, price: number): void;
  clearDraft(): void;
  setSelectedDrawing(index: number, grabbedEndpoint: number): void;
  moveDrawingEndpoint(index: number, endpoint: number, timeMs: number, price: number): void;
  translateDrawing(index: number, dTimeMs: number, dPrice: number): void;
  hitTestDrawing(x: number, y: number): { index: number; part: number; t: number } | null;
  coordAt(x: number, y: number): Coord | null;
  project(timeMs: number, price: number): { x: number; y: number } | null;
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
  setFloat(key: number, value: number): void {
    this.wc.setFloat(key, value);
  }
  setVisibleRange(startMs: number, endMs: number): void {
    this.wc.setVisibleRange(startMs, endMs);
  }
  setDefaultCandleWidth(px: number): void {
    this.wc.setDefaultCandleWidth(px);
  }
  setChartType(mode: number): void {
    this.wc.setChartType(mode);
  }
  setMorph(collapse: number, fade: number): void {
    this.wc.setMorph(collapse, fade);
  }
  getVisibleRange(): { startMs: number; endMs: number } {
    return this.wc.getVisibleRange();
  }
  resetView(): void {
    this.wc.resetView();
  }
  resetPriceScale(): void {
    this.wc.resetPriceScale();
  }
  getVisiblePriceEnvelope(): { low: number; high: number } | null {
    return this.wc.getVisiblePriceEnvelope();
  }
  preservePriceEnvelope(prevLow: number, prevHigh: number): void {
    this.wc.preservePriceEnvelope(prevLow, prevHigh);
  }
  beginIntervalMorph(): void {
    this.wc.beginIntervalMorph();
  }
  setIntervalMorph(t: number): void {
    this.wc.setIntervalMorph(t);
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
  resizeIndicatorPane(dy: number): void {
    this.wc.resizeIndicatorPane(dy);
  }
  scaleIndicatorAxis(y: number, dy: number): void {
    this.wc.scaleIndicatorAxis(y, dy);
  }
  getAxisMetrics(): AxisMetrics {
    return this.wc.getAxisMetrics();
  }
  setCrosshair(x: number, y: number): void {
    this.wc.setCrosshair(x, y);
  }
  setCrosshairData(timeMs: number, price: number): void {
    this.wc.setCrosshairData(timeMs, price);
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
  setMACD(spec: MACDSpec): void {
    this.wc.setMACD({
      ...spec,
      macdColor: spec.macdColor >>> 0,
      signalColor: spec.signalColor >>> 0,
      histUpColor: spec.histUpColor >>> 0,
      histUpFadingColor: spec.histUpFadingColor >>> 0,
      histDownColor: spec.histDownColor >>> 0,
      histDownFadingColor: spec.histDownFadingColor >>> 0,
      zeroColor: spec.zeroColor >>> 0,
    });
  }
  setOverlays(overlays: OverlaySpec[]): void {
    this.wc.setOverlays(overlays);
  }
  setVWAP(enabled: boolean, resetOffsetMin: number, color: number, width: number): void {
    this.wc.setVWAP(enabled, resetOffsetMin, color >>> 0, width);
  }
  setBollinger(spec: BollingerSpec): void {
    this.wc.setBollinger({
      ...spec,
      upperColor: spec.upperColor >>> 0,
      middleColor: spec.middleColor >>> 0,
      lowerColor: spec.lowerColor >>> 0,
    });
  }
  setVolume(spec: VolumeSpec): void {
    this.wc.setVolume({
      ...spec,
      upColor: spec.upColor >>> 0,
      downColor: spec.downColor >>> 0,
    });
  }
  setVolumeCollapse(t: number, easing: number): void {
    this.wc.setVolumeCollapse(t, easing);
  }
  setDrawings(drawings: DrawingSpec[]): void {
    this.wc.setDrawings(
      drawings.map((d) => ({ ...d, color: d.color >>> 0 })),
    );
  }
  setLiquidity(liquidity: LiquiditySpec): void {
    this.wc.setLiquidity({
      ...liquidity,
      buyColor: liquidity.buyColor >>> 0,
      sellColor: liquidity.sellColor >>> 0,
    });
  }
  setPriceLines(priceLines: PriceLinesSpec): void {
    this.wc.setPriceLines({
      ...priceLines,
      lines: priceLines.lines.map((l) => ({ ...l, color: l.color >>> 0 })),
      bodyBg: priceLines.bodyBg >>> 0,
    });
  }
  hitTestPriceLine(x: number, y: number): { index: number; part: number } | null {
    return this.wc.hitTestPriceLine(x, y);
  }
  setPriceLineHover(index: number, part: number): void {
    this.wc.setPriceLineHover(index, part);
  }
  setPriceLineDrag(index: number, price: number): void {
    this.wc.setPriceLineDrag(index, price);
  }
  setDraft(
    aTime: number,
    aPrice: number,
    hasB: boolean,
    bTime: number,
    bPrice: number,
    guide: boolean,
    color: number,
    width: number,
    kind: number,
  ): void {
    this.wc.setDraft(aTime, aPrice, hasB, bTime, bPrice, guide, color >>> 0, width, kind);
  }
  startDraftStroke(color: number, width: number): void {
    this.wc.startDraftStroke(color >>> 0, width);
  }
  appendDraftPoint(timeMs: number, price: number): void {
    this.wc.appendDraftPoint(timeMs, price);
  }
  clearDraft(): void {
    this.wc.clearDraft();
  }
  setSelectedDrawing(index: number, grabbedEndpoint: number): void {
    this.wc.setSelectedDrawing(index, grabbedEndpoint);
  }
  moveDrawingEndpoint(index: number, endpoint: number, timeMs: number, price: number): void {
    this.wc.moveDrawingEndpoint(index, endpoint, timeMs, price);
  }
  translateDrawing(index: number, dTimeMs: number, dPrice: number): void {
    this.wc.translateDrawing(index, dTimeMs, dPrice);
  }
  hitTestDrawing(x: number, y: number): { index: number; part: number; t: number } | null {
    return this.wc.hitTestDrawing(x, y);
  }
  coordAt(x: number, y: number): Coord | null {
    return this.wc.coordAt(x, y);
  }
  project(timeMs: number, price: number): { x: number; y: number } | null {
    return this.wc.project(timeMs, price);
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
