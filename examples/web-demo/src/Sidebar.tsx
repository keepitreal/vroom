import { type CSSProperties, type ReactNode, useState } from 'react';
import type {
  ChartMode,
  ChartType,
  DrawTool,
  PriceScaleMode,
  TransitionEasing,
  IntervalTransition,
  StreamTransition,
  UndoRedoState,
} from '@vroomchart/react';

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

const select: CSSProperties = {
  ...numInput,
  width: 120,
  fontFamily: 'inherit',
};

/**
 * The price-line style override. `'mixed'` keeps whatever `lineStyle` each
 * sample line declares; the rest force that one style onto every line.
 */
export type PriceLineStyleChoice = 'mixed' | 'solid' | 'dotted' | 'dashed';

const PRICE_LINE_STYLES: readonly PriceLineStyleChoice[] = [
  'mixed',
  'solid',
  'dotted',
  'dashed',
];

/** Demo settings for the crosshair plus button (order entry). */
export type CrosshairButtonChoice = {
  enabled: boolean;
  ring: boolean;
  cornerRadius: number;
  size: number;
  background: CrosshairButtonBackground;
};

/** Background presets; 'theme' leaves the color to the chart. */
export type CrosshairButtonBackground = 'theme' | 'blue' | 'green' | 'slate';

const CROSSHAIR_BUTTON_BACKGROUNDS: readonly CrosshairButtonBackground[] = [
  'theme',
  'blue',
  'green',
  'slate',
];

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
    axisFontSize: number;
    setAxisFontSize: (v: number) => void;
    chartType: ChartType;
    setChartType: (v: ChartType) => void;
    priceScaleMode: PriceScaleMode;
    setPriceScaleMode: (v: PriceScaleMode) => void;
  };
  animation: {
    transitionMs: number;
    setTransitionMs: (v: number) => void;
    easing: TransitionEasing;
    setEasing: (v: TransitionEasing) => void;
    intervalTransition: IntervalTransition;
    setIntervalTransition: (v: IntervalTransition) => void;
    streamTransition: StreamTransition;
    setStreamTransition: (v: StreamTransition) => void;
    streamTransitionMs: number;
    setStreamTransitionMs: (v: number) => void;
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
    sparse: boolean;
    setSparse: (v: boolean) => void;
    listingCounts: readonly number[];
    listingCount: number;
    setListingCount: (n: number) => void;
    backfill: boolean;
    setBackfill: (v: boolean) => void;
    loading: boolean;
    setLoading: (v: boolean) => void;
    onSimulateLoad: () => void;
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
    showPriceLines: boolean;
    setShowPriceLines: (v: boolean) => void;
    priceLineStyle: PriceLineStyleChoice;
    setPriceLineStyle: (v: PriceLineStyleChoice) => void;
    priceLineFontSize: number;
    setPriceLineFontSize: (v: number) => void;
    priceLineCornerRadius: number;
    setPriceLineCornerRadius: (v: number) => void;
    showFootprints: boolean;
    setShowFootprints: (v: boolean) => void;
    crosshairButton: CrosshairButtonChoice;
    setCrosshairButton: (v: CrosshairButtonChoice) => void;
    drawMode: ChartMode;
    drawTool: DrawTool;
    toggleLineTool: () => void;
    toggleBoxTool: () => void;
    togglePencilTool: () => void;
    togglePathTool: () => void;
    history: UndoRedoState;
    undoDrawing: () => void;
    redoDrawing: () => void;
  };
  panels: {
    activeCount: number;
    openIndicators: () => void;
    openColors: () => void;
  };
  onCollapse: () => void;
};

export function Sidebar({
  layout,
  animation,
  data,
  streaming,
  overlays,
  panels,
  onCollapse,
}: SidebarProps) {
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
        <Field label="Price scale">
          <Segmented
            options={[
              { label: 'Linear', value: 'linear' as PriceScaleMode },
              { label: 'Log', value: 'log' as PriceScaleMode },
            ]}
            value={layout.priceScaleMode}
            onChange={layout.setPriceScaleMode}
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
        <Row label="Axis px">
          <input
            type="number"
            min={10}
            max={14}
            step={1}
            value={layout.axisFontSize}
            onChange={(e) => {
              const n = Number(e.target.value);
              if (!Number.isFinite(n)) return;
              layout.setAxisFontSize(Math.min(14, Math.max(10, Math.round(n))));
            }}
            style={numInput}
            title="Axis label size in px. 11 is the default; the chart clamps anything outside 10–14. Price-line pills follow this unless Font px is set."
          />
        </Row>
      </Section>

      <Section title="Animations">
        <Row label={`Duration ${animation.transitionMs}ms`}>
          <input
            type="range"
            min={0}
            max={1000}
            step={50}
            value={animation.transitionMs}
            onChange={(e) => animation.setTransitionMs(Number(e.target.value))}
            style={{ width: 120 }}
          />
        </Row>
        <Field label="Interval switch">
          <Segmented
            options={[
              { label: 'Transform', value: 'transform' as IntervalTransition },
              { label: 'Fade', value: 'fade' as IntervalTransition },
            ]}
            value={animation.intervalTransition}
            onChange={animation.setIntervalTransition}
          />
        </Field>
        <Field label="Live update">
          <Segmented
            options={[
              { label: 'None', value: 'none' as StreamTransition },
              { label: 'Transform', value: 'transform' as StreamTransition },
            ]}
            value={animation.streamTransition}
            onChange={animation.setStreamTransition}
          />
        </Field>
        <Row label={`Live duration ${animation.streamTransitionMs}ms`}>
          <input
            type="range"
            min={0}
            max={600}
            step={25}
            value={animation.streamTransitionMs}
            onChange={(e) => animation.setStreamTransitionMs(Number(e.target.value))}
            style={{ width: 120 }}
          />
        </Row>
        <Row label="Easing">
          <select
            value={animation.easing}
            onChange={(e) => animation.setEasing(e.target.value as TransitionEasing)}
            style={select}
          >
            {(['linear', 'ease-in', 'ease-out', 'ease-in-out'] as TransitionEasing[]).map((e) => (
              <option key={e} value={e}>
                {e}
              </option>
            ))}
          </select>
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
        <ToggleRow
          label="Sparse (9 bars)"
          checked={data.sparse}
          onChange={data.setSparse}
          title="Keep only the last 9 candles — a window longer than the data. Bars should stay defaultCandleWidth on the right (empty past on the left); a pan must not snap them to the left edge or lock."
        />
        <Field label="New listing (bars)">
          <Segmented
            options={data.listingCounts.map((n) => ({ label: n === 0 ? 'Off' : String(n), value: n }))}
            value={data.listingCount}
            onChange={data.setListingCount}
          />
        </Field>
        <ToggleRow
          label="Backfill with zeros"
          checked={data.backfill}
          onChange={data.setBackfill}
          title="With New listing on: pad the pre-launch history with zero-price candles (as apps do so a new token frames like a mature chart) instead of passing only the real bars. Overrides Sparse."
        />
        <ToggleRow
          label="Loading"
          checked={data.loading}
          onChange={data.setLoading}
          title="Hold the chart in its loading state: one grey line drifting in a sine wave, with no axis text, price badge, crosshair or gestures. Unchecking delivers the data — the line should reshape into the series' silhouette, then fade out as the candles grow outward from it."
        />
        <button onClick={data.onSimulateLoad} style={{ ...btn, width: '100%' }}>
          Simulate load (1.5s)
        </button>
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
        <ToggleRow
          label="Price lines"
          checked={overlays.showPriceLines}
          onChange={overlays.setShowPriceLines}
          title="Overlay sample order/position status lines. Drag the limit order to reprice it, or click its × to cancel."
        />
        {overlays.showPriceLines && (
          <Row label="Line style">
            <select
              value={overlays.priceLineStyle}
              onChange={(e) =>
                overlays.setPriceLineStyle(e.target.value as PriceLineStyleChoice)
              }
              style={select}
              title="Force one style onto every line. 'mixed' keeps each sample's own: dashed limit buy, dotted take-profit, solid liquidation."
            >
              {PRICE_LINE_STYLES.map((s) => (
                <option key={s} value={s}>
                  {s}
                </option>
              ))}
            </select>
          </Row>
        )}
        {overlays.showPriceLines && (
          <Row label="Font px">
            <input
              type="number"
              min={10}
              max={14}
              step={1}
              value={overlays.priceLineFontSize}
              onChange={(e) => {
                const n = Number(e.target.value);
                if (!Number.isFinite(n)) return;
                overlays.setPriceLineFontSize(Math.min(14, Math.max(10, Math.round(n))));
              }}
              style={numInput}
              title="Price-line label size in px. 11 matches the axis labels; the chart clamps anything outside 10–14."
            />
          </Row>
        )}
        {overlays.showPriceLines && (
          <Row label="Corner">
            <input
              type="number"
              min={0}
              max={6}
              step={1}
              value={overlays.priceLineCornerRadius}
              onChange={(e) => {
                const n = Number(e.target.value);
                if (!Number.isFinite(n)) return;
                overlays.setPriceLineCornerRadius(Math.min(6, Math.max(0, n)));
              }}
              style={numInput}
              title="Label-pill corner radius in px. 6 is the default; 0 is square. The chart clamps anything above 6."
            />
          </Row>
        )}
        <ToggleRow
          label="Footprints"
          checked={overlays.showFootprints}
          onChange={overlays.setShowFootprints}
          title="Mark sample entries (+) and exits (−) above the bar they filled in. Hover a badge for the trade details; switch interval to watch them regroup."
        />
        <ToggleRow
          label="Crosshair +"
          checked={overlays.crosshairButton.enabled}
          onChange={(enabled) => overlays.setCrosshairButton({ ...overlays.crosshairButton, enabled })}
          title="Show a plus button on the crosshair, left of the price badge. Click it to pin the crosshair and open a buy/sell limit menu at that price."
        />
        {overlays.crosshairButton.enabled && (
          <>
            <ToggleRow
              label="Ring"
              checked={overlays.crosshairButton.ring}
              onChange={(ring) => overlays.setCrosshairButton({ ...overlays.crosshairButton, ring })}
              title="Draw a circle around the plus glyph."
            />
            <Row label="Corner">
              <input
                type="number"
                min={0}
                max={10}
                step={1}
                value={overlays.crosshairButton.cornerRadius}
                onChange={(e) => {
                  const n = Number(e.target.value);
                  if (!Number.isFinite(n)) return;
                  overlays.setCrosshairButton({
                    ...overlays.crosshairButton,
                    cornerRadius: Math.min(10, Math.max(0, n)),
                  });
                }}
                style={numInput}
                title="Container corner radius in px. The chart clamps it to half the size (a circle)."
              />
            </Row>
            <Row label="Size">
              <input
                type="number"
                min={16}
                max={28}
                step={1}
                value={overlays.crosshairButton.size}
                onChange={(e) => {
                  const n = Number(e.target.value);
                  if (!Number.isFinite(n)) return;
                  overlays.setCrosshairButton({
                    ...overlays.crosshairButton,
                    size: Math.min(28, Math.max(16, Math.round(n))),
                  });
                }}
                style={numInput}
                title="Square container side in px."
              />
            </Row>
            <Row label="Background">
              <select
                value={overlays.crosshairButton.background}
                onChange={(e) =>
                  overlays.setCrosshairButton({
                    ...overlays.crosshairButton,
                    background: e.target.value as CrosshairButtonBackground,
                  })
                }
                style={select}
              >
                {CROSSHAIR_BUTTON_BACKGROUNDS.map((b) => (
                  <option key={b} value={b}>
                    {b}
                  </option>
                ))}
              </select>
            </Row>
          </>
        )}
        <div style={{ display: 'flex', gap: 8 }}>
          <button
            onClick={overlays.toggleLineTool}
            style={{
              ...btn,
              flex: 1,
              ...(drawing && overlays.drawTool === 'line'
                ? { background: '#1f6feb', color: '#f0f6fc', border: '1px solid #1f6feb' }
                : {}),
            }}
          >
            Line (L)
          </button>
          <button
            onClick={overlays.toggleBoxTool}
            style={{
              ...btn,
              flex: 1,
              ...(drawing && overlays.drawTool === 'box'
                ? { background: '#1f6feb', color: '#f0f6fc', border: '1px solid #1f6feb' }
                : {}),
            }}
          >
            Box (R)
          </button>
          <button
            onClick={overlays.togglePencilTool}
            style={{
              ...btn,
              flex: 1,
              ...(drawing && overlays.drawTool === 'pencil'
                ? { background: '#1f6feb', color: '#f0f6fc', border: '1px solid #1f6feb' }
                : {}),
            }}
          >
            Pencil (P)
          </button>
          <button
            onClick={overlays.togglePathTool}
            style={{
              ...btn,
              flex: 1,
              ...(drawing && overlays.drawTool === 'path'
                ? { background: '#1f6feb', color: '#f0f6fc', border: '1px solid #1f6feb' }
                : {}),
            }}
            title="Multi-segment path: click each vertex, then Esc / double-click / right-click to finish"
          >
            Path (A)
          </button>
        </div>
        <div style={{ display: 'flex', gap: 8 }}>
          <button
            onClick={overlays.undoDrawing}
            disabled={!overlays.history.canUndo}
            style={{ ...btn, flex: 1, ...(overlays.history.canUndo ? {} : { opacity: 0.4, cursor: 'default' }) }}
            title="Undo the last drawing action (⌘Z / Ctrl+Z)"
          >
            ↩ Undo
          </button>
          <button
            onClick={overlays.redoDrawing}
            disabled={!overlays.history.canRedo}
            style={{ ...btn, flex: 1, ...(overlays.history.canRedo ? {} : { opacity: 0.4, cursor: 'default' }) }}
            title="Redo the last undone drawing action (⇧⌘Z / Ctrl+Y)"
          >
            ↪ Redo
          </button>
        </div>
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
