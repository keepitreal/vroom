import { getBtcCandles } from '@/lib/candles';
import { toRow } from '@/lib/hyperliquid';

export const revalidate = 15;

/**
 * BTC 1m history in compact rows. Cached at the edge so a traffic spike costs
 * Hyperliquid one request per window, not one per visitor.
 */
export async function GET() {
  const { candles, live } = await getBtcCandles();
  return Response.json(
    { symbol: 'BTC-PERP', interval: '1m', live, candles: candles.map(toRow) },
    { headers: { 'Cache-Control': 'public, s-maxage=15, stale-while-revalidate=60' } },
  );
}
