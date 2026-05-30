import { useEffect, useRef, useState } from 'react';
import type { SkPicture } from '@shopify/react-native-skia';

import NativeVroomChart from './NativeVroomChart';
import type { ChartHandle } from './jsi.d';
import type { Candle, VisibleRange } from './types';

// Wire format must match `VroomCandle` in packages/core/include/vroom/vroom_chart.h:
//   int64_t time_ms;  double open, high, low, close, volume;
// = 48 bytes per candle, 8-byte aligned, little-endian on iOS/Android.
const BYTES_PER_CANDLE = 48;

let installed = false;
function ensureInstalled(): void {
  if (installed) return;
  const ok = NativeVroomChart.install();
  if (!ok) throw new Error('VroomChartModule.install() returned false');
  if (typeof globalThis.VroomChartJSI === 'undefined') {
    throw new Error('global.VroomChartJSI undefined after install()');
  }
  installed = true;
}

function packCandles(candles: Candle[]): ArrayBuffer {
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

export type ChartCoreState = {
  handle: ChartHandle | null;
  /** Picture freshly rendered after the latest data/size/range push. */
  picture: SkPicture | null;
};

// Owns a ChartHandle and produces an "initial" picture whenever data, size,
// or the externally-controlled visible range changes. Gesture-driven updates
// happen outside this hook by calling handle.pan(...) directly and assigning
// the result into a SharedValue.
export function useChartCore(
  candles: Candle[],
  size: { width: number; height: number; pxRatio?: number },
  visibleRange?: VisibleRange,
): ChartCoreState {
  const handleRef = useRef<ChartHandle | null>(null);
  const [picture, setPicture] = useState<SkPicture | null>(null);

  if (!handleRef.current && size.width > 0 && size.height > 0) {
    ensureInstalled();
    handleRef.current = globalThis.VroomChartJSI!.create();
  }

  // When no visibleRange is provided, leave the range entirely to the C++
  // side (which defaults to a sensible recent window on first setCandles).
  // Only push setVisibleRange when the caller is actively controlling it,
  // so it doesn't clobber the default or fight gesture-driven pans.
  const explicit = visibleRange != null;
  const startMs = visibleRange?.startMs ?? 0;
  const endMs = visibleRange?.endMs ?? 0;

  useEffect(() => {
    const h = handleRef.current;
    if (!h) return;
    h.setSize(size.width, size.height, size.pxRatio ?? 1);
    if (candles.length > 0) {
      h.setCandles(packCandles(candles));
    }
    if (explicit) {
      h.setVisibleRange(startMs, endMs);
    }
    setPicture(h.render());
  }, [candles, size.width, size.height, size.pxRatio, explicit, startMs, endMs]);

  return { handle: handleRef.current, picture };
}
