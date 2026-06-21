import { useMemo, useState } from 'react';
import { VroomChart, type Candle, type CrosshairEvent } from '@vroom/react';

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
  // No wasm/asset config needed — @vroom/react defaults to the Skia-WASM core
  // bundled in @vroom/core-wasm (and falls back to the Canvas2D stub on failure).
  const candles = useMemo(() => mockCandles(300, DAY), []);
  const [readout, setReadout] = useState<string>('hover / long-press for crosshair');

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
      <div style={{ padding: '10px 14px', display: 'flex', gap: 16, alignItems: 'baseline' }}>
        <strong style={{ fontSize: 16 }}>Vroom</strong>
        <span style={{ fontFamily: 'ui-monospace, monospace', fontSize: 13, opacity: 0.85 }}>{readout}</span>
      </div>
      <div style={{ flex: 1, minHeight: 0, padding: '0 8px 8px' }}>
        <VroomChart candles={candles} onCrosshair={onCrosshair} />
      </div>
    </div>
  );
}
