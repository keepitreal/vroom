import { useEffect, useRef, useState } from 'react';
import type { SkPicture } from '@shopify/react-native-skia';

import NativeVroomChart from './NativeVroomChart';
import type { ChartHandle } from './jsi.d';
import { packCandles } from './packCandles';
import type { Candle, VisibleRange } from './types';

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
