'use client';

import { useEffect, useState } from 'react';
import type { Candle } from '@vroomchart/react';
import { fromRaw, fromRow, HL_WS_URL, MAX_CANDLES, STEP_MS, type CandleRow, type RawCandle } from '@/lib/hyperliquid';

export { STEP_MS };

export type FeedStatus = 'connecting' | 'live' | 'delayed';

/** Replace the in-progress bar, or append a new one. Older bars are ignored. */
function mergeTick(candles: Candle[], tick: Candle): Candle[] {
  const last = candles[candles.length - 1];
  if (!last || tick.timeMs > last.timeMs) return [...candles, tick].slice(-MAX_CANDLES);
  if (tick.timeMs === last.timeMs) return [...candles.slice(0, -1), tick];
  return candles;
}

/** Fresh history from our API. Throws when the API could only serve the bundled snapshot. */
async function backfill(): Promise<Candle[]> {
  const res = await fetch('/api/candles');
  if (!res.ok) throw new Error(`/api/candles ${res.status}`);
  const { candles, live } = (await res.json()) as { candles: CandleRow[]; live: boolean };
  if (!live) throw new Error('/api/candles served the stale snapshot');
  return candles.map(fromRow);
}

/**
 * BTC 1m candles: the server-rendered history first, then kept current over
 * Hyperliquid's websocket. A reconnect backfills whatever the gap missed.
 */
export function useBtcCandles(initial: Candle[]): { candles: Candle[]; status: FeedStatus } {
  const [candles, setCandles] = useState<Candle[]>(initial);
  const [status, setStatus] = useState<FeedStatus>('connecting');

  useEffect(() => {
    let disposed = false;
    let ws: WebSocket | null = null;
    let retry: ReturnType<typeof setTimeout> | undefined;
    let attempt = 0;
    let dropped = false;

    const refresh = () =>
      backfill()
        .then((history) => {
          if (disposed) return;
          setCandles((prev) => {
            const lastHist = history[history.length - 1]?.timeMs ?? 0;
            return [...history, ...prev.filter((c) => c.timeMs > lastHist)].slice(-MAX_CANDLES);
          });
        })
        .catch(() => {});

    const stale = Date.now() - (initial[initial.length - 1]?.timeMs ?? 0) > 5 * STEP_MS;
    if (stale) void refresh();

    const connect = () => {
      ws = new WebSocket(HL_WS_URL);
      ws.onopen = () => {
        attempt = 0;
        if (dropped) void refresh();
        dropped = false;
        ws?.send(
          JSON.stringify({ method: 'subscribe', subscription: { type: 'candle', coin: 'BTC', interval: '1m' } }),
        );
      };
      ws.onmessage = (ev) => {
        const msg = JSON.parse(ev.data as string) as { channel?: string; data?: RawCandle };
        if (msg.channel !== 'candle' || !msg.data) return;
        const tick = fromRaw(msg.data);
        setStatus('live');
        setCandles((prev) => mergeTick(prev, tick));
      };
      ws.onclose = () => {
        if (disposed) return;
        dropped = true;
        setStatus('delayed');
        retry = setTimeout(connect, Math.min(30_000, 1000 * 2 ** attempt++));
      };
    };
    connect();

    return () => {
      disposed = true;
      clearTimeout(retry);
      ws?.close();
    };
    // The server snapshot only seeds the first render.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  return { candles, status };
}

/** Roll 1m candles up into `stepMs` buckets aligned to the epoch. */
export function aggregate(candles: Candle[], stepMs: number): Candle[] {
  if (stepMs === STEP_MS) return candles;
  const out: Candle[] = [];
  for (const c of candles) {
    const bucket = c.timeMs - (c.timeMs % stepMs);
    const cur = out[out.length - 1];
    if (cur && cur.timeMs === bucket) {
      cur.high = Math.max(cur.high, c.high);
      cur.low = Math.min(cur.low, c.low);
      cur.close = c.close;
      cur.volume += c.volume;
    } else {
      out.push({ ...c, timeMs: bucket });
    }
  }
  return out;
}

/** Change over the last 24h of 1m candles (or the whole series if shorter). */
export function dayChange(candles: Candle[]): number {
  const last = candles[candles.length - 1];
  const ref = candles[Math.max(0, candles.length - 1441)];
  return last && ref ? (last.close - ref.close) / ref.close : 0;
}

export const formatPrice = (p: number) =>
  p.toLocaleString('en-US', { minimumFractionDigits: 1, maximumFractionDigits: 1 });

export const formatChange = (r: number) => `${r >= 0 ? '+' : ''}${(r * 100).toFixed(2)}%`;
