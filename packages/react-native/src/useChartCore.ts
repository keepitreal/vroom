import { useEffect, useRef, useState } from 'react';
import type { MutableRefObject } from 'react';
import type { SkPicture } from '@shopify/react-native-skia';

import NativeVroomChart from './NativeVroomChart';
import type { ChartHandle } from './jsi.d';
import { packCandles } from './packCandles';
import { applyTheme, parseColor } from './theme';
import type {
  BollingerBandsConfig,
  Candle,
  ChartType,
  MACDConfig,
  MovingAverageOverlay,
  PriceLine,
  PriceLinesStyle,
  RSIConfig,
  VisibleRange,
  VolumeConfig,
  VroomTheme,
  VWAPConfig,
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

// Bollinger defaults: blue bands / orange basis, matching the repo palette.
const DEFAULT_BB_BAND_COLOR = 0xff2962ff;
const DEFAULT_BB_BASIS_COLOR = 0xffff6d00;

function bollingerToSpec(cfg: BollingerBandsConfig | undefined) {
  const srcIdx = cfg?.source ? MA_SOURCES.indexOf(cfg.source) : 0;
  return {
    enabled: cfg?.enabled ?? false,
    period: cfg?.period ?? 20,
    mult: cfg?.stdDev ?? 2,
    source: srcIdx < 0 ? 0 : srcIdx,
    basisKind: cfg?.basis === 'ema' ? 1 : 0,
    upperColor:
      (cfg?.upperColor != null ? parseColor(cfg.upperColor) : null) ??
      DEFAULT_BB_BAND_COLOR,
    upperWidth: cfg?.upperWidth ?? 1,
    middleColor:
      (cfg?.middleColor != null ? parseColor(cfg.middleColor) : null) ??
      DEFAULT_BB_BASIS_COLOR,
    middleWidth: cfg?.middleWidth ?? 1,
    lowerColor:
      (cfg?.lowerColor != null ? parseColor(cfg.lowerColor) : null) ??
      DEFAULT_BB_BAND_COLOR,
    lowerWidth: cfg?.lowerWidth ?? 1,
    fillEnabled: cfg?.fill ?? true,
    fillOpacity: cfg?.fillOpacity ?? 0.1,
  };
}

// Unset style fields go down as the core's inherit sentinels (negative float,
// transparent color) rather than as literal defaults, so the theme keys stay in
// charge of anything the consumer didn't set.
function volumeToSpec(cfg: VolumeConfig | undefined) {
  return {
    enabled: cfg?.enabled ?? true,
    heightFrac: cfg?.height ?? -1,
    opacity: cfg?.opacity ?? -1,
    radiusPx: cfg?.radius ?? -1,
    upColor: (cfg?.upColor != null ? parseColor(cfg.upColor) : null) ?? 0,
    downColor: (cfg?.downColor != null ? parseColor(cfg.downColor) : null) ?? 0,
  };
}

// Price-line defaults: a soft red dotted rule with a dark translucent label,
// close in weight to the current-price indicator it sits beside.
const DEFAULT_PRICE_LINE_COLOR = 0xffef5350;
const DEFAULT_PRICE_LINE_BODY_BG = 0xd91c2128;
const DEFAULT_PRICE_LINE_HOVER_BOOST = 1.25;

const LINE_STYLES = { solid: 0, dotted: 1, dashed: 2 } as const;

// Mirrors VroomPriceLineFlags in packages/core/include/vroom/vroom_chart.h.
const PRICE_LINE_DRAGGABLE = 1 << 0;
const PRICE_LINE_CLOSABLE = 1 << 1;
const PRICE_LINE_AXIS_LABEL = 1 << 2;
const PRICE_LINE_EXTEND_LEFT = 1 << 3;

/** The price lines + their shared style, as the chart's props express them. */
export type PriceLinesProp = {
  lines: PriceLine[];
  style?: PriceLinesStyle;
  /**
   * Whether the host supplied a close handler. The close button is
   * callback-gated, so with nothing for it to do it isn't drawn at all.
   */
  hasCloseHandler: boolean;
};

function priceLinesToSpec(cfg: PriceLinesProp) {
  return {
    lines: cfg.lines.map((l) => ({
      price: l.price,
      color:
        (l.color != null ? parseColor(l.color) : null) ?? DEFAULT_PRICE_LINE_COLOR,
      width: l.width ?? 1,
      lineStyle: LINE_STYLES[l.lineStyle ?? 'dotted'],
      text: l.text ?? '',
      quantity: l.quantity ?? '',
      flags:
        (l.draggable ? PRICE_LINE_DRAGGABLE : 0) |
        (cfg.hasCloseHandler && l.closable !== false ? PRICE_LINE_CLOSABLE : 0) |
        (l.axisLabel !== false ? PRICE_LINE_AXIS_LABEL : 0) |
        (l.extendLeft !== false ? PRICE_LINE_EXTEND_LEFT : 0),
    })),
    bodyBg:
      (cfg.style?.bodyBackground != null ? parseColor(cfg.style.bodyBackground) : null) ??
      DEFAULT_PRICE_LINE_BODY_BG,
    fontSizePx: cfg.style?.fontSize ?? 0,
    lineLengthFrac: cfg.style?.inset ?? 0,
    align: cfg.style?.align === 'left' ? 0 : cfg.style?.align === 'center' ? 1 : 2,
    hoverBoost: cfg.style?.hoverBoost ?? DEFAULT_PRICE_LINE_HOVER_BOOST,
  };
}

// Cleared overlay: no lines (the style values are irrelevant, but the spec shape
// requires them).
const EMPTY_PRICE_LINES = priceLinesToSpec({ lines: [], hasCloseHandler: false });

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

/** Progress + curve of the staggered volume-bar collapse. See setVolumeCollapse. */
export type VolumeCollapse = { t: number; easing: number };

export type ChartCoreState = {
  handle: ChartHandle | null;
  /** Picture freshly rendered after the latest data/size/range push. */
  picture: SkPicture | null;
  /**
   * The last volume collapse handed to the core, or null before the first push.
   * VroomChart's animation loop owns this — it lives here only so the data effect
   * can restore it, since setVolume snaps the scalar (see below).
   */
  volumeCollapseRef: MutableRefObject<VolumeCollapse | null>;
};

// Owns a ChartHandle and produces an "initial" picture whenever data, size,
// or the externally-controlled visible range changes. Gesture-driven updates
// happen outside this hook by calling handle.pan(...) directly and assigning
// the result into a SharedValue.
export function useChartCore(
  candles: Candle[],
  size: { width: number; height: number; pxRatio?: number },
  visibleRange?: VisibleRange,
  defaultCandleWidth?: number,
  chartType?: ChartType,
  theme?: VroomTheme,
  rsi?: RSIConfig,
  macd?: MACDConfig,
  movingAverages?: MovingAverageOverlay[],
  vwap?: VWAPConfig,
  bollingerBands?: BollingerBandsConfig,
  volume?: VolumeConfig,
  priceLines?: PriceLinesProp,
): ChartCoreState {
  const handleRef = useRef<ChartHandle | null>(null);
  // Push setDefaultCandleWidth only once (first load): setCandles re-runs on
  // every data change, and the core setter re-frames when candles are present,
  // so re-pushing would snap the view away from the user's pan/zoom.
  const defaultWidthAppliedRef = useRef(false);
  const volumeCollapseRef = useRef<VolumeCollapse | null>(null);
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
  const vwapKey = vwap ? JSON.stringify(vwap) : '';
  const bollingerKey = bollingerBands ? JSON.stringify(bollingerBands) : '';
  const volumeKey = volume ? JSON.stringify(volume) : '';
  const priceLinesKey = priceLines ? JSON.stringify(priceLines) : '';

  useEffect(() => {
    const h = handleRef.current;
    if (!h) return;
    h.setSize(size.width, size.height, size.pxRatio ?? 1);
    // Drive the initial zoom from a target candle width. Pushed once, before the
    // first setCandles (while the core window is still 0/0), and only when the
    // caller isn't explicitly controlling the range.
    if (
      !defaultWidthAppliedRef.current &&
      !explicit &&
      defaultCandleWidth != null &&
      defaultCandleWidth > 0
    ) {
      h.setDefaultCandleWidth(defaultCandleWidth);
      defaultWidthAppliedRef.current = true;
    }
    if (candles.length > 0) {
      h.setCandles(packCandles(candles));
    }
    if (explicit) {
      h.setVisibleRange(startMs, endMs);
    }
    // chartType / the candle↔line morph is driven separately (VroomChart owns the
    // per-frame animation loop so it can update the picture SharedValue directly).
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
    h.setVWAP(
      vwap?.enabled ?? false,
      vwap?.resetMinutes ?? 0,
      (vwap?.color != null ? parseColor(vwap.color) : null) ?? 0xff00bcd4,
      vwap?.width ?? 1.5,
    );
    h.setBollinger(bollingerToSpec(bollingerBands));
    h.setVolume(volumeToSpec(volume));
    // setVolume snaps the collapse scalar to its `enabled`, which would cut a
    // toggle animation short whenever this effect re-runs (a streaming candle, a
    // resize). Hand the in-flight value back; VroomChart's loop drives it from
    // there.
    const collapse = volumeCollapseRef.current;
    if (collapse) h.setVolumeCollapse(collapse.t, collapse.easing);
    h.setPriceLines(
      priceLines?.lines.length ? priceLinesToSpec(priceLines) : EMPTY_PRICE_LINES,
    );
    // TODO(rn-parity): mirror the web `liquidity` overlay here (setLiquidity +
    // the VroomBand structs in the JSI handle) — web-only for now.
    setPicture(h.render());
    // theme/rsi/macd/movingAverages/vwap/bollingerBands/volume/priceLines are
    // represented by their *Key deps.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [candles, size.width, size.height, size.pxRatio, explicit, startMs, endMs, defaultCandleWidth, themeKey, rsiKey, macdKey, maKey, vwapKey, bollingerKey, volumeKey, priceLinesKey]);

  return { handle: handleRef.current, picture, volumeCollapseRef };
}
