import type { StyleProp, ViewStyle } from 'react-native';
import type { VroomChartCoreProps } from '@vroom/types';

// The data/config types are platform-agnostic and live in @vroom/types so the
// native and web components share one identical API. Re-exported here so the
// package's public surface (via index.ts) is unchanged.
export type {
  Candle,
  CrosshairEvent,
  VroomColor,
  VroomTheme,
  VisibleRange,
  RSIConfig,
  MASource,
  MovingAverageOverlay,
  VWAPConfig,
  MACDConfig,
} from '@vroom/types';

/**
 * Props for the {@link VroomChart} component. The cross-platform props come
 * from {@link VroomChartCoreProps}; `style` is the React Native flavor.
 */
export type VroomChartProps = VroomChartCoreProps & {
  /** Style for the chart's root view. Defaults to filling the parent. */
  style?: StyleProp<ViewStyle>;
};
