import { describe, expect, it } from 'vitest';

import { BYTES_PER_CANDLE, packCandles } from './packCandles';
import type { Candle } from './types';

const candle = (over: Partial<Candle> = {}): Candle => ({
  timeMs: 1_700_000_000_000,
  open: 100,
  high: 110,
  low: 95,
  close: 105,
  volume: 1234,
  ...over,
});

describe('packCandles', () => {
  it('emits 48 bytes per candle', () => {
    expect(BYTES_PER_CANDLE).toBe(48);
    expect(packCandles([candle(), candle()]).byteLength).toBe(96);
  });

  it('returns an empty buffer for no candles', () => {
    expect(packCandles([]).byteLength).toBe(0);
  });

  it('lays out fields little-endian at the expected offsets', () => {
    const c = candle({
      timeMs: 1_700_000_000_123,
      open: 1.5,
      high: 2.5,
      low: 0.5,
      close: 2.0,
      volume: 9999.25,
    });
    const view = new DataView(packCandles([c]));
    expect(view.getBigInt64(0, true)).toBe(BigInt(c.timeMs));
    expect(view.getFloat64(8, true)).toBe(c.open);
    expect(view.getFloat64(16, true)).toBe(c.high);
    expect(view.getFloat64(24, true)).toBe(c.low);
    expect(view.getFloat64(32, true)).toBe(c.close);
    expect(view.getFloat64(40, true)).toBe(c.volume);
  });

  it('preserves a large epoch-ms timestamp exactly', () => {
    // Beyond 2^53; would lose precision as a JS number but BigInt is exact.
    const timeMs = 9_007_199_254_740_993;
    const view = new DataView(packCandles([candle({ timeMs })]));
    expect(view.getBigInt64(0, true)).toBe(BigInt(timeMs));
  });

  it('round-trips negative and fractional values', () => {
    const c = candle({ open: -0.0001, low: -50.5, close: 12.3456789, volume: -7 });
    const view = new DataView(packCandles([c]));
    expect(view.getFloat64(8, true)).toBe(c.open);
    expect(view.getFloat64(24, true)).toBe(c.low);
    expect(view.getFloat64(32, true)).toBe(c.close);
    expect(view.getFloat64(40, true)).toBe(c.volume);
  });

  it('uses the correct stride for multiple candles', () => {
    const a = candle({ timeMs: 1000, open: 1 });
    const b = candle({ timeMs: 2000, open: 2 });
    const c = candle({ timeMs: 3000, open: 3 });
    const view = new DataView(packCandles([a, b, c]));
    for (let i = 0; i < 3; i++) {
      const off = i * BYTES_PER_CANDLE;
      expect(view.getBigInt64(off, true)).toBe(BigInt((i + 1) * 1000));
      expect(view.getFloat64(off + 8, true)).toBe(i + 1);
    }
  });
});
