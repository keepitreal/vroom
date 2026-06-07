import type { StyleProp, ViewStyle } from 'react-native';

export type Candle = {
  timeMs: number;
  open: number;
  high: number;
  low: number;
  close: number;
  volume: number;
};

export type CrosshairEvent = {
  active: boolean;
  timeMs: number;
  price: number;
};

export type VroomTheme = {
  background?: string;
  bull?: string;
  bear?: string;
  wick?: string;
  grid?: string;
  axisText?: string;
  crosshair?: string;
  tooltipBg?: string;
  tooltipText?: string;
};

export type VisibleRange = {
  startMs: number;
  endMs: number;
};

export type VroomChartProps = {
  candles: Candle[];
  /**
   * Explicit size overrides in logical px. When omitted, the chart fills its
   * parent (measured via onLayout). Use `style` (flex / aspectRatio / absolute
   * fill) for layout-driven sizing instead of hard-coding these.
   */
  width?: number;
  height?: number;
  /** Style for the chart's root view. Defaults to filling the parent. */
  style?: StyleProp<ViewStyle>;
  /** Time window to render. Omit (or both 0) to show every candle. */
  visibleRange?: VisibleRange;
  theme?: VroomTheme;
  /**
   * Pixels the crosshair dot / horizontal line sit *above* the touch point so
   * they aren't hidden under the thumb. The vertical line stays centered on the
   * touch x. Default 40.
   */
  crosshairOffset?: number;
  onCrosshair?: (e: CrosshairEvent) => void;
  onViewportChange?: (startMs: number, endMs: number) => void;
};
