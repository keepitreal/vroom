import { useCallback, useEffect, useMemo, useState } from 'react';
import {
  VroomChart,
  type Candle,
  type CrosshairEvent,
  type ChartMode,
  type DrawTool,
  type Drawing,
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

export function App() {
  // No wasm/asset config needed — @vroomchart/react uses the Skia-WASM core
  // bundled in @vroomchart/core-wasm.
  const [asset, setAsset] = useState<Asset>('BTC');
  const [tf, setTf] = useState<number>(MINUTE);
  const [useSeriesKey, setUseSeriesKey] = useState(true);
  const candles = useMemo(() => aggregate(baseSeries(asset), tf), [asset, tf]);
  const [readout, setReadout] = useState<string>('hover / long-press for crosshair');
  const [view, setView] = useState<'repro' | 'demo' | 'sync'>('repro');

  // Sync view: two charts of the same asset (comparable price scale so the
  // synced horizontal line stays on-screen) sharing one crosshair. Each chart
  // both emits its crosshair (onCrosshair) and mirrors the shared one
  // (crosshairOverride); the library ignores the override on whichever chart is
  // actively hovered, so no host-side source tracking is needed.
  const syncTop = useMemo(() => aggregate(baseSeries('BTC'), MINUTE), []);
  const syncBottom = useMemo(() => aggregate(baseSeries('BTC'), 15 * MINUTE), []);
  const [xhair, setXhair] = useState<{ timeMs: number; price: number } | null>(null);
  const onSyncCrosshair = useCallback((e: CrosshairEvent) => {
    setXhair(e.active && e.timeMs != null && e.price != null ? { timeMs: e.timeMs, price: e.price } : null);
  }, []);
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
            <span style={{ fontFamily: 'ui-monospace, monospace', fontSize: 13, opacity: 0.85 }}>{readout}</span>
          </>
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
            candles={candles}
            seriesKey={useSeriesKey ? asset : undefined}
            theme={theme}
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
