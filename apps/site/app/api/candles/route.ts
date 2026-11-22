import type { NextRequest } from 'next/server';
import { getBtcCandles } from '@/lib/candles';
import { fetchBtcCandles, MAX_CANDLES, STEP_MS, toRow } from '@/lib/hyperliquid';

export const revalidate = 15;

/**
 * BTC 1m history in compact rows. Cached at the edge so a traffic spike costs
 * Hyperliquid one request per window, not one per visitor.
 *
 * `?since=<ms>` returns just the bars from that time on, uncached, for a
 * client filling a gap. It never falls back to the snapshot.
 */
export async function GET(request: NextRequest) {
  const since = Number(request.nextUrl.searchParams.get('since'));
  if (since) return gapFill(since);

  const { candles, live } = await getBtcCandles();
  return Response.json(
    { symbol: 'BTC-PERP', interval: '1m', live, candles: candles.map(toRow) },
    { headers: { 'Cache-Control': 'public, s-maxage=15, stale-while-revalidate=60' } },
  );
}

async function gapFill(since: number) {
  const now = Date.now();
  if (!Number.isFinite(since) || since > now || now - since > MAX_CANDLES * STEP_MS) {
    return Response.json({ error: 'since must be a past time within the history window' }, { status: 400 });
  }
  const count = Math.min(MAX_CANDLES, Math.max(2, Math.ceil((now - since) / STEP_MS) + 1));
  const noStore = { 'Cache-Control': 'no-store' };
  try {
    const candles = await fetchBtcCandles(count, { cache: 'no-store' });
    return Response.json(
      { symbol: 'BTC-PERP', interval: '1m', live: true, candles: candles.map(toRow) },
      { headers: noStore },
    );
  } catch {
    return Response.json({ error: 'Hyperliquid unavailable' }, { status: 502, headers: noStore });
  }
}
