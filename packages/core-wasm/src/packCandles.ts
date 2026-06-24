import type { Candle } from '@vroomchart/types';

// Wire format must match `VroomCandle` in packages/core/include/vroom/vroom_chart.h:
//   int64_t time_ms;  double open, high, low, close, volume;
// = 48 bytes per candle, 8-byte aligned, little-endian.
//
// This is a deliberate mirror of packages/react-native/src/packCandles.ts so
// the web side speaks the same binary ABI the Skia-WASM core will decode. Keep
// the two in sync with the C struct.
export const BYTES_PER_CANDLE = 48;

/** Serialize candles into the packed little-endian buffer the core expects. */
export function packCandles(candles: Candle[]): ArrayBuffer {
  const buf = new ArrayBuffer(candles.length * BYTES_PER_CANDLE);
  const view = new DataView(buf);
  for (let i = 0; i < candles.length; i++) {
    const c = candles[i]!;
    const off = i * BYTES_PER_CANDLE;
    view.setBigInt64(off, BigInt(Math.trunc(c.timeMs)), true);
    view.setFloat64(off + 8, c.open, true);
    view.setFloat64(off + 16, c.high, true);
    view.setFloat64(off + 24, c.low, true);
    view.setFloat64(off + 32, c.close, true);
    view.setFloat64(off + 40, c.volume, true);
  }
  return buf;
}

/** Decode the packed buffer back to candles (inverse of packCandles). */
export function unpackCandles(buf: ArrayBuffer): Candle[] {
  const view = new DataView(buf);
  const n = Math.floor(buf.byteLength / BYTES_PER_CANDLE);
  const out: Candle[] = new Array(n);
  for (let i = 0; i < n; i++) {
    const off = i * BYTES_PER_CANDLE;
    out[i] = {
      timeMs: Number(view.getBigInt64(off, true)),
      open: view.getFloat64(off + 8, true),
      high: view.getFloat64(off + 16, true),
      low: view.getFloat64(off + 24, true),
      close: view.getFloat64(off + 32, true),
      volume: view.getFloat64(off + 40, true),
    };
  }
  return out;
}
