import type {
  Candle,
  Drawing,
  Footprint,
  LiquidityConfig,
  PriceLine,
  VroomChartProps,
  VroomTheme,
} from '@vroomchart/react';
import { STEP_MS } from './market';

export const INK = '#ECE9E2';
export const GREEN = '#3DDC84';
const GRAY = '#8A8884';
const DIM = '#4A4A4F';

export const SITE_THEME: VroomTheme = {
  background: '#0B0B0C',
  bull: '#D8D6D0',
  bear: DIM,
  wickBull: '#D8D6D0',
  wickBear: '#5A5A5E',
  accentBull: GREEN,
  accentBear: '#6B6B70',
  grid: '#151517',
  axisText: '#6E6C68',
  crosshair: INK,
  crosshairTarget: INK,
  badgeText: '#0B0B0C',
  lineColor: INK,
  axisFontSize: 11,
  candleRadius: 1,
  volumeRadius: 1,
};

const PAPER_THEME: VroomTheme = {
  background: '#F2EFE8',
  bull: '#1A1A1A',
  bear: '#B8B4AA',
  wickBull: '#1A1A1A',
  wickBear: '#B8B4AA',
  accentBull: '#1A1A1A',
  accentBear: '#B8B4AA',
  grid: '#E4E0D7',
  axisText: '#6E6A62',
  crosshair: '#1A1A1A',
  badgeText: '#F2EFE8',
  candleRadius: 3,
  volumeRadius: 3,
};

const TERMINAL_THEME: VroomTheme = {
  ...SITE_THEME,
  lineColor: GREEN,
  lineWidth: 2,
};

export const QUIET_VOLUME = { upColor: '#2A2A2D', downColor: '#1E1E21', opacity: 1, height: 0.16 };

export type SceneId =
  | 'hero'
  | 'speed'
  | 'core'
  | 'indicators'
  | 'draw'
  | 'trades'
  | 'brand'
  | 'agents'
  | 'mobile'
  | 'finale';

/** The window, in bars: `bars` wide, ending `pad` bars past the newest candle. */
export type Frame = { bars: number; pad: number };

/** Mask strength 0..1 over the chart, behind the text. */
export type Scene = { frame: Frame; mask: number; props: Partial<VroomChartProps> };

export const INDICATOR_ROWS = [
  'RSI',
  'MACD',
  'ATR',
  'SMA / EMA',
  'VWAP',
  'Bollinger Bands',
  'Ichimoku Cloud',
  'Fair Value Gaps',
  'Volume',
] as const;

export const BRAND_STEPS = ['Your colors', 'Your light mode', 'Your line chart'] as const;

export const AGENT_STEPS = [
  'set_indicators({ rsi: { period: 14 } })',
  'add_price_level({ price: support, label: "Support" })',
  'add_drawing({ type: "box", from: 40, to: 12 })',
  'set_viewport({ lastBars: 50 })',
] as const;

const tail = (candles: Candle[], n: number) => candles.slice(-n);
const highOf = (cs: Candle[]) => Math.max(...cs.map((c) => c.high));
const lowOf = (cs: Candle[]) => Math.min(...cs.map((c) => c.low));
const at = (candles: Candle[], barsAgo: number) =>
  candles[Math.max(0, candles.length - 1 - barsAgo)];

/** Thin green box around the newest bars, the on-chart viewfinder. */
function viewfinder(candles: Candle[]): Drawing[] {
  const recent = tail(candles, 6);
  const pad = (highOf(recent) - lowOf(recent)) * 0.25 || 1;
  const t0 = recent[0].timeMs - STEP_MS / 2;
  const t1 = recent[recent.length - 1].timeMs + STEP_MS * 1.5;
  const p0 = highOf(recent) + pad;
  const p1 = lowOf(recent) - pad;
  const dt = STEP_MS;
  const dp = (p0 - p1) * 0.14;
  const corners: [number, number, number, number][] = [
    [t0, p0, dt, -dp],
    [t1, p0, -dt, -dp],
    [t0, p1, dt, dp],
    [t1, p1, -dt, dp],
  ];
  return corners.map(([t, p, sx, sy], i) => ({
    id: `viewfinder-${i}`,
    type: 'path',
    color: '#ff3DDC84',
    width: 1.5,
    locked: true,
    points: [
      { timeMs: t + sx, price: p },
      { timeMs: t, price: p },
      { timeMs: t, price: p + sy },
    ],
  }));
}

function markups(candles: Candle[]): { drawings: Drawing[]; priceLines: PriceLine[] } {
  const span = tail(candles, 140);
  const firstHalf = span.slice(0, 70);
  const secondHalf = span.slice(70, 130);
  const a = firstHalf.reduce((m, c) => (c.low < m.low ? c : m), firstHalf[0]);
  const b = secondHalf.reduce((m, c) => (c.low < m.low ? c : m), secondHalf[0]);
  const zone = span.slice(-60, -35);
  const hi = highOf(span);
  const lo = lowOf(span);
  return {
    drawings: [
      {
        id: 'trend',
        type: 'line',
        color: '#ffECE9E2',
        width: 1.5,
        locked: true,
        points: [
          { timeMs: a.timeMs, price: a.low },
          { timeMs: b.timeMs, price: b.low },
        ],
      },
      {
        id: 'zone',
        type: 'box',
        color: '#ff3DDC84',
        fill: '#143DDC84',
        width: 1,
        locked: true,
        points: [
          { timeMs: zone[0].timeMs, price: highOf(zone) },
          { timeMs: zone[zone.length - 1].timeMs, price: lowOf(zone) },
        ],
      },
    ],
    priceLines: [
      { id: 'tp', price: hi + (hi - lo) * 0.08, text: 'TP', quantity: '0.25 BTC', color: GREEN },
      { id: 'sl', price: lo - (hi - lo) * 0.08, text: 'SL', quantity: '0.25 BTC', color: GRAY },
    ],
  };
}

function trades(candles: Candle[]): { footprints: Footprint[]; liquidity: LiquidityConfig } {
  const footprints: Footprint[] = [70, 52, 38, 21, 9].map((barsAgo, i) => {
    const c = at(candles, barsAgo);
    return {
      id: `fill-${i}`,
      timeMs: c.timeMs + STEP_MS / 3,
      side: i % 2 === 0 ? 'buy' : 'sell',
      price: c.close,
    };
  });
  const spot = candles[candles.length - 1].close;
  const span = tail(candles, 120);
  const unit = (highOf(span) - lowOf(span)) / 24 || 1;
  const bands: LiquidityConfig['bands'] = [];
  for (let i = 1; i <= 8; i++) {
    const size = 40 + ((i * 37) % 90);
    bands.push({ side: 'sell', minPrice: spot + unit * i, maxPrice: spot + unit * (i + 0.4), volume: size });
    bands.push({ side: 'buy', minPrice: spot - unit * (i + 0.4), maxPrice: spot - unit * i, volume: size });
  }
  return { footprints, liquidity: { bands, buyColor: GREEN, sellColor: GRAY, maxOpacity: 0.55 } };
}

function indicator(row: number): Partial<VroomChartProps> {
  switch (row) {
    case 0:
      return { rsi: { enabled: true, lineColor: INK, maColor: GREEN, bandColor: '#3A3A3E', extremeFill: false } };
    case 1:
      return { macd: { enabled: true, lineColor: INK, signalColor: GREEN } };
    case 2:
      return { atr: { enabled: true, lineColor: GREEN } };
    case 3:
      return {
        movingAverages: [
          { id: 'ema21', maType: 'ema', period: 21, color: INK, width: 1.25 },
          { id: 'sma50', maType: 'sma', period: 50, color: GREEN, width: 1.25 },
        ],
      };
    case 4:
      return { vwap: { enabled: true, color: GREEN, width: 1.5 } };
    case 5:
      return {
        bollingerBands: {
          enabled: true,
          upperColor: GRAY,
          lowerColor: GRAY,
          middleColor: GREEN,
          fillOpacity: 0.06,
        },
      };
    case 6:
      return {
        ichimoku: {
          enabled: true,
          tenkanColor: INK,
          kijunColor: GRAY,
          senkouAColor: GREEN,
          senkouBColor: '#6B6B70',
          chikouVisible: false,
          bullishCloudColor: GREEN,
          bearishCloudColor: '#6B6B70',
          cloudOpacity: 0.12,
        },
      };
    case 7:
      return { fairValueGaps: { enabled: true, bullishColor: GREEN, bearishColor: GRAY, opacity: 0.14 } };
    default:
      return { volume: { upColor: '#2F6B49', downColor: '#3A3A3E', opacity: 1, height: 0.4 } };
  }
}

/**
 * What the background chart shows for a section. `step` is the sub-step inside
 * tall sections (the active indicator row, theme, or agent call).
 */
export function sceneFor(id: SceneId, step: number, candles: Candle[]): Scene {
  const base: Partial<VroomChartProps> = { theme: SITE_THEME, volume: QUIET_VOLUME, chartType: 'candles' };
  switch (id) {
    case 'hero':
      return { frame: { bars: 110, pad: 8 }, mask: 0.82, props: { ...base, drawings: viewfinder(candles) } };
    case 'speed':
      return { frame: { bars: 110, pad: 8 }, mask: 0.9, props: base };
    case 'core':
      return { frame: { bars: 180, pad: 8 }, mask: 0.86, props: base };
    case 'indicators':
      return {
        frame: { bars: 160, pad: step === 6 ? 30 : 8 },
        mask: 0.9,
        props: { ...base, ...indicator(step) },
      };
    case 'draw':
      return { frame: { bars: 150, pad: 12 }, mask: 0.9, props: { ...base, ...markups(candles) } };
    case 'trades':
      return { frame: { bars: 100, pad: 10 }, mask: 0.9, props: { ...base, ...trades(candles) } };
    case 'brand': {
      if (step === 1) return { frame: { bars: 120, pad: 8 }, mask: 0.9, props: { ...base, theme: PAPER_THEME, volume: { opacity: 0.35 } } };
      if (step === 2) return { frame: { bars: 120, pad: 8 }, mask: 0.9, props: { ...base, theme: TERMINAL_THEME, chartType: 'line' } };
      return { frame: { bars: 120, pad: 8 }, mask: 0.9, props: base };
    }
    case 'agents': {
      const support = lowOf(tail(candles, 80));
      const zone = candles.slice(-40, -12);
      const props: Partial<VroomChartProps> = { ...base, ...indicator(0) };
      if (step >= 1) {
        props.priceLines = [{ id: 'support', price: support, text: 'Support · agent', color: GREEN }];
      }
      if (step >= 2) {
        props.drawings = [
          {
            id: 'agent-zone',
            type: 'box',
            color: '#ff3DDC84',
            fill: '#103DDC84',
            width: 1,
            locked: true,
            label: 'Range',
            meta: { source: 'agent' },
            points: [
              { timeMs: zone[0].timeMs, price: highOf(zone) },
              { timeMs: zone[zone.length - 1].timeMs, price: lowOf(zone) },
            ],
          },
        ];
      }
      return { frame: { bars: step >= 3 ? 50 : 150, pad: 6 }, mask: 0.9, props };
    }
    case 'mobile':
      return { frame: { bars: 220, pad: 8 }, mask: 0.86, props: base };
    case 'finale':
      return { frame: { bars: 130, pad: 10 }, mask: 0.9, props: base };
  }
}
