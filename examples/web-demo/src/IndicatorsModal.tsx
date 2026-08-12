import { useEffect, useState } from 'react';
import type {
  BollingerBandsConfig,
  MACDConfig,
  MASource,
  MovingAverageOverlay,
  RSIConfig,
  VolumeConfig,
  VWAPConfig,
} from '@vroomchart/react';

// Ported from the React Native test bench (examples/test-bench/IndicatorsMenu.tsx):
// a list -> detail catalog of indicators, each with an enable toggle and
// parameter controls. The host (App) owns all state; this component only owns
// list<->detail navigation.

export type IndicatorId =
  | 'bb'
  | 'ema'
  | 'macd'
  | 'ma'
  | 'rsi'
  | 'volume'
  | 'vwap';

export type IndicatorConfig = {
  enabled: boolean;
};

export type IndicatorState = Record<IndicatorId, IndicatorConfig>;

type IndicatorMeta = {
  id: IndicatorId;
  name: string;
  description: string;
};

export const INDICATORS: IndicatorMeta[] = [
  {
    id: 'bb',
    name: 'Bollinger Bands',
    description:
      'A moving-average basis with bands ±N standard deviations away — the bands widen with volatility and squeeze when it fades.',
  },
  {
    id: 'ema',
    name: 'Exponential Moving Average',
    description:
      'A moving average that weights recent prices more heavily, so it reacts faster to new moves than a simple average.',
  },
  {
    id: 'macd',
    name: 'MACD',
    description:
      'Moving Average Convergence Divergence — momentum from the gap between two EMAs, drawn with a signal line and histogram.',
  },
  {
    id: 'ma',
    name: 'Moving Average',
    description:
      'The average price over a rolling window (SMA). Smooths price action to reveal the underlying trend.',
  },
  {
    id: 'rsi',
    name: 'RSI',
    description:
      'Relative Strength Index — a 0–100 momentum oscillator that flags overbought and oversold conditions.',
  },
  {
    id: 'volume',
    name: 'Volume',
    description:
      'Traded size per candle, drawn as bottom-anchored bars under the price. Bars auto-fit the loudest volume in view, so height reads relatively rather than absolutely.',
  },
  {
    id: 'vwap',
    name: 'VWAP',
    description:
      'Volume Weighted Average Price — the average price over the session weighted by traded volume.',
  },
];

export const DEFAULT_INDICATOR_STATE: IndicatorState = {
  bb: { enabled: false },
  ema: { enabled: false },
  macd: { enabled: false },
  ma: { enabled: false },
  rsi: { enabled: false },
  // The one indicator that ships on, matching the chart's own default.
  volume: { enabled: true },
  vwap: { enabled: false },
};

export function enabledCount(state: IndicatorState): number {
  return Object.values(state).filter((c) => c.enabled).length;
}

export type RSIParams = {
  period: number;
  upperBand: number;
  lowerBand: number;
  maEnabled: boolean;
  maPeriod: number;
};

export const DEFAULT_RSI_PARAMS: RSIParams = {
  period: 14,
  upperBand: 70,
  lowerBand: 30,
  maEnabled: true,
  maPeriod: 14,
};

export type MACDParams = {
  fast: number;
  slow: number;
  signal: number;
  source: MASource;
  maType: 'sma' | 'ema';
  signalMaType: 'sma' | 'ema';
  macdColor: string;
  macdVisible: boolean;
  signalColor: string;
  signalVisible: boolean;
  /** One stroke width for both lines, to keep the panel compact. */
  width: number;
  histogramVisible: boolean;
  histogramUpColor: string;
  histogramDownColor: string;
  zeroLineVisible: boolean;
};

export const DEFAULT_MACD_PARAMS: MACDParams = {
  fast: 12,
  slow: 26,
  signal: 9,
  source: 'close',
  maType: 'ema',
  signalMaType: 'ema',
  macdColor: '#2962ff',
  macdVisible: true,
  signalColor: '#ff9800',
  signalVisible: true,
  width: 1.5,
  histogramVisible: true,
  histogramUpColor: '#26a69a',
  histogramDownColor: '#f85149',
  zeroLineVisible: true,
};

export type MALineParams = {
  length: number;
  source: MASource;
  color: string;
  width: number;
};

export const DEFAULT_MA_LINE: MALineParams = {
  length: 9,
  source: 'close',
  color: '#2962ff',
  width: 1.5,
};

export const DEFAULT_EMA_LINE: MALineParams = {
  length: 9,
  source: 'close',
  color: '#ff9800',
  width: 1.5,
};

export type VWAPParams = {
  resetHour: number;
  color: string;
  width: number;
};

export const DEFAULT_VWAP_PARAMS: VWAPParams = {
  resetHour: 0,
  color: '#00bcd4',
  width: 1.5,
};

export type BollingerParams = {
  period: number;
  stdDev: number;
  source: MASource;
  basis: 'sma' | 'ema';
  upperColor: string;
  middleColor: string;
  lowerColor: string;
  width: number;
  fill: boolean;
  fillOpacity: number;
};

export const DEFAULT_BOLLINGER_PARAMS: BollingerParams = {
  period: 20,
  stdDev: 2,
  source: 'close',
  basis: 'sma',
  upperColor: '#2962ff',
  middleColor: '#ff6d00',
  lowerColor: '#2962ff',
  width: 1,
  fill: true,
  fillOpacity: 0.1,
};

export type VolumeParams = {
  opacity: number;
  height: number;
  radius: number;
};

// Mirrors the core's own volume defaults, so the panel opens showing what the
// chart is already drawing.
export const DEFAULT_VOLUME_PARAMS: VolumeParams = {
  opacity: 0.5,
  height: 0.2,
  radius: 0,
};

export type OverlayEditor = {
  lines: MALineParams[];
  onChange: (index: number, patch: Partial<MALineParams>) => void;
  onAdd: () => void;
  onRemove: (index: number) => void;
};

const MA_SOURCES: MASource[] = [
  'close',
  'open',
  'high',
  'low',
  'hl2',
  'hlc3',
  'ohlc4',
];
const MA_SWATCHES = [
  '#2962ff',
  '#ff9800',
  '#26a69a',
  '#f85149',
  '#8957e5',
  '#00bcd4',
];
const MA_WIDTHS = [
  { label: 'Thin', value: 1 },
  { label: 'Med', value: 1.5 },
  { label: 'Thick', value: 2.5 },
];
const MA_KINDS = [
  { label: 'SMA', value: 0 },
  { label: 'EMA', value: 1 },
];

// Tap-to-cycle button used by the enum rows (price source).
const cycleButton: React.CSSProperties = {
  display: 'flex',
  alignItems: 'center',
  gap: 6,
  padding: '6px 12px',
  borderRadius: 8,
  background: '#161b22',
  border: '1px solid #30363d',
  color: '#c9d1d9',
  fontSize: 14,
  fontWeight: 600,
  cursor: 'pointer',
};

// The chart props derived from the indicator state — shared by both demo views.
export type IndicatorChartProps = {
  rsi: RSIConfig;
  macd: MACDConfig;
  movingAverages: MovingAverageOverlay[];
  vwap: VWAPConfig;
  bollingerBands: BollingerBandsConfig;
  volume: VolumeConfig;
};

export function deriveIndicatorProps(
  state: IndicatorState,
  rsiParams: RSIParams,
  macdParams: MACDParams,
  maLines: MALineParams[],
  emaLines: MALineParams[],
  vwapParams: VWAPParams,
  bbParams: BollingerParams,
  volumeParams: VolumeParams,
): IndicatorChartProps {
  const movingAverages: MovingAverageOverlay[] = [
    ...(state.ma.enabled
      ? maLines.map(
          (l): MovingAverageOverlay => ({
            kind: 'sma',
            length: l.length,
            source: l.source,
            color: l.color,
            width: l.width,
          }),
        )
      : []),
    ...(state.ema.enabled
      ? emaLines.map(
          (l): MovingAverageOverlay => ({
            kind: 'ema',
            length: l.length,
            source: l.source,
            color: l.color,
            width: l.width,
          }),
        )
      : []),
  ];
  return {
    rsi: { enabled: state.rsi.enabled, ...rsiParams },
    macd: {
      enabled: state.macd.enabled,
      fast: macdParams.fast,
      slow: macdParams.slow,
      signal: macdParams.signal,
      source: macdParams.source,
      maType: macdParams.maType,
      signalMaType: macdParams.signalMaType,
      macdColor: macdParams.macdColor,
      macdWidth: macdParams.width,
      macdVisible: macdParams.macdVisible,
      signalColor: macdParams.signalColor,
      signalWidth: macdParams.width,
      signalVisible: macdParams.signalVisible,
      histogramVisible: macdParams.histogramVisible,
      histogramUpColor: macdParams.histogramUpColor,
      histogramDownColor: macdParams.histogramDownColor,
      zeroLineVisible: macdParams.zeroLineVisible,
    },
    movingAverages,
    vwap: {
      enabled: state.vwap.enabled,
      resetMinutes: vwapParams.resetHour * 60,
      color: vwapParams.color,
      width: vwapParams.width,
    },
    bollingerBands: {
      enabled: state.bb.enabled,
      period: bbParams.period,
      stdDev: bbParams.stdDev,
      source: bbParams.source,
      basis: bbParams.basis,
      upperColor: bbParams.upperColor,
      upperWidth: bbParams.width,
      middleColor: bbParams.middleColor,
      middleWidth: bbParams.width,
      lowerColor: bbParams.lowerColor,
      lowerWidth: bbParams.width,
      fill: bbParams.fill,
      fillOpacity: bbParams.fillOpacity,
    },
    volume: {
      enabled: state.volume.enabled,
      opacity: volumeParams.opacity,
      height: volumeParams.height,
      radius: volumeParams.radius,
    },
  };
}

const overlay: React.CSSProperties = {
  position: 'fixed',
  inset: 0,
  background: 'rgba(1, 4, 9, 0.6)',
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'center',
  zIndex: 100,
};

const panel: React.CSSProperties = {
  background: '#161b22',
  color: '#c9d1d9',
  border: '1px solid #30363d',
  borderRadius: 10,
  width: 460,
  maxWidth: 'calc(100vw - 32px)',
  maxHeight: 'calc(100vh - 64px)',
  display: 'flex',
  flexDirection: 'column',
  overflow: 'hidden',
  boxShadow: '0 12px 32px rgba(1, 4, 9, 0.7)',
  fontFamily: 'system-ui, sans-serif',
};

const navBar: React.CSSProperties = {
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'space-between',
  padding: '12px 16px',
  borderBottom: '1px solid #30363d',
};

const navAction: React.CSSProperties = {
  background: 'transparent',
  border: 'none',
  color: '#58a6ff',
  fontSize: 14,
  cursor: 'pointer',
  padding: 0,
};

const scrollArea: React.CSSProperties = { overflowY: 'auto', padding: '8px 0' };

export function IndicatorsModal({
  visible,
  onClose,
  state,
  onToggle,
  rsiParams,
  onRsiParamsChange,
  macdParams,
  onMacdParamsChange,
  maEditor,
  emaEditor,
  vwapParams,
  onVwapParamsChange,
  bbParams,
  onBbParamsChange,
  volumeParams,
  onVolumeParamsChange,
}: {
  visible: boolean;
  onClose: () => void;
  state: IndicatorState;
  onToggle: (id: IndicatorId, enabled: boolean) => void;
  rsiParams: RSIParams;
  onRsiParamsChange: (patch: Partial<RSIParams>) => void;
  macdParams: MACDParams;
  onMacdParamsChange: (patch: Partial<MACDParams>) => void;
  maEditor: OverlayEditor;
  emaEditor: OverlayEditor;
  vwapParams: VWAPParams;
  onVwapParamsChange: (patch: Partial<VWAPParams>) => void;
  bbParams: BollingerParams;
  onBbParamsChange: (patch: Partial<BollingerParams>) => void;
  volumeParams: VolumeParams;
  onVolumeParamsChange: (patch: Partial<VolumeParams>) => void;
}) {
  const [detailId, setDetailId] = useState<IndicatorId | null>(null);

  // Always reopen on the list, never the last-viewed detail.
  useEffect(() => {
    if (!visible) setDetailId(null);
  }, [visible]);

  if (!visible) return null;

  const detail = detailId
    ? (INDICATORS.find((i) => i.id === detailId) ?? null)
    : null;

  return (
    <div style={overlay} onClick={onClose}>
      <div style={panel} onClick={(e) => e.stopPropagation()}>
        {detail ? (
          <DetailScreen
            meta={detail}
            enabled={state[detail.id].enabled}
            onToggle={(v) => onToggle(detail.id, v)}
            onBack={() => setDetailId(null)}
            rsiParams={detail.id === 'rsi' ? rsiParams : undefined}
            onRsiParamsChange={
              detail.id === 'rsi' ? onRsiParamsChange : undefined
            }
            macdParams={detail.id === 'macd' ? macdParams : undefined}
            onMacdParamsChange={
              detail.id === 'macd' ? onMacdParamsChange : undefined
            }
            editor={
              detail.id === 'ma'
                ? maEditor
                : detail.id === 'ema'
                  ? emaEditor
                  : undefined
            }
            vwapParams={detail.id === 'vwap' ? vwapParams : undefined}
            onVwapParamsChange={
              detail.id === 'vwap' ? onVwapParamsChange : undefined
            }
            bbParams={detail.id === 'bb' ? bbParams : undefined}
            onBbParamsChange={
              detail.id === 'bb' ? onBbParamsChange : undefined
            }
            volumeParams={detail.id === 'volume' ? volumeParams : undefined}
            onVolumeParamsChange={
              detail.id === 'volume' ? onVolumeParamsChange : undefined
            }
          />
        ) : (
          <ListScreen state={state} onClose={onClose} onSelect={setDetailId} />
        )}
      </div>
    </div>
  );
}

function ListScreen({
  state,
  onClose,
  onSelect,
}: {
  state: IndicatorState;
  onClose: () => void;
  onSelect: (id: IndicatorId) => void;
}) {
  return (
    <>
      <div style={navBar}>
        <strong style={{ fontSize: 15 }}>Indicators</strong>
        <button style={navAction} onClick={onClose}>
          Done
        </button>
      </div>
      <div style={scrollArea}>
        {INDICATORS.map((item, i) => (
          <button
            key={item.id}
            onClick={() => onSelect(item.id)}
            style={{
              display: 'flex',
              alignItems: 'center',
              gap: 8,
              width: '100%',
              textAlign: 'left',
              background: 'transparent',
              border: 'none',
              borderTop: i === 0 ? 'none' : '1px solid #21262d',
              color: '#c9d1d9',
              padding: '12px 16px',
              cursor: 'pointer',
            }}
          >
            <span style={{ flex: 1, minWidth: 0 }}>
              <span
                style={{ display: 'block', fontSize: 15, fontWeight: 600 }}
              >
                {item.name}
              </span>
              <span
                style={{
                  display: 'block',
                  fontSize: 12,
                  color: '#8b949e',
                  marginTop: 2,
                  overflow: 'hidden',
                  textOverflow: 'ellipsis',
                  whiteSpace: 'nowrap',
                }}
              >
                {item.description}
              </span>
            </span>
            {state[item.id].enabled && (
              <span style={{ color: '#3fb950', fontSize: 12, fontWeight: 600 }}>
                On
              </span>
            )}
            <span style={{ color: '#6e7681', fontSize: 20 }}>›</span>
          </button>
        ))}
      </div>
    </>
  );
}

function Toggle({
  value,
  onChange,
}: {
  value: boolean;
  onChange: (value: boolean) => void;
}) {
  return (
    <button
      role="switch"
      aria-checked={value}
      onClick={() => onChange(!value)}
      style={{
        width: 40,
        height: 22,
        borderRadius: 11,
        border: 'none',
        padding: 2,
        cursor: 'pointer',
        background: value ? '#238636' : '#30363d',
        display: 'flex',
        justifyContent: value ? 'flex-end' : 'flex-start',
        alignItems: 'center',
        transition: 'background 0.15s',
      }}
    >
      <span
        style={{
          width: 18,
          height: 18,
          borderRadius: '50%',
          background: '#f0f6fc',
        }}
      />
    </button>
  );
}

function Stepper({
  label,
  value,
  min,
  max,
  onChange,
}: {
  label: string;
  value: number;
  min: number;
  max: number;
  onChange: (value: number) => void;
}) {
  const stepBtn: React.CSSProperties = {
    width: 32,
    height: 32,
    borderRadius: 8,
    background: '#0d1117',
    border: '1px solid #30363d',
    color: '#c9d1d9',
    fontSize: 18,
    fontWeight: 600,
    cursor: 'pointer',
  };
  return (
    <div style={paramRow}>
      <span style={paramLabel}>{label}</span>
      <div style={{ display: 'flex', alignItems: 'center', gap: 8 }}>
        <button style={stepBtn} onClick={() => onChange(Math.max(min, value - 1))}>
          −
        </button>
        <span
          style={{
            minWidth: 40,
            textAlign: 'center',
            fontWeight: 700,
            fontVariantNumeric: 'tabular-nums',
          }}
        >
          {value}
        </span>
        <button style={stepBtn} onClick={() => onChange(Math.min(max, value + 1))}>
          +
        </button>
      </div>
    </div>
  );
}

function Swatches({
  value,
  onChange,
}: {
  value: string;
  onChange: (color: string) => void;
}) {
  return (
    <div style={{ display: 'flex', alignItems: 'center', gap: 8 }}>
      {MA_SWATCHES.map((c) => (
        <button
          key={c}
          onClick={() => onChange(c)}
          aria-label={c}
          style={{
            width: 22,
            height: 22,
            borderRadius: '50%',
            background: c,
            cursor: 'pointer',
            border:
              value.toLowerCase() === c.toLowerCase()
                ? '2px solid #f0f6fc'
                : '2px solid transparent',
          }}
        />
      ))}
    </div>
  );
}

function Segmented({
  options,
  value,
  onChange,
}: {
  options: { label: string; value: number }[];
  value: number;
  onChange: (value: number) => void;
}) {
  return (
    <div
      style={{
        display: 'flex',
        borderRadius: 8,
        overflow: 'hidden',
        border: '1px solid #30363d',
      }}
    >
      {options.map((o) => {
        const active = o.value === value;
        return (
          <button
            key={o.label}
            onClick={() => onChange(o.value)}
            style={{
              padding: '6px 12px',
              background: active ? '#21262d' : '#0d1117',
              color: active ? '#c9d1d9' : '#8b949e',
              border: 'none',
              fontSize: 13,
              fontWeight: 500,
              cursor: 'pointer',
            }}
          >
            {o.label}
          </button>
        );
      })}
    </div>
  );
}

function OverlayLineEditor({
  line,
  index,
  onChange,
  onRemove,
}: {
  line: MALineParams;
  index: number;
  onChange: (patch: Partial<MALineParams>) => void;
  onRemove: () => void;
}) {
  const cycleSource = () => {
    const i = MA_SOURCES.indexOf(line.source);
    onChange({ source: MA_SOURCES[(i + 1) % MA_SOURCES.length] });
  };
  return (
    <div
      style={{
        marginBottom: 14,
        padding: 12,
        borderRadius: 10,
        background: '#0d1117',
        border: '1px solid #21262d',
      }}
    >
      <div
        style={{
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-between',
          marginBottom: 4,
        }}
      >
        <span style={{ color: '#8b949e', fontSize: 12, fontWeight: 600 }}>
          Line {index + 1}
        </span>
        <button
          onClick={onRemove}
          style={{
            background: 'transparent',
            border: 'none',
            color: '#f85149',
            fontSize: 13,
            fontWeight: 600,
            cursor: 'pointer',
            padding: 0,
          }}
        >
          Remove
        </button>
      </div>
      <Stepper
        label="Length"
        value={line.length}
        min={1}
        max={400}
        onChange={(n) => onChange({ length: n })}
      />
      <div style={paramRow}>
        <span style={paramLabel}>Source</span>
        <button
          onClick={cycleSource}
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: 6,
            padding: '6px 12px',
            borderRadius: 8,
            background: '#161b22',
            border: '1px solid #30363d',
            color: '#c9d1d9',
            fontSize: 14,
            fontWeight: 600,
            cursor: 'pointer',
          }}
        >
          {line.source}
          <span style={{ color: '#6e7681', fontSize: 13 }}>⟳</span>
        </button>
      </div>
      <div style={paramRow}>
        <span style={paramLabel}>Color</span>
        <Swatches value={line.color} onChange={(c) => onChange({ color: c })} />
      </div>
      <div style={paramRow}>
        <span style={paramLabel}>Width</span>
        <Segmented
          options={MA_WIDTHS}
          value={line.width}
          onChange={(w) => onChange({ width: w })}
        />
      </div>
    </div>
  );
}

const paramRow: React.CSSProperties = {
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'space-between',
  padding: '8px 0',
};
const paramLabel: React.CSSProperties = { color: '#c9d1d9', fontSize: 14 };

function DetailScreen({
  meta,
  enabled,
  onToggle,
  onBack,
  rsiParams,
  onRsiParamsChange,
  macdParams,
  onMacdParamsChange,
  editor,
  vwapParams,
  onVwapParamsChange,
  bbParams,
  onBbParamsChange,
  volumeParams,
  onVolumeParamsChange,
}: {
  meta: IndicatorMeta;
  enabled: boolean;
  onToggle: (value: boolean) => void;
  onBack: () => void;
  rsiParams?: RSIParams;
  onRsiParamsChange?: (patch: Partial<RSIParams>) => void;
  macdParams?: MACDParams;
  onMacdParamsChange?: (patch: Partial<MACDParams>) => void;
  editor?: OverlayEditor;
  vwapParams?: VWAPParams;
  onVwapParamsChange?: (patch: Partial<VWAPParams>) => void;
  bbParams?: BollingerParams;
  onBbParamsChange?: (patch: Partial<BollingerParams>) => void;
  volumeParams?: VolumeParams;
  onVolumeParamsChange?: (patch: Partial<VolumeParams>) => void;
}) {
  const rsi = rsiParams && onRsiParamsChange ? rsiParams : null;
  const macd = macdParams && onMacdParamsChange ? macdParams : null;
  const vwap = vwapParams && onVwapParamsChange ? vwapParams : null;
  const bb = bbParams && onBbParamsChange ? bbParams : null;
  const vol = volumeParams && onVolumeParamsChange ? volumeParams : null;
  return (
    <>
      <div style={navBar}>
        <button style={navAction} onClick={onBack}>
          ‹ Indicators
        </button>
      </div>
      <div style={{ ...scrollArea, padding: 16 }}>
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'space-between',
            gap: 12,
          }}
        >
          <strong style={{ fontSize: 18, color: '#f0f6fc' }}>{meta.name}</strong>
          <Toggle value={enabled} onChange={onToggle} />
        </div>

        <p
          style={{
            color: '#8b949e',
            fontSize: 14,
            lineHeight: 1.5,
            marginTop: 12,
          }}
        >
          {meta.description}
        </p>

        <div style={{ marginTop: 24 }}>
          <div
            style={{
              color: '#6e7681',
              fontSize: 12,
              fontWeight: 600,
              letterSpacing: 0.5,
              marginBottom: 4,
            }}
          >
            SETTINGS
          </div>
          {rsi ? (
            <>
              <Stepper
                label="Period"
                value={rsi.period}
                min={2}
                max={50}
                onChange={(n) => onRsiParamsChange!({ period: n })}
              />
              <Stepper
                label="Overbought"
                value={rsi.upperBand}
                min={rsi.lowerBand + 1}
                max={100}
                onChange={(n) => onRsiParamsChange!({ upperBand: n })}
              />
              <Stepper
                label="Oversold"
                value={rsi.lowerBand}
                min={0}
                max={rsi.upperBand - 1}
                onChange={(n) => onRsiParamsChange!({ lowerBand: n })}
              />
              <div style={paramRow}>
                <span style={paramLabel}>Trendline (RSI MA)</span>
                <Toggle
                  value={rsi.maEnabled}
                  onChange={(v) => onRsiParamsChange!({ maEnabled: v })}
                />
              </div>
              {rsi.maEnabled && (
                <Stepper
                  label="Trendline length"
                  value={rsi.maPeriod}
                  min={1}
                  max={50}
                  onChange={(n) => onRsiParamsChange!({ maPeriod: n })}
                />
              )}
            </>
          ) : macd ? (
            <>
              <Stepper
                label="Fast"
                value={macd.fast}
                min={1}
                max={macd.slow - 1}
                onChange={(n) => onMacdParamsChange!({ fast: n })}
              />
              <Stepper
                label="Slow"
                value={macd.slow}
                min={macd.fast + 1}
                max={100}
                onChange={(n) => onMacdParamsChange!({ slow: n })}
              />
              <Stepper
                label="Signal"
                value={macd.signal}
                min={1}
                max={50}
                onChange={(n) => onMacdParamsChange!({ signal: n })}
              />
              <div style={paramRow}>
                <span style={paramLabel}>Source</span>
                <button
                  onClick={() => {
                    const i = MA_SOURCES.indexOf(macd.source);
                    onMacdParamsChange!({
                      source: MA_SOURCES[(i + 1) % MA_SOURCES.length],
                    });
                  }}
                  style={cycleButton}
                >
                  {macd.source}
                  <span style={{ color: '#6e7681', fontSize: 13 }}>⟳</span>
                </button>
              </div>
              <div style={paramRow}>
                <span style={paramLabel}>Averaging</span>
                <Segmented
                  options={MA_KINDS}
                  value={macd.maType === 'ema' ? 1 : 0}
                  onChange={(v) =>
                    onMacdParamsChange!({ maType: v === 1 ? 'ema' : 'sma' })
                  }
                />
              </div>
              <div style={paramRow}>
                <span style={paramLabel}>Signal averaging</span>
                <Segmented
                  options={MA_KINDS}
                  value={macd.signalMaType === 'ema' ? 1 : 0}
                  onChange={(v) =>
                    onMacdParamsChange!({
                      signalMaType: v === 1 ? 'ema' : 'sma',
                    })
                  }
                />
              </div>
              <div style={paramRow}>
                <span style={paramLabel}>MACD line</span>
                <Toggle
                  value={macd.macdVisible}
                  onChange={(v) => onMacdParamsChange!({ macdVisible: v })}
                />
              </div>
              {macd.macdVisible && (
                <div style={paramRow}>
                  <span style={paramLabel}>MACD color</span>
                  <Swatches
                    value={macd.macdColor}
                    onChange={(c) => onMacdParamsChange!({ macdColor: c })}
                  />
                </div>
              )}
              <div style={paramRow}>
                <span style={paramLabel}>Signal line</span>
                <Toggle
                  value={macd.signalVisible}
                  onChange={(v) => onMacdParamsChange!({ signalVisible: v })}
                />
              </div>
              {macd.signalVisible && (
                <div style={paramRow}>
                  <span style={paramLabel}>Signal color</span>
                  <Swatches
                    value={macd.signalColor}
                    onChange={(c) => onMacdParamsChange!({ signalColor: c })}
                  />
                </div>
              )}
              <div style={paramRow}>
                <span style={paramLabel}>Line width</span>
                <Segmented
                  options={MA_WIDTHS}
                  value={macd.width}
                  onChange={(w) => onMacdParamsChange!({ width: w })}
                />
              </div>
              <div style={paramRow}>
                <span style={paramLabel}>Histogram</span>
                <Toggle
                  value={macd.histogramVisible}
                  onChange={(v) => onMacdParamsChange!({ histogramVisible: v })}
                />
              </div>
              {macd.histogramVisible && (
                <>
                  <div style={paramRow}>
                    <span style={paramLabel}>Above zero</span>
                    <Swatches
                      value={macd.histogramUpColor}
                      onChange={(c) =>
                        onMacdParamsChange!({ histogramUpColor: c })
                      }
                    />
                  </div>
                  <div style={paramRow}>
                    <span style={paramLabel}>Below zero</span>
                    <Swatches
                      value={macd.histogramDownColor}
                      onChange={(c) =>
                        onMacdParamsChange!({ histogramDownColor: c })
                      }
                    />
                  </div>
                </>
              )}
              <div style={paramRow}>
                <span style={paramLabel}>Zero line</span>
                <Toggle
                  value={macd.zeroLineVisible}
                  onChange={(v) => onMacdParamsChange!({ zeroLineVisible: v })}
                />
              </div>
            </>
          ) : editor ? (
            <>
              {editor.lines.map((line, i) => (
                <OverlayLineEditor
                  key={i}
                  line={line}
                  index={i}
                  onChange={(patch) => editor.onChange(i, patch)}
                  onRemove={() => editor.onRemove(i)}
                />
              ))}
              <button
                onClick={editor.onAdd}
                style={{
                  width: '100%',
                  marginTop: 4,
                  padding: '12px',
                  borderRadius: 8,
                  border: '1px dashed #30363d',
                  background: 'transparent',
                  color: '#58a6ff',
                  fontSize: 15,
                  fontWeight: 600,
                  cursor: 'pointer',
                }}
              >
                + Add line
              </button>
            </>
          ) : vwap ? (
            <>
              <Stepper
                label="Reset hour (UTC)"
                value={vwap.resetHour}
                min={0}
                max={23}
                onChange={(n) => onVwapParamsChange!({ resetHour: n })}
              />
              <div style={paramRow}>
                <span style={paramLabel}>Color</span>
                <Swatches
                  value={vwap.color}
                  onChange={(c) => onVwapParamsChange!({ color: c })}
                />
              </div>
              <div style={paramRow}>
                <span style={paramLabel}>Width</span>
                <Segmented
                  options={MA_WIDTHS}
                  value={vwap.width}
                  onChange={(w) => onVwapParamsChange!({ width: w })}
                />
              </div>
            </>
          ) : bb ? (
            <>
              <Stepper
                label="Period"
                value={bb.period}
                min={1}
                max={200}
                onChange={(n) => onBbParamsChange!({ period: n })}
              />
              <div style={paramRow}>
                <span style={paramLabel}>Std dev</span>
                <Segmented
                  options={[
                    { label: '1', value: 1 },
                    { label: '1.5', value: 1.5 },
                    { label: '2', value: 2 },
                    { label: '2.5', value: 2.5 },
                    { label: '3', value: 3 },
                  ]}
                  value={bb.stdDev}
                  onChange={(v) => onBbParamsChange!({ stdDev: v })}
                />
              </div>
              <div style={paramRow}>
                <span style={paramLabel}>Basis</span>
                <Segmented
                  options={MA_KINDS}
                  value={bb.basis === 'ema' ? 1 : 0}
                  onChange={(v) =>
                    onBbParamsChange!({ basis: v === 1 ? 'ema' : 'sma' })
                  }
                />
              </div>
              <div style={paramRow}>
                <span style={paramLabel}>Source</span>
                <button
                  onClick={() => {
                    const i = MA_SOURCES.indexOf(bb.source);
                    onBbParamsChange!({
                      source: MA_SOURCES[(i + 1) % MA_SOURCES.length],
                    });
                  }}
                  style={cycleButton}
                >
                  {bb.source}
                  <span style={{ color: '#6e7681', fontSize: 13 }}>⟳</span>
                </button>
              </div>
              <div style={paramRow}>
                <span style={paramLabel}>Band color</span>
                <Swatches
                  value={bb.upperColor}
                  onChange={(c) =>
                    onBbParamsChange!({ upperColor: c, lowerColor: c })
                  }
                />
              </div>
              <div style={paramRow}>
                <span style={paramLabel}>Basis color</span>
                <Swatches
                  value={bb.middleColor}
                  onChange={(c) => onBbParamsChange!({ middleColor: c })}
                />
              </div>
              <div style={paramRow}>
                <span style={paramLabel}>Width</span>
                <Segmented
                  options={MA_WIDTHS}
                  value={bb.width}
                  onChange={(w) => onBbParamsChange!({ width: w })}
                />
              </div>
              <div style={paramRow}>
                <span style={paramLabel}>Fill</span>
                <Toggle
                  value={bb.fill}
                  onChange={(v) => onBbParamsChange!({ fill: v })}
                />
              </div>
              {bb.fill && (
                <div style={paramRow}>
                  <span style={paramLabel}>Fill opacity</span>
                  <Segmented
                    options={[
                      { label: '5%', value: 0.05 },
                      { label: '10%', value: 0.1 },
                      { label: '20%', value: 0.2 },
                    ]}
                    value={bb.fillOpacity}
                    onChange={(v) => onBbParamsChange!({ fillOpacity: v })}
                  />
                </div>
              )}
            </>
          ) : vol ? (
            <>
              <div style={paramRow}>
                <span style={paramLabel}>Opacity</span>
                <Segmented
                  options={[
                    { label: '25%', value: 0.25 },
                    { label: '50%', value: 0.5 },
                    { label: '75%', value: 0.75 },
                    { label: '100%', value: 1 },
                  ]}
                  value={vol.opacity}
                  onChange={(v) => onVolumeParamsChange!({ opacity: v })}
                />
              </div>
              <div style={paramRow}>
                <span style={paramLabel}>Height</span>
                <Segmented
                  options={[
                    { label: '10%', value: 0.1 },
                    { label: '20%', value: 0.2 },
                    { label: '35%', value: 0.35 },
                    { label: '50%', value: 0.5 },
                  ]}
                  value={vol.height}
                  onChange={(v) => onVolumeParamsChange!({ height: v })}
                />
              </div>
              <div style={paramRow}>
                <span style={paramLabel}>Top radius</span>
                <Segmented
                  options={[
                    { label: 'Square', value: 0 },
                    { label: '2px', value: 2 },
                    { label: '4px', value: 4 },
                    { label: '8px', value: 8 },
                  ]}
                  value={vol.radius}
                  onChange={(v) => onVolumeParamsChange!({ radius: v })}
                />
              </div>
            </>
          ) : (
            <p style={{ color: '#8b949e', fontSize: 14 }}>
              No parameters for this indicator.
            </p>
          )}
        </div>
      </div>
    </>
  );
}
