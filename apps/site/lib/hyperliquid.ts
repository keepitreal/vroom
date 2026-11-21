import type { Candle } from '@vroomchart/react';

export const STEP_MS = 60_000;
export const MAX_CANDLES = 1500;
export const HL_INFO_URL = 'https://api.hyperliquid.xyz/info';
export const HL_WS_URL = 'wss://api.hyperliquid.xyz/ws';

/** Hyperliquid's wire format: numbers arrive as strings. */
export type RawCandle = { t: number; o: string; h: string; l: string; c: string; v: string };

export const fromRaw = (k: RawCandle): Candle => ({
  timeMs: k.t,
  open: +k.o,
  high: +k.h,
  low: +k.l,
  close: +k.c,
  volume: +k.v,
});

/** Compact rows for the wire and the page payload: `[timeMs, open, high, low, close, volume]`. */
export type CandleRow = [number, number, number, number, number, number];

export const toRow = (c: Candle): CandleRow => [c.timeMs, c.open, c.high, c.low, c.close, c.volume];

export const fromRow = ([timeMs, open, high, low, close, volume]: CandleRow): Candle => ({
  timeMs,
  open,
  high,
  low,
  close,
  volume,
});

/** The most recent `count` one-minute BTC candles from Hyperliquid's public API. */
export async function fetchBtcCandles(count = MAX_CANDLES, init?: RequestInit): Promise<Candle[]> {
  const endTime = Date.now();
  const res = await fetch(HL_INFO_URL, {
    ...init,
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      type: 'candleSnapshot',
      req: { coin: 'BTC', interval: '1m', startTime: endTime - count * STEP_MS, endTime },
    }),
  });
  if (!res.ok) throw new Error(`candleSnapshot ${res.status}`);
  const rows = (await res.json()) as RawCandle[];
  if (rows.length === 0) throw new Error('candleSnapshot returned no candles');
  return rows.map(fromRaw);
}
