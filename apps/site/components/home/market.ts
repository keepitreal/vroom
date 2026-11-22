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

/** Splice fetched bars in by timestamp: they win where they overlap, newer live bars are kept. */
function mergeHistory(prev: Candle[], fetched: Candle[]): Candle[] {
  if (fetched.length === 0) return prev;
  const first = fetched[0].timeMs;
  const last = fetched[fetched.length - 1].timeMs;
  return [
    ...prev.filter((c) => c.timeMs < first),
    ...fetched,
    ...prev.filter((c) => c.timeMs > last),
  ].slice(-MAX_CANDLES);
}

/** Every bar from `sinceMs` on, straight from Hyperliquid via our uncached gap-fill endpoint. */
async function fetchSince(sinceMs: number): Promise<Candle[]> {
  const res = await fetch(`/api/candles?since=${sinceMs}`);
  if (!res.ok) throw new Error(`/api/candles?since ${res.status}`);
  const { candles } = (await res.json()) as { candles: CandleRow[] };
  return candles.map(fromRow);
}

/** A socket this quiet after sleep or a background tab is presumed dead. */
const SILENT_MS = 20_000;

/**
 * BTC 1m candles: the server-rendered history first, then kept current over
 * Hyperliquid's websocket. Missed minutes (a reconnect, a tick that skips
 * ahead, a tab coming back from sleep) are filled from the last bar we hold.
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
    let filling = false;
    let lastBarMs = initial[initial.length - 1]?.timeMs ?? 0;
    let lastMsgAt = Date.now();

    const fill = (sinceMs = lastBarMs) => {
      if (filling || !sinceMs) return;
      filling = true;
      fetchSince(sinceMs)
        .then((fetched) => {
          if (disposed || fetched.length === 0) return;
          lastBarMs = Math.max(lastBarMs, fetched[fetched.length - 1].timeMs);
          setCandles((prev) => mergeHistory(prev, fetched));
        })
        .catch(() => {})
        .finally(() => {
          filling = false;
        });
    };

    if (Date.now() - lastBarMs > 2 * STEP_MS) fill();

    const connect = () => {
      ws = new WebSocket(HL_WS_URL);
      ws.onopen = () => {
        attempt = 0;
        lastMsgAt = Date.now();
        if (dropped) fill();
        dropped = false;
        ws?.send(
          JSON.stringify({ method: 'subscribe', subscription: { type: 'candle', coin: 'BTC', interval: '1m' } }),
        );
      };
      ws.onmessage = (ev) => {
        lastMsgAt = Date.now();
        const msg = JSON.parse(ev.data as string) as { channel?: string; data?: RawCandle };
        if (msg.channel !== 'candle' || !msg.data) return;
        const tick = fromRaw(msg.data);
        const gapFrom = lastBarMs && tick.timeMs > lastBarMs + STEP_MS ? lastBarMs : 0;
        lastBarMs = Math.max(lastBarMs, tick.timeMs);
        setStatus('live');
        setCandles((prev) => mergeTick(prev, tick));
        if (gapFrom) fill(gapFrom);
      };
      ws.onclose = () => {
        if (disposed) return;
        dropped = true;
        setStatus('delayed');
        retry = setTimeout(connect, Math.min(30_000, 1000 * 2 ** attempt++));
      };
    };
    connect();

    const resync = () => {
      if (document.visibilityState !== 'visible') return;
      if (ws?.readyState === WebSocket.OPEN && Date.now() - lastMsgAt > SILENT_MS) {
        // A dead socket can take ages to report its close, so drop it and dial again now.
        const dead = ws;
        dead.onclose = dead.onmessage = null;
        dead.close();
        dropped = true;
        clearTimeout(retry);
        connect();
      } else if (Date.now() - lastBarMs > 2 * STEP_MS) {
        fill();
      }
    };
    document.addEventListener('visibilitychange', resync);
    addEventListener('online', resync);

    return () => {
      disposed = true;
      clearTimeout(retry);
      document.removeEventListener('visibilitychange', resync);
      removeEventListener('online', resync);
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
