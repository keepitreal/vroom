import { type CSSProperties, type ReactNode, useState } from 'react';
import type { ChartMode, ChartType } from '@vroomchart/react';

// GitHub-dark palette (matches the rest of the demo's inline styling).
const PANEL = '#161b22';
const BORDER = '#30363d';
const DIVIDER = '#21262d';
const ACTIVE = '#21262d';
const TEXT = '#c9d1d9';
const MUTED = '#8b949e';
const ACCENT = '#58a6ff';

const btn: CSSProperties = {
  background: 'transparent',
  color: TEXT,
  border: `1px solid ${BORDER}`,
  borderRadius: 6,
  padding: '5px 10px',
  fontSize: 13,
  cursor: 'pointer',
};

const numInput: CSSProperties = {
  width: 64,
  background: '#0d1117',
  color: TEXT,
  border: `1px solid ${BORDER}`,
  borderRadius: 6,
  padding: '3px 6px',
  fontSize: 12,
  fontFamily: 'ui-monospace, monospace',
};

const badge: CSSProperties = {
  background: '#238636',
  color: '#f0f6fc',
  borderRadius: 10,
  fontSize: 11,
  fontWeight: 700,
  padding: '0 6px',
  lineHeight: '16px',
};

// ---- Small presentational helpers -----------------------------------------

function Section({
  title,
  defaultOpen = true,
  children,
}: {
  title: string;
  defaultOpen?: boolean;
  children: ReactNode;
}) {
  const [open, setOpen] = useState(defaultOpen);
  return (
    <div style={{ borderBottom: `1px solid ${DIVIDER}` }}>
      <button
        onClick={() => setOpen((o) => !o)}
        style={{
          width: '100%',
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
          background: 'transparent',
          color: TEXT,
          border: 'none',
          padding: '10px 12px',
          fontSize: 12,
          fontWeight: 600,
          letterSpacing: 0.4,
          textTransform: 'uppercase',
          cursor: 'pointer',
        }}
      >
        <span>{title}</span>
        <span style={{ color: MUTED }}>{open ? '▾' : '▸'}</span>
      </button>
      {open && (
        <div style={{ padding: '0 12px 12px', display: 'flex', flexDirection: 'column', gap: 8 }}>
          {children}
        </div>
      )}
    </div>
  );
}

// Inline label + control on one row (for compact controls).
function Row({ label, children }: { label: string; children: ReactNode }) {
  return (
    <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', gap: 8 }}>
      <span style={{ fontSize: 12, color: MUTED }}>{label}</span>
      <div style={{ display: 'flex', alignItems: 'center', gap: 6 }}>{children}</div>
    </div>
  );
}

// Stacked label above control (for button groups that may wrap).
function Field({ label, children }: { label: string; children: ReactNode }) {
  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: 5 }}>
      <span style={{ fontSize: 12, color: MUTED }}>{label}</span>
      {children}
    </div>
  );
}

function ToggleRow({
  label,
  checked,
  onChange,
  title,
}: {
  label: string;
  checked: boolean;
  onChange: (v: boolean) => void;
  title?: string;
}) {
  return (
    <label
      style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', gap: 8, fontSize: 12, cursor: 'pointer' }}
      title={title}
    >
      <span style={{ color: TEXT }}>{label}</span>
      <input type="checkbox" checked={checked} onChange={(e) => onChange(e.target.checked)} />
    </label>
  );
}

function Segmented<T extends string | number>({
  options,
  value,
  onChange,
}: {
  options: { label: string; value: T }[];
  value: T;
  onChange: (v: T) => void;
}) {
  return (
    <div style={{ display: 'flex', gap: 4, flexWrap: 'wrap' }}>
      {options.map((o) => {
        const on = o.value === value;
        return (
          <button
            key={String(o.value)}
            onClick={() => onChange(o.value)}
            style={{
              ...btn,
              padding: '4px 9px',
              background: on ? ACTIVE : 'transparent',
              borderColor: on ? ACCENT : BORDER,
              color: on ? '#f0f6fc' : TEXT,
            }}
          >
            {o.label}
          </button>
        );
      })}
    </div>
  );
}

// ---- Props ----------------------------------------------------------------

export type SidebarProps = {
  layout: {
    twoPane: boolean;
    setTwoPane: (v: boolean) => void;
    candleWidth: number;
    setCandleWidth: (v: number) => void;
    chartType: ChartType;
    setChartType: (v: ChartType) => void;
  };
  data: {
    assets: readonly string[];
    asset: string;
    setAsset: (a: string) => void;
    timeframes: readonly { label: string; stepMs: number }[];
    tf: number;
    setTf: (ms: number) => void;
    useSeriesKey: boolean;
    setUseSeriesKey: (v: boolean) => void;
    gaps: boolean;
    setGaps: (v: boolean) => void;
  };
  streaming: {
    onAddCandle: () => void;
    onUpdateLast: () => void;
    live: boolean;
    setLive: (v: boolean) => void;
    intervalMs: number;
    setIntervalMs: (v: number) => void;
    streamMode: 'append' | 'update';
    setStreamMode: (m: 'append' | 'update') => void;
    count: number;
  };
  overlays: {
    showLiquidity: boolean;
    setShowLiquidity: (v: boolean) => void;
    bandHeight: number;
    setBandHeight: (v: number) => void;
    drawMode: ChartMode;
    toggleLineTool: () => void;
  };
  panels: {
    activeCount: number;
    openIndicators: () => void;
    openColors: () => void;
  };
  onCollapse: () => void;
};

export function Sidebar({ layout, data, streaming, overlays, panels, onCollapse }: SidebarProps) {
  const drawing = overlays.drawMode === 'draw';
  return (
    <aside
      style={{
        width: 300,
        flexShrink: 0,
        height: '100%',
        background: PANEL,
        borderLeft: `1px solid ${BORDER}`,
        overflowY: 'auto',
        display: 'flex',
        flexDirection: 'column',
      }}
    >
      <div
        style={{
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-between',
          padding: '10px 12px',
          borderBottom: `1px solid ${DIVIDER}`,
          position: 'sticky',
          top: 0,
          background: PANEL,
        }}
      >
        <strong style={{ fontSize: 13, letterSpacing: 0.3 }}>Controls</strong>
        <button onClick={onCollapse} style={{ ...btn, padding: '2px 8px' }} title="Collapse sidebar">
          ›
        </button>
      </div>

      <Section title="Layout">
        <Field label="Chart type">
          <Segmented
            options={[
              { label: 'Candles', value: 'candles' as ChartType },
              { label: 'Line', value: 'line' as ChartType },
            ]}
            value={layout.chartType}
            onChange={layout.setChartType}
          />
        </Field>
        <ToggleRow
          label="Two panes"
          checked={layout.twoPane}
          onChange={layout.setTwoPane}
          title="Show a second, higher-timeframe pane with a crosshair linked to the top one."
        />
        <Row label="Candle px">
          <input
            type="number"
            min={2}
            max={40}
            step={1}
            value={layout.candleWidth}
            onChange={(e) => layout.setCandleWidth(Number(e.target.value))}
            style={numInput}
          />
        </Row>
      </Section>

      <Section title="Data">
        <Field label="Asset">
          <Segmented
            options={data.assets.map((a) => ({ label: a, value: a }))}
            value={data.asset}
            onChange={data.setAsset}
          />
        </Field>
        <Field label="Timeframe">
          <Segmented
            options={data.timeframes.map((t) => ({ label: t.label, value: t.stepMs }))}
            value={data.tf}
            onChange={data.setTf}
          />
        </Field>
        <ToggleRow
          label="seriesKey"
          checked={data.useSeriesKey}
          onChange={data.setUseSeriesKey}
          title="Pass seriesKey={asset} so asset switches reset explicitly; uncheck to exercise pure data-heuristic detection."
        />
        <ToggleRow
          label="Gaps"
          checked={data.gaps}
          onChange={data.setGaps}
          title="Punch interior holes into the series (non-uniform grid). Pan, then Add/Update a candle — the pan must hold."
        />
      </Section>

      <Section title="Streaming">
        <div style={{ display: 'flex', gap: 6 }}>
          <button onClick={streaming.onAddCandle} style={{ ...btn, flex: 1 }}>
            Add candle
          </button>
          <button onClick={streaming.onUpdateLast} style={{ ...btn, flex: 1 }}>
            Update last
          </button>
        </div>
        <ToggleRow
          label="Live"
          checked={streaming.live}
          onChange={streaming.setLive}
          title="Auto-stream on the interval below."
        />
        <Row label="Interval (ms)">
          <input
            type="number"
            min={100}
            step={100}
            value={streaming.intervalMs}
            onChange={(e) => streaming.setIntervalMs(Number(e.target.value))}
            style={numInput}
          />
        </Row>
        <Field label="Mode">
          <Segmented
            options={[
              { label: 'append', value: 'append' },
              { label: 'update', value: 'update' },
            ]}
            value={streaming.streamMode}
            onChange={streaming.setStreamMode}
          />
        </Field>
        <div style={{ fontSize: 12, color: MUTED, fontFamily: 'ui-monospace, monospace' }}>
          {streaming.count} candles
        </div>
      </Section>

      <Section title="Overlays">
        <ToggleRow
          label="Liquidity"
          checked={overlays.showLiquidity}
          onChange={overlays.setShowLiquidity}
          title="Overlay sample resting-order liquidity bands (buy below / sell above spot); opacity scales with volume."
        />
        {overlays.showLiquidity && (
          <Row label="Band height">
            <input
              type="range"
              min={0.1}
              max={3}
              step={0.05}
              value={overlays.bandHeight}
              onChange={(e) => overlays.setBandHeight(Number(e.target.value))}
              style={{ width: 110 }}
            />
            <span style={{ fontFamily: 'ui-monospace, monospace', fontSize: 12, width: 30 }}>
              {overlays.bandHeight.toFixed(2)}
            </span>
          </Row>
        )}
        <button
          onClick={overlays.toggleLineTool}
          style={{
            ...btn,
            textAlign: 'left',
            ...(drawing ? { background: '#1f6feb', color: '#f0f6fc', borderColor: '#1f6feb' } : {}),
          }}
        >
          Line tool{drawing ? ' (on)' : ''}
        </button>
      </Section>

      <Section title="Indicators & colors">
        <button
          onClick={panels.openIndicators}
          style={{ ...btn, display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}
        >
          <span>Indicators</span>
          {panels.activeCount > 0 && <span style={badge}>{panels.activeCount}</span>}
        </button>
        <button onClick={panels.openColors} style={{ ...btn, textAlign: 'left' }}>
          Colors
        </button>
      </Section>
    </aside>
  );
}
