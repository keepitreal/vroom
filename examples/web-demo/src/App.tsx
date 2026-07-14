import { type CSSProperties, useCallback, useEffect, useMemo, useState } from 'react';
import {
  VroomChart,
  type Candle,
  type CrosshairEvent,
  type ChartMode,
  type DrawTool,
  type Drawing,
  type LiquidityBand,
  type LiquidityConfig,
} from '@vroomchart/react';
import { StreamingRepro } from './StreamingRepro';
import { SettingsModal, DEFAULT_THEME, type ThemeState } from './SettingsModal';
import {
  IndicatorsModal,
  DEFAULT_INDICATOR_STATE,
  DEFAULT_RSI_PARAMS,
  DEFAULT_MACD_PARAMS,
  DEFAULT_MA_LINE,
  DEFAULT_EMA_LINE,
  DEFAULT_VWAP_PARAMS,
  deriveIndicatorProps,
  enabledCount,
  type IndicatorId,
  type IndicatorState,
  type MACDParams,
  type MALineParams,
  type RSIParams,
  type VWAPParams,
} from './IndicatorsModal';

const THEME_STORAGE_KEY = 'vroom-theme';
const CANDLE_WIDTH_KEY = 'vroom-candle-width';

// Persisted default candle body width (px) driving the demo chart's initial
// zoom. Falls back to a default on missing/corrupt data.
function loadCandleWidth(): number {
  const DEFAULT = 8;
  if (typeof window === 'undefined') return DEFAULT;
  try {
    const raw = window.localStorage.getItem(CANDLE_WIDTH_KEY);
    if (!raw) return DEFAULT;
    const n = Number(raw);
    return Number.isFinite(n) && n > 0 ? n : DEFAULT;
  } catch {
    return DEFAULT;
  }
}

// Drawing-tool props shared by both chart views (the line tool toggled in the
// top bar). Bundled so each view just spreads them onto its <VroomChart>.
export type DrawProps = {
  mode: ChartMode;
  tool: DrawTool;
  drawings: Drawing[];
  onDrawingComplete: (d: Drawing) => void;
  onModeChange: (m: ChartMode) => void;
};

// Read the saved theme, merging onto DEFAULT_THEME so newly-added fields are
// always present even if an older payload was stored. Falls back to defaults
// on missing/corrupt data.
function loadTheme(): ThemeState {
  if (typeof window === 'undefined') return DEFAULT_THEME;
  try {
    const raw = window.localStorage.getItem(THEME_STORAGE_KEY);
    if (!raw) return DEFAULT_THEME;
    const parsed = JSON.parse(raw) as Partial<ThemeState>;
    return { ...DEFAULT_THEME, ...parsed };
  } catch {
    return DEFAULT_THEME;
  }
}

// ---- Demo data: per-asset seeded 1m base series + timeframe aggregation ----
// Both timeframes of one asset aggregate the same 1m walk, so a timeframe
// switch has genuine price/time continuity (exercising the chart's heuristic
// detection, not just the seriesKey escape hatch).

const MINUTE = 60 * 1000;
const HOUR = 60 * MINUTE;

const ASSETS = { BTC: 60_000, SOL: 80 } as const;
type Asset = keyof typeof ASSETS;

const TIMEFRAMES = [
  { label: '1m', stepMs: MINUTE },
  { label: '5m', stepMs: 5 * MINUTE },
  { label: '15m', stepMs: 15 * MINUTE },
  { label: '1h', stepMs: HOUR },
] as const;

// Deterministic PRNG so each asset renders the same walk across switches.
function mulberry32(seed: number): () => number {
  return () => {
    seed |= 0;
    seed = (seed + 0x6d2b79f5) | 0;
    let t = Math.imul(seed ^ (seed >>> 15), 1 | seed);
    t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
}

const BASE_BARS = 12_000; // 1m bars -> 200 bars even at the 1h timeframe

const baseCache = new Map<Asset, Candle[]>();
function baseSeries(asset: Asset): Candle[] {
  const cached = baseCache.get(asset);
  if (cached) return cached;
  const seed = [...asset].reduce((a, ch) => a * 31 + ch.charCodeAt(0), 7);
  const rand = mulberry32(seed);
  const basePrice = ASSETS[asset];
  // Anchor to the top of the hour so every timeframe buckets cleanly.
  const endMs = Math.floor(Date.now() / HOUR) * HOUR;
  const out: Candle[] = [];
  let price: number = basePrice;
  for (let i = 0; i < BASE_BARS; i++) {
    const open = price;
    const close = open + (rand() - 0.5) * basePrice * 0.002;
    const high = Math.max(open, close) + rand() * basePrice * 0.001;
    const low = Math.min(open, close) - rand() * basePrice * 0.001;
    out.push({
      timeMs: endMs - (BASE_BARS - i) * MINUTE,
      open,
      high,
      low,
      close,
      volume: rand() * 1000,
    });
    price = close;
  }
  baseCache.set(asset, out);
  return out;
}

// Bucket 1m bars into a coarser timeframe (first open, max high, min low,
// last close, summed volume).
function aggregate(base: Candle[], stepMs: number): Candle[] {
  if (stepMs <= MINUTE) return base;
  const out: Candle[] = [];
  for (const c of base) {
    const bucket = Math.floor(c.timeMs / stepMs) * stepMs;
    const last = out[out.length - 1];
    if (last && last.timeMs === bucket) {
      last.high = Math.max(last.high, c.high);
      last.low = Math.min(last.low, c.low);
      last.close = c.close;
      last.volume += c.volume;
    } else {
      out.push({ ...c, timeMs: bucket });
    }
  }
  return out;
}

// Streaming mutations for the demo/sync views. Volatility scales with price
// (matching baseSeries) so BTC and SOL both move realistically. Non-deterministic
// (Math.random) since these fire on live user interaction, not initial framing.
function appendCandle(prev: Candle[], stepMs: number): Candle[] {
  if (prev.length === 0) return prev;
  const last = prev[prev.length - 1];
  const vol = last.close * 0.002;
  const open = last.close;
  const close = open + (Math.random() - 0.5) * 2 * vol;
  const high = Math.max(open, close) + Math.random() * vol;
  const low = Math.min(open, close) - Math.random() * vol;
  return [
    ...prev,
    { timeMs: last.timeMs + stepMs, open, high, low, close, volume: Math.random() * 1000 },
  ];
}

// Mutate the most recent bar in place (simulates an in-progress candle ticking).
function updateLast(prev: Candle[]): Candle[] {
  if (prev.length === 0) return prev;
  const last = prev[prev.length - 1];
  const vol = last.open * 0.002;
  const close = last.open + (Math.random() - 0.5) * 2 * vol;
  const high = Math.max(last.high, last.open, close);
  const low = Math.min(last.low, last.open, close);
  return [...prev.slice(0, -1), { ...last, close, high, low }];
}

// Drop a few runs of interior bars to simulate downtime / illiquid gaps — a
// NON-uniform bar grid. The last bar is left intact (splices run high->low so
// earlier indices don't shift). The "Gaps" toggle uses this to reproduce the
// bug where a gappy series made an in-place tick misclassify as a viewport
// reset: pan a chart, enable Gaps, then Add/Update — the pan must hold.
function punchGaps(c: Candle[]): Candle[] {
  const out = c.slice();
  for (const frac of [0.8, 0.55, 0.3]) out.splice(Math.floor(out.length * frac), 3);
  return out;
}

// A simulated L2 order book around the latest close: buy bands stepping below
// spot, sell bands above, each a thin price interval. Resting size tapers with
// distance from spot (denser near the mid) plus a couple of prominent "walls"
// (big resting orders) that drift over time, so the volume-driven opacity has
// obvious variation. `tick` advances on an interval while the overlay is live,
// animating the walls; the whole book re-anchors to `spot` as price streams.
// Deterministic given (spot, tick) so React re-renders don't jitter it.
// `heightFrac` scales each band's thickness relative to the level spacing:
// <1 leaves gaps, 1 makes adjacent bands touch, >1 overlaps them into a wall.
function makeBands(candles: Candle[], tick = 0, heightFrac = 0.85): LiquidityConfig {
  const bands: LiquidityBand[] = [];
  if (candles.length === 0) return { bands };
  const spot = candles[candles.length - 1].close;
  const step = spot * 0.0009;
  const half = (step * heightFrac) / 2; // each band centered on its level line
  const levels = 28;
  const noise = (n: number) => {
    const x = Math.sin(n) * 43758.5453;
    return x - Math.floor(x);
  };
  // Two walls per side that slowly walk toward/away from the mid as `tick` grows.
  const wall = (seed: number) => 3 + Math.floor((tick * 0.5 + seed * 7) % (levels - 6));
  const sellWalls = new Set([wall(1), wall(4)]);
  const buyWalls = new Set([wall(2), wall(6)]);
  for (let i = 1; i <= levels; i++) {
    const taper = 1 - i / (levels + 4); // more resting size near spot
    const shimmer = 0.4 + 0.6 * noise(i * 3.1 + tick * 0.7); // gentle live wobble
    const sMid = spot + i * step;
    bands.push({
      minPrice: sMid - half,
      maxPrice: sMid + half,
      side: 'sell',
      volume: Math.max(0.05, taper * shimmer * (sellWalls.has(i) ? 4 : 1)),
    });
    const bMid = spot - i * step;
    const bShimmer = 0.4 + 0.6 * noise(i * 5.7 + tick * 0.7);
    bands.push({
      minPrice: bMid - half,
      maxPrice: bMid + half,
      side: 'buy',
      volume: Math.max(0.05, taper * bShimmer * (buyWalls.has(i) ? 4 : 1)),
    });
  }
  return { bands, buyColor: '#26a69a', sellColor: '#ef5350' };
}

// Shared toolbar button style.
const toolBtn: CSSProperties = {
  background: 'transparent',
  color: '#c9d1d9',
  border: '1px solid #30363d',
  borderRadius: 6,
  padding: '4px 10px',
  fontSize: 13,
  cursor: 'pointer',
};

export function App() {
  // No wasm/asset config needed — @vroomchart/react uses the Skia-WASM core
  // bundled in @vroomchart/core-wasm.
  const [asset, setAsset] = useState<Asset>('BTC');
  const [tf, setTf] = useState<number>(MINUTE);
  const [useSeriesKey, setUseSeriesKey] = useState(true);
  const [gaps, setGaps] = useState(false);
  const [showLiquidity, setShowLiquidity] = useState(false);
  // Demo candles are stateful so the Add/Update tools can stream into them; they
  // reset to the base series whenever the asset/timeframe (or Gaps) changes.
  const demoBase = useMemo(() => {
    const base = aggregate(baseSeries(asset), tf);
    return gaps ? punchGaps(base) : base;
  }, [asset, tf, gaps]);
  const [candles, setCandles] = useState<Candle[]>(demoBase);
  useEffect(() => {
    setCandles(demoBase);
  }, [demoBase]);
  // Simulated live order book. `liqTick` advances on an interval while the
  // overlay is on, animating the walls; the book re-anchors to the latest close.
  const [liqTick, setLiqTick] = useState(0);
  const [bandHeight, setBandHeight] = useState(0.85);
  // Default candle body width (px) driving the demo chart's initial zoom,
  // persisted to localStorage so it applies on first load.
  const [candleWidth, setCandleWidth] = useState(loadCandleWidth);
  useEffect(() => {
    if (!showLiquidity) return;
    const id = setInterval(() => setLiqTick((t) => t + 1), 700);
    return () => clearInterval(id);
  }, [showLiquidity]);
  const demoLiquidity = useMemo(
    () => makeBands(candles, liqTick, bandHeight),
    [candles, liqTick, bandHeight],
  );
  const [readout, setReadout] = useState<string>('hover / long-press for crosshair');
  const [view, setView] = useState<'repro' | 'demo' | 'sync'>('repro');

  // Sync view: two charts of the same asset (comparable price scale so the
  // synced horizontal line stays on-screen) sharing one crosshair. Each chart
  // both emits its crosshair (onCrosshair) and mirrors the shared one
  // (crosshairOverride); the library ignores the override on whichever chart is
  // actively hovered, so no host-side source tracking is needed.
  const [syncTop, setSyncTop] = useState<Candle[]>(() => aggregate(baseSeries('BTC'), MINUTE));
  const [syncBottom, setSyncBottom] = useState<Candle[]>(() => aggregate(baseSeries('BTC'), 15 * MINUTE));
  const [xhair, setXhair] = useState<{ timeMs: number; price: number } | null>(null);
  const onSyncCrosshair = useCallback((e: CrosshairEvent) => {
    setXhair(e.active && e.timeMs != null && e.price != null ? { timeMs: e.timeMs, price: e.price } : null);
  }, []);

  // Streaming tools shared by the demo and sync views. In sync, both charts get
  // a bar on each click (each at its own timeframe step) to show streaming and
  // crosshair sync coexisting; in demo, the single series streams at `tf`.
  const onAddCandle = useCallback(() => {
    if (view === 'sync') {
      setSyncTop((p) => appendCandle(p, MINUTE));
      setSyncBottom((p) => appendCandle(p, 15 * MINUTE));
    } else {
      setCandles((p) => appendCandle(p, tf));
    }
  }, [view, tf]);
  const onUpdateLast = useCallback(() => {
    if (view === 'sync') {
      setSyncTop(updateLast);
      setSyncBottom(updateLast);
    } else {
      setCandles(updateLast);
    }
  }, [view]);
  const [theme, setTheme] = useState<ThemeState>(loadTheme);
  const [settingsOpen, setSettingsOpen] = useState(false);

  // Drawing tool state. `drawMode`/`drawTool` drive the chart; `drawings` is the
  // controlled list the chart appends to via onDrawingComplete.
  const [drawMode, setDrawMode] = useState<ChartMode>('pan');
  const [drawTool, setDrawTool] = useState<DrawTool>(null);
  const [drawings, setDrawings] = useState<Drawing[]>([]);

  const toggleLineTool = useCallback(() => {
    setDrawMode((m) => {
      const next = m === 'draw' ? 'pan' : 'draw';
      setDrawTool(next === 'draw' ? 'line' : null);
      return next;
    });
  }, []);

  const drawProps: DrawProps = {
    mode: drawMode,
    tool: drawTool,
    drawings,
    onDrawingComplete: (d) => setDrawings((p) => [...p, d]),
    // The chart asks to return to pan after the user clicks away from a line.
    onModeChange: (m) => {
      setDrawMode(m);
      if (m === 'pan') setDrawTool(null);
    },
  };

  useEffect(() => {
    try {
      window.localStorage.setItem(THEME_STORAGE_KEY, JSON.stringify(theme));
    } catch {
      // Ignore quota / private-mode write failures; persistence is best-effort.
    }
  }, [theme]);

  useEffect(() => {
    if (!(candleWidth > 0)) return;
    try {
      window.localStorage.setItem(CANDLE_WIDTH_KEY, String(candleWidth));
    } catch {
      // best-effort
    }
  }, [candleWidth]);

  // Indicator enable/config state lives here so it drives both chart views.
  const [indicatorsOpen, setIndicatorsOpen] = useState(false);
  const [indicators, setIndicators] = useState<IndicatorState>(
    DEFAULT_INDICATOR_STATE,
  );
  const [rsiParams, setRsiParams] = useState<RSIParams>(DEFAULT_RSI_PARAMS);
  const [macdParams, setMacdParams] = useState<MACDParams>(DEFAULT_MACD_PARAMS);
  const [maLines, setMaLines] = useState<MALineParams[]>([DEFAULT_MA_LINE]);
  const [emaLines, setEmaLines] = useState<MALineParams[]>([DEFAULT_EMA_LINE]);
  const [vwapParams, setVwapParams] = useState<VWAPParams>(DEFAULT_VWAP_PARAMS);

  const patchRsi = useCallback(
    (patch: Partial<RSIParams>) => setRsiParams((p) => ({ ...p, ...patch })),
    [],
  );
  const patchMacd = useCallback(
    (patch: Partial<MACDParams>) => setMacdParams((p) => ({ ...p, ...patch })),
    [],
  );
  const patchVwap = useCallback(
    (patch: Partial<VWAPParams>) => setVwapParams((p) => ({ ...p, ...patch })),
    [],
  );

  const maEditor = {
    lines: maLines,
    onChange: (i: number, patch: Partial<MALineParams>) =>
      setMaLines((p) => p.map((l, idx) => (idx === i ? { ...l, ...patch } : l))),
    onAdd: () => setMaLines((p) => [...p, DEFAULT_MA_LINE]),
    onRemove: (i: number) => setMaLines((p) => p.filter((_, idx) => idx !== i)),
  };
  const emaEditor = {
    lines: emaLines,
    onChange: (i: number, patch: Partial<MALineParams>) =>
      setEmaLines((p) => p.map((l, idx) => (idx === i ? { ...l, ...patch } : l))),
    onAdd: () => setEmaLines((p) => [...p, DEFAULT_EMA_LINE]),
    onRemove: (i: number) => setEmaLines((p) => p.filter((_, idx) => idx !== i)),
  };

  const toggleIndicator = useCallback((id: IndicatorId, enabled: boolean) => {
    setIndicators((prev) => ({ ...prev, [id]: { ...prev[id], enabled } }));
    if (enabled && id === 'ma') setMaLines((p) => (p.length ? p : [DEFAULT_MA_LINE]));
    if (enabled && id === 'ema') setEmaLines((p) => (p.length ? p : [DEFAULT_EMA_LINE]));
  }, []);

  const indicatorProps = deriveIndicatorProps(
    indicators,
    rsiParams,
    macdParams,
    maLines,
    emaLines,
    vwapParams,
  );
  const activeCount = enabledCount(indicators);

  const onCrosshair = (e: CrosshairEvent) => {
    if (!e.active || !e.candle) {
      setReadout('hover / long-press for crosshair');
      return;
    }
    const c = e.candle;
    const d = new Date(c.timeMs).toISOString().slice(0, 10);
    setReadout(`${d}  O ${c.open.toFixed(2)}  H ${c.high.toFixed(2)}  L ${c.low.toFixed(2)}  C ${c.close.toFixed(2)}`);
  };

  return (
    <div style={{ height: '100%', display: 'flex', flexDirection: 'column', color: '#c9d1d9', fontFamily: 'system-ui, sans-serif' }}>
      <div style={{ padding: '10px 14px', display: 'flex', gap: 16, alignItems: 'center' }}>
        <strong style={{ fontSize: 16 }}>Vroom</strong>
        <div style={{ display: 'flex', gap: 4 }}>
          {(['repro', 'demo', 'sync'] as const).map((v) => (
            <button
              key={v}
              onClick={() => setView(v)}
              style={{
                background: view === v ? '#21262d' : 'transparent',
                color: '#c9d1d9',
                border: '1px solid #30363d',
                borderRadius: 6,
                padding: '4px 10px',
                fontSize: 13,
                cursor: 'pointer',
              }}
            >
              {v === 'repro' ? 'Repro' : v === 'demo' ? 'Demo' : 'Sync'}
            </button>
          ))}
        </div>
        {view === 'demo' && (
          <>
            <div style={{ display: 'flex', gap: 4 }}>
              {(Object.keys(ASSETS) as Asset[]).map((a) => (
                <button
                  key={a}
                  onClick={() => setAsset(a)}
                  style={{
                    background: asset === a ? '#21262d' : 'transparent',
                    color: '#c9d1d9',
                    border: '1px solid #30363d',
                    borderRadius: 6,
                    padding: '4px 10px',
                    fontSize: 13,
                    cursor: 'pointer',
                  }}
                >
                  {a}
                </button>
              ))}
            </div>
            <div style={{ display: 'flex', gap: 4 }}>
              {TIMEFRAMES.map((t) => (
                <button
                  key={t.label}
                  onClick={() => setTf(t.stepMs)}
                  style={{
                    background: tf === t.stepMs ? '#21262d' : 'transparent',
                    color: '#c9d1d9',
                    border: '1px solid #30363d',
                    borderRadius: 6,
                    padding: '4px 10px',
                    fontSize: 13,
                    cursor: 'pointer',
                  }}
                >
                  {t.label}
                </button>
              ))}
            </div>
            <label
              style={{ display: 'flex', alignItems: 'center', gap: 5, fontSize: 12, opacity: 0.8, cursor: 'pointer' }}
              title="Pass seriesKey={asset} so asset switches reset explicitly; uncheck to exercise pure data-heuristic detection"
            >
              <input
                type="checkbox"
                checked={useSeriesKey}
                onChange={(e) => setUseSeriesKey(e.target.checked)}
              />
              seriesKey
            </label>
            <label
              style={{ display: 'flex', alignItems: 'center', gap: 5, fontSize: 12, opacity: 0.8, cursor: 'pointer' }}
              title="Punch interior holes into the series (non-uniform grid). Pan the chart, then Add/Update a candle — the pan must hold (regression test for gappy-series streaming)."
            >
              <input type="checkbox" checked={gaps} onChange={(e) => setGaps(e.target.checked)} />
              gaps
            </label>
            <label
              style={{ display: 'flex', alignItems: 'center', gap: 5, fontSize: 12, opacity: 0.8, cursor: 'pointer' }}
              title="Overlay sample resting-order liquidity bands (buy below / sell above spot); opacity scales with volume."
            >
              <input
                type="checkbox"
                checked={showLiquidity}
                onChange={(e) => setShowLiquidity(e.target.checked)}
              />
              liquidity
            </label>
            {showLiquidity && (
              <label
                style={{ display: 'flex', alignItems: 'center', gap: 6, fontSize: 12, opacity: 0.8 }}
                title="Band height as a fraction of the level spacing (<1 gaps, 1 touching, >1 overlapping)."
              >
                height
                <input
                  type="range"
                  min={0.1}
                  max={3}
                  step={0.05}
                  value={bandHeight}
                  onChange={(e) => setBandHeight(Number(e.target.value))}
                  style={{ width: 90 }}
                />
                <span style={{ fontFamily: 'ui-monospace, monospace', width: 30 }}>
                  {bandHeight.toFixed(2)}
                </span>
              </label>
            )}
            <label
              style={{ display: 'flex', alignItems: 'center', gap: 6, fontSize: 12, opacity: 0.8 }}
              title="Default candle body width (px) for the initial zoom. Persisted to localStorage; larger = more zoomed in. Editing remounts the chart to re-frame."
            >
              candle&nbsp;px
              <input
                type="number"
                min={2}
                max={40}
                step={1}
                value={candleWidth}
                onChange={(e) => setCandleWidth(Number(e.target.value))}
                style={{ width: 56 }}
              />
            </label>
            <span style={{ fontFamily: 'ui-monospace, monospace', fontSize: 13, opacity: 0.85 }}>{readout}</span>
          </>
        )}
        {(view === 'demo' || view === 'sync') && (
          <div style={{ display: 'flex', gap: 6 }}>
            <button onClick={onAddCandle} style={toolBtn}>
              Add candle
            </button>
            <button onClick={onUpdateLast} style={toolBtn}>
              Update last candle
            </button>
          </div>
        )}
        <div style={{ marginLeft: 'auto', display: 'flex', gap: 6 }}>
          <button
            onClick={toggleLineTool}
            style={{
              background: drawMode === 'draw' ? '#1f6feb' : 'transparent',
              color: drawMode === 'draw' ? '#f0f6fc' : '#c9d1d9',
              border: '1px solid #30363d',
              borderRadius: 6,
              padding: '4px 10px',
              fontSize: 13,
              cursor: 'pointer',
            }}
          >
            Line
          </button>
          <button
            onClick={() => setIndicatorsOpen(true)}
            style={{
              display: 'flex',
              alignItems: 'center',
              gap: 6,
              background: 'transparent',
              color: '#c9d1d9',
              border: '1px solid #30363d',
              borderRadius: 6,
              padding: '4px 10px',
              fontSize: 13,
              cursor: 'pointer',
            }}
          >
            Indicators
            {activeCount > 0 && (
              <span
                style={{
                  background: '#238636',
                  color: '#f0f6fc',
                  borderRadius: 10,
                  fontSize: 11,
                  fontWeight: 700,
                  padding: '0 6px',
                  lineHeight: '16px',
                }}
              >
                {activeCount}
              </span>
            )}
          </button>
          <button
            onClick={() => setSettingsOpen(true)}
            style={{
              background: 'transparent',
              color: '#c9d1d9',
              border: '1px solid #30363d',
              borderRadius: 6,
              padding: '4px 10px',
              fontSize: 13,
              cursor: 'pointer',
            }}
          >
            Settings
          </button>
        </div>
      </div>
      {view === 'repro' ? (
        <div style={{ flex: 1, minHeight: 0 }}>
          <StreamingRepro theme={theme} indicators={indicatorProps} draw={drawProps} />
        </div>
      ) : view === 'sync' ? (
        <div style={{ flex: 1, minHeight: 0, display: 'flex', flexDirection: 'row', gap: 8, padding: '0 8px 8px' }}>
          <div style={{ flex: 1, minWidth: 0, minHeight: 0 }}>
            <VroomChart
              candles={syncTop}
              seriesKey="BTC-1m"
              theme={theme}
              onCrosshair={onSyncCrosshair}
              crosshairOverride={xhair}
            />
          </div>
          <div style={{ flex: 1, minWidth: 0, minHeight: 0 }}>
            <VroomChart
              candles={syncBottom}
              seriesKey="BTC-15m"
              theme={theme}
              onCrosshair={onSyncCrosshair}
              crosshairOverride={xhair}
            />
          </div>
        </div>
      ) : (
        <div style={{ flex: 1, minHeight: 0, padding: '0 8px 8px' }}>
          <VroomChart
            // Remount on width change so the new default framing applies (it only
            // takes effect on a fresh handle — mirrors a real "first load").
            key={candleWidth}
            candles={candles}
            seriesKey={useSeriesKey ? asset : undefined}
            theme={theme}
            defaultCandleWidth={candleWidth > 0 ? candleWidth : undefined}
            liquidity={showLiquidity ? demoLiquidity : undefined}
            {...indicatorProps}
            {...drawProps}
            onCrosshair={onCrosshair}
          />
        </div>
      )}
      {settingsOpen && (
        <SettingsModal
          theme={theme}
          onChange={setTheme}
          onClose={() => setSettingsOpen(false)}
        />
      )}
      <IndicatorsModal
        visible={indicatorsOpen}
        onClose={() => setIndicatorsOpen(false)}
        state={indicators}
        onToggle={toggleIndicator}
        rsiParams={rsiParams}
        onRsiParamsChange={patchRsi}
        macdParams={macdParams}
        onMacdParamsChange={patchMacd}
        maEditor={maEditor}
        emaEditor={emaEditor}
        vwapParams={vwapParams}
        onVwapParamsChange={patchVwap}
      />
    </div>
  );
}
