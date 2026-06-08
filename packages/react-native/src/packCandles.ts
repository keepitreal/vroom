import type { Candle } from './types';

// Wire format must match `VroomCandle` in packages/core/include/vroom/vroom_chart.h:
//   int64_t time_ms;  double open, high, low, close, volume;
// = 48 bytes per candle, 8-byte aligned, little-endian on iOS/Android.
export const BYTES_PER_CANDLE = 48;

// Serializes candles into the packed little-endian buffer the C++ core expects.
// Pure (no native/Skia deps) so it can be unit-tested in isolation.
export function packCandles(candles: Candle[]): ArrayBuffer {
  const buf = new ArrayBuffer(candles.length * BYTES_PER_CANDLE);
  const view = new DataView(buf);
  for (let i = 0; i < candles.length; i++) {
    const c = candles[i]!;
    const off = i * BYTES_PER_CANDLE;
    view.setBigInt64(off, BigInt(c.timeMs), true);
    view.setFloat64(off + 8, c.open, true);
    view.setFloat64(off + 16, c.high, true);
    view.setFloat64(off + 24, c.low, true);
    view.setFloat64(off + 32, c.close, true);
    view.setFloat64(off + 40, c.volume, true);
  }
  return buf;
}
