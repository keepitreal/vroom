import 'server-only';
import type { Candle } from '@vroomchart/react';
import snapshot from './btc-snapshot.json';
import { fetchBtcCandles, fromRow, type CandleRow } from './hyperliquid';

/**
 * BTC history for server rendering. Falls back to the bundled snapshot so the
 * page still renders a chart when Hyperliquid is unreachable at build time.
 */
export async function getBtcCandles(): Promise<{ candles: Candle[]; live: boolean }> {
  try {
    return { candles: await fetchBtcCandles(undefined, { next: { revalidate: 30 } }), live: true };
  } catch {
    return { candles: (snapshot as CandleRow[]).map(fromRow), live: false };
  }
}
