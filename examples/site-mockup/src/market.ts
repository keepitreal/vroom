import { useEffect, useState } from 'react';
import type { Candle } from '@vroomchart/react';
import snapshot from './snapshot.json';

export const STEP_MS = 60_000;
const MAX_CANDLES = 1500;
const INFO_URL = 'https://api.hyperliquid.xyz/info';
const WS_URL = 'wss://api.hyperliquid.xyz/ws';

export type FeedStatus = 'connecting' | 'live' | 'delayed';

type RawCandle = { t: number; o: string; h: string; l: string; c: string; v: string };

const fromRaw = (k: RawCandle): Candle => ({
  timeMs: k.t,
  open: +k.o,
  high: +k.h,
  low: +k.l,
  close: +k.c,
  volume: +k.v,
});

/** Rows are `[timeMs, open, high, low, close, volume]`. */
const snapshotCandles = (): Candle[] =>
  (snapshot as number[][]).map(([timeMs, open, high, low, close, volume]) => ({
    timeMs,
    open,
    high,
    low,
    close,
    volume,
  }));

async function fetchHistory(): Promise<Candle[]> {
  const endTime = Date.now();
  const res = await fetch(INFO_URL, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      type: 'candleSnapshot',
      req: { coin: 'BTC', interval: '1m', startTime: endTime - MAX_CANDLES * STEP_MS, endTime },
    }),
  });
  if (!res.ok) throw new Error(`candleSnapshot ${res.status}`);
  const rows = (await res.json()) as RawCandle[];
  if (rows.length === 0) throw new Error('candleSnapshot returned no candles');
  return rows.map(fromRaw);
}

/** Replace the in-progress bar, or append a new one. Older bars are ignored. */
function mergeTick(candles: Candle[], tick: Candle): Candle[] {
  const last = candles[candles.length - 1];
  if (!last || tick.timeMs > last.timeMs) return [...candles, tick].slice(-MAX_CANDLES);
  if (tick.timeMs === last.timeMs) return [...candles.slice(0, -1), tick];
  return candles;
}

/**
 * BTC 1m candles: the bundled snapshot first, swapped for Hyperliquid's history
 * once it loads, then kept current over the websocket.
 */
export function useBtcCandles(): { candles: Candle[]; status: FeedStatus } {
  const [candles, setCandles] = useState<Candle[]>(snapshotCandles);
  const [status, setStatus] = useState<FeedStatus>('connecting');

  useEffect(() => {
    let disposed = false;
    let ws: WebSocket | null = null;
    let retry: ReturnType<typeof setTimeout> | undefined;
    let attempt = 0;

    fetchHistory()
      .then((history) => !disposed && setCandles(history))
      .catch(() => !disposed && setStatus('delayed'));

    const connect = () => {
      ws = new WebSocket(WS_URL);
      ws.onopen = () => {
        attempt = 0;
        ws?.send(
          JSON.stringify({
            method: 'subscribe',
            subscription: { type: 'candle', coin: 'BTC', interval: '1m' },
          }),
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
