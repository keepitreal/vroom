import type { StyleProp, ViewStyle } from 'react-native';
import type { VroomChartCoreProps } from '@vroomchart/types';

// The data/config types are platform-agnostic and live in @vroomchart/types so the
// native and web components share one identical API. Re-exported here so the
// package's public surface (via index.ts) is unchanged.
export type {
  Candle,
  CrosshairEvent,
  CrosshairIndicatorKind,
  VroomColor,
  VroomTheme,
  VisibleRange,
  RSIConfig,
  MASource,
  MAKind,
  MovingAverageOverlay,
  VWAPConfig,
  BollingerBandsConfig,
  IchimokuConfig,
  FairValueGapsConfig,
  VolumeConfig,
  MACDConfig,
  ATRConfig,
  ATRSmoothing,
  ChartType,
  PriceScaleMode,
  TransitionEasing,
  IntervalTransition,
  StreamTransition,
  PriceLine,
  PriceLinesStyle,
  Footprint,
  FootprintSide,
  FootprintsStyle,
  FootprintEvent,
  CrosshairButtonConfig,
  CrosshairButtonEvent,
  CrosshairConfig,
  CrosshairLineStyle,
  CrosshairMode,
  CrosshairTimeFormat,
  PlotRect,
  DefaultDrawingStyle,
} from '@vroomchart/types';

/**
 * Props for the {@link VroomChart} component. The cross-platform props come
 * from {@link VroomChartCoreProps}; `style` is the React Native flavor.
 */
export type VroomChartProps = VroomChartCoreProps & {
  /** Style for the chart's root view. Defaults to filling the parent. */
  style?: StyleProp<ViewStyle>;
  /**
   * Declares a timeframe switch: when it changes between renders on the same
   * `seriesKey`, the data change morphs (`intervalTransition`) even when the
   * candle step is unchanged or the data heuristics would call it a stream or a
   * reset. With `visibleRange` set, this is the only way a switch morphs; the
   * range is where it lands. Omit to rely on automatic detection.
   */
  transitionKey?: string | number;
};
