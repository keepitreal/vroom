import { useCallback, useEffect, useMemo, useState } from 'react';
import { VroomChart, type Candle, type CrosshairEvent } from '@vroomchart/react';
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

// Synthetic random-walk candles for the demo.
function mockCandles(n: number, stepMs: number): Candle[] {
  const out: Candle[] = [];
  let price = 100;
  const now = Date.now();
  for (let i = 0; i < n; i++) {
    const open = price;
    const close = open + (Math.random() - 0.5) * 4;
    const high = Math.max(open, close) + Math.random() * 2;
    const low = Math.min(open, close) - Math.random() * 2;
    out.push({
      timeMs: now - (n - i) * stepMs,
      open,
      high,
      low,
      close,
      volume: Math.random() * 1000,
    });
    price = close;
  }
  return out;
}

const DAY = 24 * 60 * 60 * 1000;

export function App() {
  // No wasm/asset config needed — @vroomchart/react uses the Skia-WASM core
  // bundled in @vroomchart/core-wasm.
  const candles = useMemo(() => mockCandles(300, DAY), []);
  const [readout, setReadout] = useState<string>('hover / long-press for crosshair');
  const [view, setView] = useState<'repro' | 'demo'>('repro');
  const [theme, setTheme] = useState<ThemeState>(loadTheme);
  const [settingsOpen, setSettingsOpen] = useState(false);

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
          {(['repro', 'demo'] as const).map((v) => (
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
              {v === 'repro' ? 'Repro' : 'Demo'}
            </button>
          ))}
        </div>
        {view === 'demo' && (
          <span style={{ fontFamily: 'ui-monospace, monospace', fontSize: 13, opacity: 0.85 }}>{readout}</span>
        )}
        <div style={{ marginLeft: 'auto', display: 'flex', gap: 6 }}>
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
          <StreamingRepro theme={theme} indicators={indicatorProps} />
        </div>
      ) : (
        <div style={{ flex: 1, minHeight: 0, padding: '0 8px 8px' }}>
          <VroomChart
            candles={candles}
            theme={theme}
            {...indicatorProps}
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
