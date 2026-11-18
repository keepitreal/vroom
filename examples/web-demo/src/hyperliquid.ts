import { useEffect, useState } from 'react';
import type { Candle } from '@vroomchart/react';

// Live index perps from Hyperliquid's `xyz` (trade.xyz) dex: the NQ and S&P 500
// markets the ORB assistant is usually run on. Public endpoints, no key; reached
// through the Vite proxy (vite.config.ts) so the page stays same-origin.
export const LIVE_MARKETS = { XYZ100: 'xyz:XYZ100', SP500: 'xyz:SP500' } as const;
export type LiveAsset = keyof typeof LIVE_MARKETS;

const INFO_URL = '/hyperliquid/info';
const wsUrl = () => `${location.protocol === 'https:' ? 'wss' : 'ws'}://${location.host}/hyperliquid/ws`;
// candleSnapshot returns at most 5000 bars per request.
const MAX_BARS = 5000;

const INTERVALS: Record<number, string> = {
  60_000: '1m',
  300_000: '5m',
  900_000: '15m',
  3_600_000: '1h',
};

type HlCandle = { t: number; o: string; h: string; l: string; c: string; v: string };

const toCandle = (c: HlCandle): Candle => ({
  timeMs: c.t,
  open: Number(c.o),
  high: Number(c.h),
  low: Number(c.l),
  close: Number(c.c),
  volume: Number(c.v),
});

async function fetchHistory(coin: string, stepMs: number, signal: AbortSignal): Promise<Candle[]> {
  const endTime = Date.now();
  const res = await fetch(INFO_URL, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      type: 'candleSnapshot',
      req: { coin, interval: INTERVALS[stepMs], startTime: endTime - MAX_BARS * stepMs, endTime },
    }),
    signal,
  });
  if (!res.ok) throw new Error(`candleSnapshot ${coin} ${INTERVALS[stepMs]}: HTTP ${res.status}`);
  return ((await res.json()) as HlCandle[]).map(toCandle);
}

// Replaces the bar with the same open time, or appends a newer one.
function merge(prev: Candle[], c: Candle): Candle[] {
  const last = prev[prev.length - 1];
  if (!last || c.timeMs > last.timeMs) return [...prev, c];
  if (c.timeMs === last.timeMs) return [...prev.slice(0, -1), c];
  return prev;
}

/**
 * History for `asset` at `stepMs`, then live updates from the candle websocket.
 * `null` while the history is loading (or when `asset` is null).
 */
export function useHyperliquidCandles(asset: LiveAsset | null, stepMs: number): Candle[] | null {
  const [candles, setCandles] = useState<Candle[] | null>(null);

  useEffect(() => {
    setCandles(null);
    if (!asset || !INTERVALS[stepMs]) return;
    const coin = LIVE_MARKETS[asset];
    const interval = INTERVALS[stepMs];
    const abort = new AbortController();
    // Updates that arrive before the history lands are replayed onto it.
    const early: Candle[] = [];
    let loaded = false;

    const ws = new WebSocket(wsUrl());
    ws.onopen = () =>
      ws.send(JSON.stringify({ method: 'subscribe', subscription: { type: 'candle', coin, interval } }));
    ws.onmessage = (m) => {
      const msg = JSON.parse(String(m.data)) as { channel: string; data: HlCandle & { s: string; i: string } };
      if (msg.channel !== 'candle' || msg.data.s !== coin || msg.data.i !== interval) return;
      const c = toCandle(msg.data);
      if (loaded) setCandles((p) => (p ? merge(p, c) : p));
      else early.push(c);
    };

    fetchHistory(coin, stepMs, abort.signal)
      .then((history) => {
        loaded = true;
        setCandles(early.reduce(merge, history));
      })
      .catch((e: unknown) => {
        if (!abort.signal.aborted) console.error('Hyperliquid history fetch failed', asset, interval, e);
      });

    return () => {
      abort.abort();
      ws.close();
    };
  }, [asset, stepMs]);

  return candles;
}
