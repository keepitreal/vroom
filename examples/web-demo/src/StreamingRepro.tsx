import { useCallback, useEffect, useRef, useState } from 'react';
import { VroomChart, type Candle } from '@vroomchart/react';

const DAY = 24 * 60 * 60 * 1000;

// Synthetic random-walk candles, same shape as the main demo.
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

// Append a brand-new candle that continues the random walk from the last close.
function appendCandle(prev: Candle[]): Candle[] {
  const last = prev[prev.length - 1];
  const open = last.close;
  const close = open + (Math.random() - 0.5) * 4;
  const high = Math.max(open, close) + Math.random() * 2;
  const low = Math.min(open, close) - Math.random() * 2;
  const next: Candle = {
    timeMs: last.timeMs + DAY,
    open,
    high,
    low,
    close,
    volume: Math.random() * 1000,
  };
  return [...prev, next];
}

// Mutate the most recent candle (simulates an in-progress bar ticking).
function updateLast(prev: Candle[]): Candle[] {
  if (prev.length === 0) return prev;
  const last = prev[prev.length - 1];
  const close = last.open + (Math.random() - 0.5) * 4;
  const high = Math.max(last.high, last.open, close) + Math.random() * 1;
  const low = Math.min(last.low, last.open, close) - Math.random() * 1;
  const updated: Candle = { ...last, close, high, low };
  return [...prev.slice(0, -1), updated];
}

const btn: React.CSSProperties = {
  background: '#21262d',
  color: '#c9d1d9',
  border: '1px solid #30363d',
  borderRadius: 6,
  padding: '6px 12px',
  fontSize: 13,
  cursor: 'pointer',
};

export function StreamingRepro() {
  const [candles, setCandles] = useState<Candle[]>(() => mockCandles(300, DAY));
  const [live, setLive] = useState(false);
  const [intervalMs, setIntervalMs] = useState(1000);
  // 'append' = a new bar arrives; 'update' = the current bar ticks.
  const [mode, setMode] = useState<'append' | 'update'>('append');

  const tick = useCallback(() => {
    setCandles((prev) => (mode === 'append' ? appendCandle(prev) : updateLast(prev)));
  }, [mode]);

  // Keep the latest tick in a ref so the interval doesn't restart on every change.
  const tickRef = useRef(tick);
  tickRef.current = tick;

  useEffect(() => {
    if (!live) return;
    const id = setInterval(() => tickRef.current(), Math.max(50, intervalMs));
    return () => clearInterval(id);
  }, [live, intervalMs]);

  return (
    <div style={{ height: '100%', display: 'flex', flexDirection: 'column' }}>
      <div
        style={{
          padding: '10px 14px',
          display: 'flex',
          gap: 12,
          alignItems: 'center',
          flexWrap: 'wrap',
          fontSize: 13,
        }}
      >
        <button style={btn} onClick={() => setCandles((p) => appendCandle(p))}>
          Add candle
        </button>
        <button style={btn} onClick={() => setCandles((p) => updateLast(p))}>
          Update last candle
        </button>
        <label style={{ display: 'flex', alignItems: 'center', gap: 6, cursor: 'pointer' }}>
          <input type="checkbox" checked={live} onChange={(e) => setLive(e.target.checked)} />
          Live
        </label>
        <label style={{ display: 'flex', alignItems: 'center', gap: 6 }}>
          every
          <input
            type="number"
            min={50}
            step={50}
            value={intervalMs}
            onChange={(e) => setIntervalMs(Number(e.target.value) || 0)}
            style={{
              width: 70,
              background: '#0d1117',
              color: '#c9d1d9',
              border: '1px solid #30363d',
              borderRadius: 6,
              padding: '4px 6px',
            }}
          />
          ms
        </label>
        <label style={{ display: 'flex', alignItems: 'center', gap: 6 }}>
          mode
          <select
            value={mode}
            onChange={(e) => setMode(e.target.value as 'append' | 'update')}
            style={{
              background: '#0d1117',
              color: '#c9d1d9',
              border: '1px solid #30363d',
              borderRadius: 6,
              padding: '4px 6px',
            }}
          >
            <option value="append">append (new bar)</option>
            <option value="update">update (last bar)</option>
          </select>
        </label>
        <span style={{ opacity: 0.6, fontFamily: 'ui-monospace, monospace' }}>
          {candles.length} candles
        </span>
      </div>
      <div
        style={{
          padding: '0 14px 8px',
          fontSize: 12,
          opacity: 0.8,
          lineHeight: 1.5,
        }}
      >
        Repro: 1) Drag the chart up/down (or drag the price axis) to translate it in Y. 2) Click
        &ldquo;Add candle&rdquo; or enable &ldquo;Live&rdquo;. Bug: the Y translation snaps back to the
        auto-fit bounds on each new candle instead of staying where you put it.
      </div>
      <div style={{ flex: 1, minHeight: 0, padding: '0 8px 8px' }}>
        <VroomChart candles={candles} />
      </div>
    </div>
  );
}
