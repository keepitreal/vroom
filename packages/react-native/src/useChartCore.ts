import { useEffect, useRef, useState } from 'react';
import type { SkPicture } from '@shopify/react-native-skia';

import NativeVroomChart from './NativeVroomChart';
import type { ChartHandle } from './jsi.d';
import type { Candle } from './types';

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

// Owns a ChartHandle for the lifetime of the consuming component. Pushes the
// latest candles + size into the core whenever they change, then renders and
// returns the resulting SkPicture. The picture is recomputed inside an effect
// (not during render) so it always reflects the most recent data load.
export function useChartCore(
  candles: Candle[],
  size: { width: number; height: number; pxRatio?: number },
): SkPicture | null {
  const handleRef = useRef<ChartHandle | null>(null);
  const [picture, setPicture] = useState<SkPicture | null>(null);

  if (!handleRef.current && size.width > 0 && size.height > 0) {
    ensureInstalled();
    handleRef.current = globalThis.VroomChartJSI!.create();
  }

  useEffect(() => {
    const h = handleRef.current;
    if (!h) return;
    h.setSize(size.width, size.height, size.pxRatio ?? 1);
    if (candles.length > 0) {
      h.setCandles(packCandles(candles));
    }
    setPicture(h.render());
  }, [candles, size.width, size.height, size.pxRatio]);

  return picture;
}
