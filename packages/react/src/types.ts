import type { CSSProperties } from 'react';
import type { VroomChartCoreProps } from '@vroom/types';

// The cross-platform props come from @vroom/types; `style`/`className` are the
// web (DOM) flavor. This mirrors the React Native package's VroomChartProps,
// which adds StyleProp<ViewStyle> instead.
export type VroomChartProps = VroomChartCoreProps & {
  /** Class applied to the chart's root element. */
  className?: string;
  /** Inline style for the chart's root element. Defaults to filling its parent. */
  style?: CSSProperties;
};
