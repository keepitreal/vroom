// Canvas2D stub of the chart core.
//
// A pure-TS stand-in for the Skia-WASM core so @vroomchart/react can be built and
// run in a browser before the WASM toolchain exists. It implements the same
// VroomChartHandle contract and the same viewport model as the C++ core
// (count-based default window, fixed candle slots, persistent price bounds),
// drawing candles / axes / crosshair with the Canvas2D API.
//
// Intentional gaps vs. the real core: indicators (RSI/MACD/MA/VWAP) and volume
// bars are accepted but not drawn, and there's no label-fade animation. Swapping
// loadVroom() to the WASM module replaces this with pixel-accurate rendering and
// no API change.

import type { Candle } from '@vroomchart/types';
import {
  ColorKey,
  type AxisMetrics,
  type CrosshairCandle,
  type OverlaySpec,
  type VroomChartHandle,
  type VroomModule,
} from '../handle';
import { argbToCss } from '../color';
import { unpackCandles } from '../packCandles';

const DEFAULT_VISIBLE = 80; // matches the core's kDefaultVisible
const Y_AXIS_W = 58;
const X_AXIS_H = 22;
const RIGHT_PAD_CANDLES = 4; // empty future slots past the last candle

// Default theme — mirrors the core's defaults closely enough for the stub.
const DEFAULT_COLORS: Record<number, number> = {
  [ColorKey.Background]: 0xff0d1117,
  [ColorKey.Bull]: 0xff26a69a,
  [ColorKey.Bear]: 0xffef5350,
  [ColorKey.Wick]: 0xff787b86,
  [ColorKey.Grid]: 0xff1f2630,
  [ColorKey.AxisText]: 0xff8b949e,
  [ColorKey.Crosshair]: 0xffc9d1d9,
  [ColorKey.CrosshairTarget]: 0xffc9d1d9,
};

function median(xs: number[]): number {
  if (xs.length === 0) return 0;
  const s = [...xs].sort((a, b) => a - b);
  const mid = s.length >> 1;
  return s.length % 2 ? s[mid]! : (s[mid - 1]! + s[mid]!) / 2;
}

// A "nice" axis step (1/2/5 × 10^n) at or just above `raw`.
function niceStep(raw: number): number {
  if (raw <= 0 || !Number.isFinite(raw)) return 1;
  const mag = Math.pow(10, Math.floor(Math.log10(raw)));
  const norm = raw / mag;
  const step = norm <= 1 ? 1 : norm <= 2 ? 2 : norm <= 5 ? 5 : 10;
  return step * mag;
}

class StubChart implements VroomChartHandle {
  private ctx: CanvasRenderingContext2D;
  private candles: Candle[] = [];
  private durationMs = 60_000;

  private wCss = 0;
  private hCss = 0;
  private dpr = 1;

  private visStart = 0;
  private visEnd = 0;
  private priceMin = 0;
  private priceMax = 1;
  private rangeInit = false;

  private crosshair = false;
  private chX = 0;
  private chY = 0;

  private colors: Record<number, number> = { ...DEFAULT_COLORS };

  constructor(private canvas: HTMLCanvasElement) {
    const ctx = canvas.getContext('2d');
    if (!ctx) throw new Error('VroomChart stub: 2D canvas context unavailable');
    this.ctx = ctx;
  }

  // --- geometry helpers ---------------------------------------------------
  private get plotW() {
    return Math.max(0, this.wCss - Y_AXIS_W);
  }
  private get plotH() {
    return Math.max(0, this.hCss - X_AXIS_H);
  }
  private xOf(timeMs: number) {
    const span = this.visEnd - this.visStart || 1;
    return ((timeMs - this.visStart) / span) * this.plotW;
  }
  private yOf(price: number) {
    const span = this.priceMax - this.priceMin || 1;
    return ((this.priceMax - price) / span) * this.plotH;
  }
  private color(key: number) {
    return argbToCss(this.colors[key] ?? DEFAULT_COLORS[key] ?? 0xff000000);
  }

  // --- data ---------------------------------------------------------------
  setCandles(packed: ArrayBuffer): void {
    this.candles = unpackCandles(packed);
    if (this.candles.length >= 2) {
      const diffs: number[] = [];
      for (let i = 1; i < this.candles.length; i++) {
        diffs.push(this.candles[i]!.timeMs - this.candles[i - 1]!.timeMs);
      }
      const m = median(diffs);
      if (m > 0) this.durationMs = m;
    }
    if (!this.rangeInit) this.resetWindow();
    this.recomputePriceBounds();
  }

  private resetWindow(): void {
    const n = this.candles.length;
    if (n === 0) {
      this.visStart = 0;
      this.visEnd = 1;
      return;
    }
    const last = this.candles[n - 1]!.timeMs;
    this.visEnd = last + this.durationMs * RIGHT_PAD_CANDLES;
    this.visStart = this.visEnd - this.durationMs * (DEFAULT_VISIBLE + RIGHT_PAD_CANDLES);
    this.rangeInit = true;
  }

  private recomputePriceBounds(): void {
    let lo = Infinity;
    let hi = -Infinity;
    for (const c of this.candles) {
      if (c.timeMs < this.visStart || c.timeMs > this.visEnd) continue;
      if (c.low < lo) lo = c.low;
      if (c.high > hi) hi = c.high;
    }
    if (!Number.isFinite(lo) || !Number.isFinite(hi) || hi <= lo) {
      // No visible candles (or flat): keep prior bounds if sensible.
      if (this.priceMax > this.priceMin) return;
      lo = 0;
      hi = 1;
    }
    const pad = (hi - lo) * 0.08;
    this.priceMin = lo - pad;
    this.priceMax = hi + pad;
  }

  // --- sizing -------------------------------------------------------------
  setSize(width: number, height: number, dpr: number): void {
    this.wCss = width;
    this.hCss = height;
    this.dpr = dpr || 1;
    this.canvas.width = Math.max(1, Math.round(width * this.dpr));
    this.canvas.height = Math.max(1, Math.round(height * this.dpr));
    this.ctx.setTransform(this.dpr, 0, 0, this.dpr, 0, 0);
  }

  setColor(key: number, argb: number): void {
    this.colors[key] = argb >>> 0;
  }

  setVisibleRange(startMs: number, endMs: number): void {
    if (startMs === 0 && endMs === 0) {
      this.rangeInit = false;
      this.resetWindow();
    } else {
      this.visStart = startMs;
      this.visEnd = endMs;
      this.rangeInit = true;
    }
    this.recomputePriceBounds();
  }

  // --- viewport mutators --------------------------------------------------
  pan(dx: number, dy: number): void {
    this.translate(dx, dy);
  }

  translate(dx: number, dy: number): void {
    const span = this.visEnd - this.visStart;
    const dt = (dx / (this.plotW || 1)) * span;
    this.visStart -= dt;
    this.visEnd -= dt;
    if (dy) {
      const pspan = this.priceMax - this.priceMin;
      const dp = (dy / (this.plotH || 1)) * pspan;
      this.priceMin += dp;
      this.priceMax += dp;
    }
  }

  zoom(scaleX: number, scaleY: number, fx: number, fy: number): void {
    if (scaleX !== 1) {
      const span = this.visEnd - this.visStart;
      const focalT = this.visStart + (fx / (this.plotW || 1)) * span;
      const newSpan = span / scaleX;
      this.visStart = focalT - (fx / (this.plotW || 1)) * newSpan;
      this.visEnd = this.visStart + newSpan;
    }
    if (scaleY !== 1) {
      const pspan = this.priceMax - this.priceMin;
      const focalP = this.priceMax - (fy / (this.plotH || 1)) * pspan;
      const newSpan = pspan / scaleY;
      this.priceMax = focalP + (fy / (this.plotH || 1)) * newSpan;
      this.priceMin = this.priceMax - newSpan;
    }
  }

  scalePriceAxis(dy: number): void {
    const factor = Math.max(0.1, 1 + dy / (this.plotH || 1));
    const center = (this.priceMin + this.priceMax) / 2;
    const half = ((this.priceMax - this.priceMin) / 2) * factor;
    this.priceMin = center - half;
    this.priceMax = center + half;
  }

  scaleTimeAxis(dx: number): void {
    const factor = Math.max(0.1, 1 + dx / (this.plotW || 1));
    const span = (this.visEnd - this.visStart) * factor;
    this.visStart = this.visEnd - span;
  }

  getAxisMetrics(): AxisMetrics {
    return { yAxisWidth: Y_AXIS_W, xAxisHeight: X_AXIS_H, indicatorHeight: 0 };
  }

  // --- crosshair ----------------------------------------------------------
  setCrosshair(x: number, y: number): void {
    this.crosshair = true;
    this.chX = x;
    this.chY = y;
  }
  clearCrosshair(): void {
    this.crosshair = false;
  }
  private snapIndex(): number {
    if (this.candles.length === 0) return -1;
    const span = this.visEnd - this.visStart || 1;
    const t = this.visStart + (this.chX / (this.plotW || 1)) * span;
    let best = -1;
    let bestD = Infinity;
    for (let i = 0; i < this.candles.length; i++) {
      const d = Math.abs(this.candles[i]!.timeMs - t);
      if (d < bestD) {
        bestD = d;
        best = i;
      }
    }
    return best;
  }
  getCrosshairCandle(): CrosshairCandle | null {
    if (!this.crosshair) return null;
    const i = this.snapIndex();
    if (i < 0) return null;
    const c = this.candles[i]!;
    return { ...c };
  }

  // --- indicators (accepted, not drawn in the stub) -----------------------
  setRSI(): void {}
  setMACD(): void {}
  setOverlays(_overlays: OverlaySpec[]): void {}
  setVWAP(): void {}

  isAnimating(): boolean {
    return false;
  }

  // --- rendering ----------------------------------------------------------
  present(): void {
    const ctx = this.ctx;
    const w = this.wCss;
    const h = this.hCss;
    if (w <= 0 || h <= 0) return;

    ctx.clearRect(0, 0, w, h);
    ctx.fillStyle = this.color(ColorKey.Background);
    ctx.fillRect(0, 0, w, h);

    this.drawPriceAxis();
    this.drawTimeAxis();
    this.drawCandles();
    if (this.crosshair) this.drawCrosshair();
  }

  private priceTicks(): number[] {
    const span = this.priceMax - this.priceMin;
    if (span <= 0) return [];
    const step = niceStep(span / 5);
    const first = Math.ceil(this.priceMin / step) * step;
    const out: number[] = [];
    for (let p = first; p <= this.priceMax; p += step) out.push(p);
    return out;
  }

  private fmtPrice(p: number): string {
    const span = this.priceMax - this.priceMin;
    const decimals = span < 1 ? 4 : span < 100 ? 2 : 0;
    return p.toFixed(decimals);
  }

  private drawPriceAxis(): void {
    const ctx = this.ctx;
    const plotW = this.plotW;
    ctx.lineWidth = 1;
    ctx.strokeStyle = this.color(ColorKey.Grid);
    ctx.fillStyle = this.color(ColorKey.AxisText);
    ctx.font = '11px -apple-system, system-ui, "Segoe UI", sans-serif';
    ctx.textAlign = 'left';
    ctx.textBaseline = 'middle';
    for (const p of this.priceTicks()) {
      const y = this.yOf(p);
      if (y < 0 || y > this.plotH) continue;
      ctx.beginPath();
      ctx.moveTo(0, Math.round(y) + 0.5);
      ctx.lineTo(plotW, Math.round(y) + 0.5);
      ctx.stroke();
      ctx.fillText(this.fmtPrice(p), plotW + 6, y);
    }
  }

  private fmtTime(timeMs: number): string {
    const d = new Date(timeMs);
    const span = this.visEnd - this.visStart;
    const twoDays = 2 * 24 * 3600 * 1000;
    const pad = (n: number) => String(n).padStart(2, '0');
    if (span < twoDays) return `${pad(d.getHours())}:${pad(d.getMinutes())}`;
    return `${d.getMonth() + 1}/${d.getDate()}`;
  }

  private drawTimeAxis(): void {
    const ctx = this.ctx;
    const span = this.visEnd - this.visStart;
    if (span <= 0) return;
    const targetPx = 80;
    const stepMs = niceStep((span / this.plotW) * targetPx);
    const first = Math.ceil(this.visStart / stepMs) * stepMs;
    ctx.fillStyle = this.color(ColorKey.AxisText);
    ctx.font = '11px -apple-system, system-ui, "Segoe UI", sans-serif';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    const y = this.plotH + X_AXIS_H / 2;
    for (let t = first; t <= this.visEnd; t += stepMs) {
      const x = this.xOf(t);
      if (x < 0 || x > this.plotW) continue;
      ctx.fillText(this.fmtTime(t), x, y);
    }
  }

  private drawCandles(): void {
    const ctx = this.ctx;
    const span = this.visEnd - this.visStart || 1;
    const slotW = (this.durationMs / span) * this.plotW;
    const bodyW = Math.max(1, slotW * 0.7);
    const bull = this.color(ColorKey.Bull);
    const bear = this.color(ColorKey.Bear);
    const wick = this.color(ColorKey.Wick);

    for (const c of this.candles) {
      const x = this.xOf(c.timeMs);
      if (x < -slotW || x > this.plotW + slotW) continue;
      const up = c.close >= c.open;
      const yHigh = this.yOf(c.high);
      const yLow = this.yOf(c.low);
      const yOpen = this.yOf(c.open);
      const yClose = this.yOf(c.close);

      // wick
      ctx.strokeStyle = wick;
      ctx.lineWidth = 1;
      ctx.beginPath();
      ctx.moveTo(Math.round(x) + 0.5, yHigh);
      ctx.lineTo(Math.round(x) + 0.5, yLow);
      ctx.stroke();

      // body
      ctx.fillStyle = up ? bull : bear;
      const top = Math.min(yOpen, yClose);
      const bodyH = Math.max(1, Math.abs(yClose - yOpen));
      ctx.fillRect(x - bodyW / 2, top, bodyW, bodyH);
    }
  }

  private drawCrosshair(): void {
    const ctx = this.ctx;
    const i = this.snapIndex();
    if (i < 0) return;
    const c = this.candles[i]!;
    const x = this.xOf(c.timeMs);
    const y = Math.max(0, Math.min(this.plotH, this.chY));

    ctx.save();
    ctx.strokeStyle = this.color(ColorKey.Crosshair);
    ctx.lineWidth = 1;
    ctx.setLineDash([4, 4]);
    ctx.beginPath();
    ctx.moveTo(Math.round(x) + 0.5, 0);
    ctx.lineTo(Math.round(x) + 0.5, this.plotH);
    ctx.moveTo(0, Math.round(y) + 0.5);
    ctx.lineTo(this.plotW, Math.round(y) + 0.5);
    ctx.stroke();
    ctx.setLineDash([]);

    // target ring at the close
    const ty = this.yOf(c.close);
    ctx.strokeStyle = this.color(ColorKey.CrosshairTarget);
    ctx.beginPath();
    ctx.arc(x, ty, 3.5, 0, Math.PI * 2);
    ctx.stroke();
    ctx.restore();
  }

  destroy(): void {
    this.candles = [];
  }
}

export function createStubModule(): VroomModule {
  return {
    create(canvas: HTMLCanvasElement): VroomChartHandle {
      return new StubChart(canvas);
    },
  };
}
