import { type CSSProperties, useCallback, useEffect, useMemo, useRef, useState } from 'react';
import {
  VroomChart,
  type Candle,
  type CrosshairEvent,
  type ChartMode,
  type ChartType,
  type DrawTool,
  type DrawingSelection,
  type DrawingStore,
  type LiquidityBand,
  type LiquidityConfig,
  type PriceLine,
  type TransitionEasing,
  type UndoRedoControls,
  type UndoRedoState,
} from '@vroomchart/react';
import { Sidebar, type PriceLineStyleChoice } from './Sidebar';
import { SelectionTray } from './SelectionTray';
import { SettingsModal, DEFAULT_THEME, type ThemeState, type NumericStyle } from './SettingsModal';
import {
  IndicatorsModal,
  DEFAULT_INDICATOR_STATE,
  DEFAULT_RSI_PARAMS,
  DEFAULT_MACD_PARAMS,
  DEFAULT_MA_LINE,
  DEFAULT_EMA_LINE,
  DEFAULT_VWAP_PARAMS,
  DEFAULT_BOLLINGER_PARAMS,
  DEFAULT_VOLUME_PARAMS,
  deriveIndicatorProps,
  enabledCount,
  type BollingerParams,
  type IndicatorId,
  type IndicatorState,
  type MACDParams,
  type MALineParams,
  type RSIParams,
  type VolumeParams,
  type VWAPParams,
} from './IndicatorsModal';

const THEME_STORAGE_KEY = 'vroom-theme';
const CANDLE_WIDTH_KEY = 'vroom-candle-width';
const WICK_WIDTH_KEY = 'vroom-wick-width';
const CANDLE_RADIUS_KEY = 'vroom-candle-radius';
const WICK_CAP_KEY = 'vroom-wick-cap';
const LINE_TENSION_KEY = 'vroom-line-tension';
const LINE_TIP_DOT_KEY = 'vroom-line-tip-dot';
const LINE_TIP_PULSE_KEY = 'vroom-line-tip-pulse';
const CHART_TYPE_KEY = 'vroom-chart-type';
const TRANSITION_MS_KEY = 'vroom-transition-ms';
const TRANSITION_EASING_KEY = 'vroom-transition-easing';
const SIDEBAR_KEY = 'vroom-sidebar';

const EASINGS: readonly TransitionEasing[] = ['linear', 'ease-in', 'ease-out', 'ease-in-out'];

// Generic localStorage getters for the numeric/boolean style knobs.
function loadNum(key: string, def: number): number {
  if (typeof window === 'undefined') return def;
  try {
    const raw = window.localStorage.getItem(key);
    if (raw == null) return def;
    const n = Number(raw);
    return Number.isFinite(n) && n >= 0 ? n : def;
  } catch {
    return def;
  }
}
function loadBool(key: string, def: boolean): boolean {
  if (typeof window === 'undefined') return def;
  try {
    const raw = window.localStorage.getItem(key);
    return raw == null ? def : raw === '1';
  } catch {
    return def;
  }
}

// Persisted wick stroke width (px). Falls back to 1 on missing/corrupt data.
function loadWickWidth(): number {
  const DEFAULT = 1;
  if (typeof window === 'undefined') return DEFAULT;
  try {
    const raw = window.localStorage.getItem(WICK_WIDTH_KEY);
    if (!raw) return DEFAULT;
    const n = Number(raw);
    return Number.isFinite(n) && n > 0 ? n : DEFAULT;
  } catch {
    return DEFAULT;
  }
}

// Persisted sidebar expand/collapse state (default expanded).
function loadSidebarOpen(): boolean {
  if (typeof window === 'undefined') return true;
  try {
    return window.localStorage.getItem(SIDEBAR_KEY) !== '0';
  } catch {
    return true;
  }
}

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
  // Managed persistence: the chart owns the drawings array and loads/saves it
  // through this adapter, keyed by the market (seriesKey).
  drawingStore: DrawingStore;
  onModeChange: (m: ChartMode) => void;
  // Undo/redo surface (managed mode): availability out, button-driven controls in.
  onHistoryChange: (s: UndoRedoState) => void;
  historyRef: { current: UndoRedoControls | null };
  // Where the selected drawing sits on screen, for the floating tray.
  onSelectionChange: (s: DrawingSelection | null) => void;
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

// Short-series repro: a 9-bar window is far shorter than defaultCandleWidth
// framing on a typical plot, so empty past sits to the left of the last bar.
const SPARSE_COUNT = 9;

function maybeSparse(series: Candle[], sparse: boolean): Candle[] {
  if (!sparse || series.length <= SPARSE_COUNT) return series;
  return series.slice(-SPARSE_COUNT);
}

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

// A price line plus the label prefix its `text` is built from, so a drag can
// rebuild the caption around the new price.
type DemoPriceLine = PriceLine & { label: string };

const priceLineText = (label: string, price: number) => `${label} @ ${price.toFixed(2)}`;

// Sample order/position lines anchored around the latest close: a draggable
// resting limit buy under spot (with a size pill and a cancel button), the
// take-profit it pairs with above, and a fixed liquidation level the user can
// neither move nor dismiss. Each takes a different lineStyle (the take-profit
// leaves it unset for the dotted default) so all three render at once.
function makePriceLines(candles: Candle[]): DemoPriceLine[] {
  if (candles.length === 0) return [];
  // Spread the samples across the price range of the candles that are roughly
  // on screen, rather than a fixed percentage of spot: off-range lines are
  // culled, and a percentage that reads well for a $100 asset puts every line
  // outside the viewport for one priced in the tens of thousands.
  const recent = candles.slice(-120);
  let lo = recent[0].low;
  let hi = recent[0].high;
  for (const c of recent) {
    if (c.low < lo) lo = c.low;
    if (c.high > hi) hi = c.high;
  }
  const at = (frac: number) => lo + (hi - lo) * frac;
  const line = (
    id: string,
    label: string,
    price: number,
    rest: Partial<DemoPriceLine>,
  ): DemoPriceLine => ({ id, label, price, text: priceLineText(label, price), ...rest });
  return [
    line('limit-buy', 'Limit Buy', at(0.3), {
      quantity: '0.75',
      color: '#26a69a',
      draggable: true,
      lineStyle: 'dashed',
    }),
    line('take-profit', 'Take Profit', at(0.78), {
      quantity: 'Full',
      color: '#ef5350',
      draggable: true,
    }),
    line('liquidation', 'Liquidation', at(0.08), {
      color: '#f0a020',
      closable: false,
      lineStyle: 'solid',
    }),
  ];
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

const toolBtnActive: CSSProperties = {
  ...toolBtn,
  background: '#1f6feb',
  border: '1px solid #1f6feb',
  color: '#fff',
};

export function App() {
  // No wasm/asset config needed — @vroomchart/react uses the Skia-WASM core
  // bundled in @vroomchart/core-wasm.
  const [asset, setAsset] = useState<Asset>('BTC');
  const [tf, setTf] = useState<number>(MINUTE);
  const [useSeriesKey, setUseSeriesKey] = useState(true);
  const [gaps, setGaps] = useState(false);
  // Short-series repro (last 9 bars). `?sparse=1` turns it on at load so the
  // verify flow can deep-link; the sidebar toggle is the interactive control.
  const [sparse, setSparse] = useState(
    () => typeof window !== 'undefined' && new URLSearchParams(window.location.search).has('sparse'),
  );
  const [showLiquidity, setShowLiquidity] = useState(false);
  const [showPriceLines, setShowPriceLines] = useState(false);
  // Demo candles are stateful so the Add/Update tools can stream into them; they
  // reset to the base series whenever the asset/timeframe (or Gaps/Sparse) changes.
  const demoBase = useMemo(() => {
    const base = aggregate(baseSeries(asset), tf);
    const series = gaps ? punchGaps(base) : base;
    return maybeSparse(series, sparse);
  }, [asset, tf, gaps, sparse]);
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
  // Price-series render style (candles vs line), persisted.
  const [chartType, setChartType] = useState<ChartType>(() =>
    typeof window !== 'undefined' && window.localStorage.getItem(CHART_TYPE_KEY) === 'line'
      ? 'line'
      : 'candles',
  );
  // Duration (ms) of the candle↔line and interval-switch animations. 0 = snap.
  const [transitionMs, setTransitionMs] = useState(() => loadNum(TRANSITION_MS_KEY, 300));
  const [easing, setEasing] = useState<TransitionEasing>(() => {
    if (typeof window === 'undefined') return 'ease-in-out';
    const v = window.localStorage.getItem(TRANSITION_EASING_KEY);
    return EASINGS.includes(v as TransitionEasing) ? (v as TransitionEasing) : 'ease-in-out';
  });
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

  // Price lines are a controlled prop, so the demo owns the array: a drag reports
  // a candidate price and the × reports a cancellation, and this state is what
  // decides whether either actually happens.
  const [priceLines, setPriceLines] = useState<DemoPriceLine[]>([]);
  useEffect(() => {
    if (!showPriceLines) return;
    setPriceLines(makePriceLines(candles));
    // Keyed to the series rather than `candles`: re-seeding on every streamed
    // candle would throw away the user's drags.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [showPriceLines, asset, tf]);
  const onPriceLineDrag = useCallback((id: string, price: number) => {
    setReadout(`dragging ${id} → ${price.toFixed(2)}`);
  }, []);
  const onPriceLineDragEnd = useCallback((id: string, price: number) => {
    setReadout(`moved ${id} to ${price.toFixed(2)}`);
    setPriceLines((prev) =>
      prev.map((l) =>
        l.id === id ? { ...l, price, text: priceLineText(l.label, price) } : l,
      ),
    );
  }, []);
  const onPriceLineClose = useCallback((id: string) => {
    setReadout(`cancelled ${id}`);
    setPriceLines((prev) => prev.filter((l) => l.id !== id));
  }, []);
  // Forces one style onto every line so the three can be compared directly;
  // 'mixed' leaves each sample's own lineStyle alone. Applied on the way out
  // rather than to the state above, so switching styles mid-drag doesn't
  // overwrite the price being dragged.
  const [priceLineStyle, setPriceLineStyle] = useState<PriceLineStyleChoice>('mixed');
  const styledPriceLines = useMemo(
    () =>
      priceLineStyle === 'mixed'
        ? priceLines
        : priceLines.map((l) => ({ ...l, lineStyle: priceLineStyle })),
    [priceLines, priceLineStyle],
  );
  const priceLineProps = showPriceLines
    ? {
        priceLines: styledPriceLines,
        onPriceLineDrag,
        onPriceLineDragEnd,
        onPriceLineClose,
      }
    : {};

  // Layout: single chart, or two stacked crosshair-linked panes (replaces the
  // old Sync view). Sidebar expand/collapse is persisted.
  const [twoPane, setTwoPane] = useState(false);
  const [sidebarOpen, setSidebarOpen] = useState(loadSidebarOpen);

  // Live streaming (folded in from the old Repro view): auto-append/update the
  // primary series on an interval, in addition to the manual buttons.
  const [live, setLive] = useState(false);
  const [intervalMs, setIntervalMs] = useState(1000);
  const [streamMode, setStreamMode] = useState<'append' | 'update'>('append');
  useEffect(() => {
    if (!live) return;
    const id = setInterval(() => {
      setCandles((p) => (streamMode === 'append' ? appendCandle(p, tf) : updateLast(p)));
    }, Math.max(100, intervalMs));
    return () => clearInterval(id);
  }, [live, intervalMs, streamMode, tf]);

  // Second pane = the same asset at the next-higher timeframe (or 4× when already
  // at the top), so the two panes always differ. Crosshair is shared via `xhair`:
  // each pane emits onCrosshair (updating xhair) and mirrors crosshairOverride;
  // the library ignores the override on whichever pane is actively hovered.
  const secondTf = useMemo(() => {
    const idx = TIMEFRAMES.findIndex((t) => t.stepMs === tf);
    const higher = TIMEFRAMES[Math.min(idx + 1, TIMEFRAMES.length - 1)];
    return higher.stepMs === tf ? tf * 4 : higher.stepMs;
  }, [tf]);
  const secondCandles = useMemo(
    () => maybeSparse(aggregate(baseSeries(asset), secondTf), sparse),
    [asset, secondTf, sparse],
  );
  const seriesKey = useSeriesKey ? (sparse ? `${asset}-sparse` : asset) : undefined;
  // Remount when sparsity flips so defaultCandleWidth re-frames (same reason
  // the width input remounts). A suffix on seriesKey also marks it a new series.
  const chartKey = `${candleWidth}-${sparse ? 'sparse' : 'full'}`;
  const [xhair, setXhair] = useState<{ timeMs: number; price: number } | null>(null);
  const onSecondaryCrosshair = useCallback((e: CrosshairEvent) => {
    setXhair(e.active && e.timeMs != null && e.price != null ? { timeMs: e.timeMs, price: e.price } : null);
  }, []);

  // Manual streaming tools operate on the primary series at its timeframe.
  const onAddCandle = useCallback(() => {
    setCandles((p) => appendCandle(p, tf));
  }, [tf]);
  const onUpdateLast = useCallback(() => {
    setCandles(updateLast);
  }, []);
  const [theme, setTheme] = useState<ThemeState>(loadTheme);
  const [wickWidth, setWickWidth] = useState(loadWickWidth);
  const [candleRadius, setCandleRadius] = useState(() => loadNum(CANDLE_RADIUS_KEY, 0));
  const [wickRoundCap, setWickRoundCap] = useState(() => loadBool(WICK_CAP_KEY, false));
  const [lineTension, setLineTension] = useState(() => loadNum(LINE_TENSION_KEY, 0));
  const [lineTipDot, setLineTipDot] = useState(() => loadBool(LINE_TIP_DOT_KEY, true));
  const [lineTipPulse, setLineTipPulse] = useState(() => loadBool(LINE_TIP_PULSE_KEY, false));
  const [settingsOpen, setSettingsOpen] = useState(false);

  // Drawing tool state. `drawMode`/`drawTool` drive the chart. Drawings are
  // persisted by the chart itself via `drawingStore` (managed mode), keyed by the
  // asset — so lines survive timeframe switches and reloads, but not asset
  // switches. localStorage here; a real app might use a backend.
  const [drawMode, setDrawMode] = useState<ChartMode>('pan');
  const [drawTool, setDrawTool] = useState<DrawTool>(null);
  const drawingStore = useMemo<DrawingStore>(
    () => ({
      // Opaque string in / out — the chart owns the versioned envelope, so the
      // adapter is just a byte store keyed by market.
      load: (marketId) => {
        try {
          return window.localStorage.getItem(`vroom:drawings:${marketId}`);
        } catch {
          return null;
        }
      },
      save: (marketId, data) => {
        try {
          window.localStorage.setItem(`vroom:drawings:${marketId}`, data);
        } catch {
          /* best-effort */
        }
      },
    }),
    [],
  );

  // Toggle a draw tool: pressing the active tool again returns to pan mode;
  // pressing another switches to it while staying in draw mode. `drawToolRef`
  // lets these stable callbacks read the current tool without re-subscribing.
  const drawToolRef = useRef<DrawTool>(null);
  drawToolRef.current = drawTool;
  const selectTool = useCallback((t: Exclude<DrawTool, null>) => {
    setDrawMode((m) => {
      const turnOff = m === 'draw' && drawToolRef.current === t;
      const next = turnOff ? 'pan' : 'draw';
      setDrawTool(next === 'draw' ? t : null);
      return next;
    });
  }, []);
  const toggleLineTool = useCallback(() => selectTool('line'), [selectTool]);
  const toggleBoxTool = useCallback(() => selectTool('box'), [selectTool]);
  const togglePencilTool = useCallback(() => selectTool('pencil'), [selectTool]);
  const togglePathTool = useCallback(() => selectTool('path'), [selectTool]);

  // Drawing undo/redo: the chart owns the history (managed mode); the sidebar
  // buttons just mirror availability and trigger it. ⌘Z / ⇧⌘Z work natively.
  const [history, setHistory] = useState<UndoRedoState>({ canUndo: false, canRedo: false });
  const historyRef = useRef<UndoRedoControls | null>(null);
  const undoDrawing = useCallback(() => historyRef.current?.undo(), []);
  const redoDrawing = useCallback(() => historyRef.current?.redo(), []);

  // The selected drawing and its live screen rect, driving the floating tray.
  const [selection, setSelection] = useState<DrawingSelection | null>(null);

  // "L" toggles the line tool, "R" the box (rectangle), "P" the pencil, "A" the
  // arrow path — Figma/Excalidraw style. This lives in the demo, not the
  // library — the hotkey is the consuming app's choice, so vroom doesn't
  // enshrine one. Ignore it while typing in a field or with a modifier held.
  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (e.metaKey || e.ctrlKey || e.altKey) return;
      const k = e.key.toLowerCase();
      if (k !== 'l' && k !== 'r' && k !== 'p' && k !== 'a') return;
      const ae = document.activeElement as HTMLElement | null;
      const tag = ae?.tagName;
      if (tag === 'INPUT' || tag === 'TEXTAREA' || ae?.isContentEditable) return;
      e.preventDefault();
      if (k === 'l') toggleLineTool();
      else if (k === 'r') toggleBoxTool();
      else if (k === 'a') togglePathTool();
      else togglePencilTool();
    };
    window.addEventListener('keydown', onKey);
    return () => window.removeEventListener('keydown', onKey);
  }, [toggleLineTool, toggleBoxTool, togglePencilTool, togglePathTool]);

  const drawProps: DrawProps = {
    mode: drawMode,
    tool: drawTool,
    drawingStore,
    // The chart asks to return to pan after the user clicks away from a line.
    onModeChange: (m) => {
      setDrawMode(m);
      if (m === 'pan') setDrawTool(null);
    },
    onHistoryChange: setHistory,
    historyRef,
    onSelectionChange: setSelection,
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

  useEffect(() => {
    try {
      window.localStorage.setItem(CHART_TYPE_KEY, chartType);
    } catch {
      // best-effort
    }
  }, [chartType]);

  useEffect(() => {
    try {
      window.localStorage.setItem(TRANSITION_MS_KEY, String(transitionMs));
    } catch {
      // best-effort
    }
  }, [transitionMs]);

  useEffect(() => {
    try {
      window.localStorage.setItem(TRANSITION_EASING_KEY, easing);
    } catch {
      // best-effort
    }
  }, [easing]);

  useEffect(() => {
    try {
      window.localStorage.setItem(SIDEBAR_KEY, sidebarOpen ? '1' : '0');
    } catch {
      // best-effort
    }
  }, [sidebarOpen]);

  useEffect(() => {
    if (!(wickWidth > 0)) return;
    try {
      window.localStorage.setItem(WICK_WIDTH_KEY, String(wickWidth));
    } catch {
      // best-effort
    }
  }, [wickWidth]);

  useEffect(() => {
    try {
      window.localStorage.setItem(CANDLE_RADIUS_KEY, String(candleRadius));
      window.localStorage.setItem(WICK_CAP_KEY, wickRoundCap ? '1' : '0');
      window.localStorage.setItem(LINE_TENSION_KEY, String(lineTension));
      window.localStorage.setItem(LINE_TIP_DOT_KEY, lineTipDot ? '1' : '0');
      window.localStorage.setItem(LINE_TIP_PULSE_KEY, lineTipPulse ? '1' : '0');
    } catch {
      // best-effort
    }
  }, [candleRadius, wickRoundCap, lineTension, lineTipDot, lineTipPulse]);

  // Color theme plus the numeric/boolean style knobs, as one VroomTheme for the charts.
  const chartTheme = useMemo(
    () => ({
      ...theme,
      wickWidth,
      candleRadius,
      wickRoundCap,
      lineTension,
      lineTipDot,
      lineTipPulse,
    }),
    [theme, wickWidth, candleRadius, wickRoundCap, lineTension, lineTipDot, lineTipPulse],
  );
  const numericStyle: NumericStyle = {
    wickWidth,
    candleRadius,
    wickRoundCap,
    lineTension,
    lineTipDot,
    lineTipPulse,
  };
  const onNumericStyleChange = (patch: Partial<NumericStyle>) => {
    if (patch.wickWidth !== undefined) setWickWidth(patch.wickWidth);
    if (patch.candleRadius !== undefined) setCandleRadius(patch.candleRadius);
    if (patch.wickRoundCap !== undefined) setWickRoundCap(patch.wickRoundCap);
    if (patch.lineTension !== undefined) setLineTension(patch.lineTension);
    if (patch.lineTipDot !== undefined) setLineTipDot(patch.lineTipDot);
    if (patch.lineTipPulse !== undefined) setLineTipPulse(patch.lineTipPulse);
  };

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
  const [bbParams, setBbParams] = useState<BollingerParams>(
    DEFAULT_BOLLINGER_PARAMS,
  );
  const [volumeParams, setVolumeParams] = useState<VolumeParams>(
    DEFAULT_VOLUME_PARAMS,
  );

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
  const patchBb = useCallback(
    (patch: Partial<BollingerParams>) =>
      setBbParams((p) => ({ ...p, ...patch })),
    [],
  );
  const patchVolume = useCallback(
    (patch: Partial<VolumeParams>) =>
      setVolumeParams((p) => ({ ...p, ...patch })),
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
    bbParams,
    volumeParams,
  );
  const activeCount = enabledCount(indicators);

  const onPrimaryCrosshair = (e: CrosshairEvent) => {
    if (!e.active || !e.candle) {
      setReadout('hover / long-press for crosshair');
    } else {
      const c = e.candle;
      const d = new Date(c.timeMs).toISOString().slice(0, 10);
      setReadout(`${d}  O ${c.open.toFixed(2)}  H ${c.high.toFixed(2)}  L ${c.low.toFixed(2)}  C ${c.close.toFixed(2)}`);
    }
    // Drive the linked crosshair only when the second pane is showing.
    if (twoPane) {
      setXhair(e.active && e.timeMs != null && e.price != null ? { timeMs: e.timeMs, price: e.price } : null);
    }
  };

  return (
    <div style={{ height: '100%', display: 'flex', flexDirection: 'column', color: '#c9d1d9', fontFamily: 'system-ui, sans-serif' }}>
      <div style={{ padding: '10px 14px', display: 'flex', gap: 16, alignItems: 'center', borderBottom: '1px solid #21262d' }}>
        <strong style={{ fontSize: 16 }}>Vroom 🏎️💨</strong>
        <div style={{ display: 'flex', gap: 4 }}>
          {TIMEFRAMES.map((t) => (
            <button
              key={t.label}
              onClick={() => setTf(t.stepMs)}
              style={t.stepMs === tf ? toolBtnActive : toolBtn}
              title={`Switch to the ${t.label} interval`}
            >
              {t.label}
            </button>
          ))}
        </div>
        <span style={{ fontFamily: 'ui-monospace, monospace', fontSize: 13, opacity: 0.85 }}>{readout}</span>
        <button
          onClick={() => setSidebarOpen((o) => !o)}
          style={{ ...toolBtn, marginLeft: 'auto' }}
          title={sidebarOpen ? 'Hide controls' : 'Show controls'}
        >
          {sidebarOpen ? '✕ Controls' : '☰ Controls'}
        </button>
      </div>
      <div style={{ flex: 1, minHeight: 0, display: 'flex', flexDirection: 'row' }}>
        <div style={{ flex: 1, minWidth: 0, minHeight: 0, padding: 8, display: 'flex', flexDirection: 'column', gap: 8 }}>
          {twoPane ? (
            <>
              <div style={{ flex: 1, minHeight: 0 }}>
                <VroomChart
                  key={chartKey}
                  candles={candles}
                  seriesKey={seriesKey}
                  theme={chartTheme}
                  chartType={chartType}
                  transitionMs={transitionMs}
                  transitionEasing={easing}
                  defaultCandleWidth={candleWidth > 0 ? candleWidth : undefined}
                  liquidity={showLiquidity ? demoLiquidity : undefined}
                  {...priceLineProps}
                  {...indicatorProps}
                  {...drawProps}
                  onCrosshair={onPrimaryCrosshair}
                  crosshairOverride={xhair}
                />
              </div>
              <div style={{ flex: 1, minHeight: 0 }}>
                <VroomChart
                  key={`second-${chartKey}`}
                  candles={secondCandles}
                  seriesKey={sparse ? `${asset}-2-sparse` : `${asset}-2`}
                  theme={chartTheme}
                  chartType={chartType}
                  transitionMs={transitionMs}
                  transitionEasing={easing}
                  defaultCandleWidth={candleWidth > 0 ? candleWidth : undefined}
                  onCrosshair={onSecondaryCrosshair}
                  crosshairOverride={xhair}
                />
              </div>
            </>
          ) : (
            <div style={{ flex: 1, minHeight: 0, position: 'relative' }}>
              <VroomChart
                // Remount on width / sparse change so the new default framing
                // applies (it only takes effect on a fresh handle — mirrors a
                // real "first load").
                key={chartKey}
                candles={candles}
                seriesKey={seriesKey}
                theme={chartTheme}
                chartType={chartType}
                transitionMs={transitionMs}
                transitionEasing={easing}
                defaultCandleWidth={candleWidth > 0 ? candleWidth : undefined}
                liquidity={showLiquidity ? demoLiquidity : undefined}
                {...priceLineProps}
                {...indicatorProps}
                {...drawProps}
                onCrosshair={onPrimaryCrosshair}
              />
              <SelectionTray selection={selection} controls={historyRef} />
            </div>
          )}
        </div>
        {sidebarOpen && (
          <Sidebar
            layout={{ twoPane, setTwoPane, candleWidth, setCandleWidth, chartType, setChartType }}
            animation={{ transitionMs, setTransitionMs, easing, setEasing }}
            data={{
              assets: Object.keys(ASSETS),
              asset,
              setAsset: (a) => setAsset(a as Asset),
              timeframes: TIMEFRAMES,
              tf,
              setTf,
              useSeriesKey,
              setUseSeriesKey,
              gaps,
              setGaps,
              sparse,
              setSparse,
            }}
            streaming={{
              onAddCandle,
              onUpdateLast,
              live,
              setLive,
              intervalMs,
              setIntervalMs,
              streamMode,
              setStreamMode,
              count: candles.length,
            }}
            overlays={{ showLiquidity, setShowLiquidity, bandHeight, setBandHeight, showPriceLines, setShowPriceLines, priceLineStyle, setPriceLineStyle, drawMode, drawTool, toggleLineTool, toggleBoxTool, togglePencilTool, togglePathTool, history, undoDrawing, redoDrawing }}
            panels={{
              activeCount,
              openIndicators: () => setIndicatorsOpen(true),
              openColors: () => setSettingsOpen(true),
            }}
            onCollapse={() => setSidebarOpen(false)}
          />
        )}
      </div>
      {settingsOpen && (
        <SettingsModal
          theme={theme}
          onChange={setTheme}
          numericStyle={numericStyle}
          onNumericStyleChange={onNumericStyleChange}
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
        bbParams={bbParams}
        onBbParamsChange={patchBb}
        volumeParams={volumeParams}
        onVolumeParamsChange={patchVolume}
      />
    </div>
  );
}
