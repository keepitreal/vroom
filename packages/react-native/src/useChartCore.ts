import { useEffect, useRef, useState } from 'react';
import type { SkPicture } from '@shopify/react-native-skia';

import NativeVroomChart from './NativeVroomChart';
import type { ChartHandle } from './jsi.d';
import { packCandles } from './packCandles';
import { applyTheme, parseColor } from './theme';
import type {
  Candle,
  MACDConfig,
  MovingAverageOverlay,
  RSIConfig,
  VisibleRange,
  VroomTheme,
} from './types';

// Mirrors vroom::ma::Source order in packages/core/src/ma.h.
const MA_SOURCES = [
  'close',
  'open',
  'high',
  'low',
  'hl2',
  'hlc3',
  'ohlc4',
] as const;

function overlayToNumeric(o: MovingAverageOverlay) {
  const srcIdx = o.source ? MA_SOURCES.indexOf(o.source) : 0;
  return {
    kind: o.kind === 'ema' ? 1 : 0,
    period: o.length,
    source: srcIdx < 0 ? 0 : srcIdx,
    color: (o.color != null ? parseColor(o.color) : null) ?? 0xff2962ff,
    width: o.width ?? 1.5,
  };
}

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
  theme?: VroomTheme,
  rsi?: RSIConfig,
  macd?: MACDConfig,
  movingAverages?: MovingAverageOverlay[],
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

  // Stable deps so inline `theme={{...}}` / `rsi={{...}}` literals don't re-run
  // the effect every render — only when the actual values change.
  const themeKey = theme ? JSON.stringify(theme) : '';
  const rsiKey = rsi ? JSON.stringify(rsi) : '';
  const macdKey = macd ? JSON.stringify(macd) : '';
  const maKey = movingAverages ? JSON.stringify(movingAverages) : '';

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
    if (theme) {
      applyTheme(h, theme);
    }
    h.setRSI(
      rsi?.enabled ?? false,
      rsi?.period ?? 14,
      rsi?.upperBand ?? 70,
      rsi?.lowerBand ?? 30,
      rsi?.maEnabled ?? true,
      rsi?.maPeriod ?? 14,
    );
    h.setMACD(
      macd?.enabled ?? false,
      macd?.fast ?? 12,
      macd?.slow ?? 26,
      macd?.signal ?? 9,
    );
    h.setOverlays((movingAverages ?? []).map(overlayToNumeric));
    setPicture(h.render());
    // theme/rsi/macd/movingAverages are represented by their *Key deps.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [candles, size.width, size.height, size.pxRatio, explicit, startMs, endMs, themeKey, rsiKey, macdKey, maKey]);

  return { handle: handleRef.current, picture };
}
