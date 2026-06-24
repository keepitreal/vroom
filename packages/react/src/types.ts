import type { CSSProperties } from 'react';
import type { VroomChartCoreProps } from '@vroomchart/types';
import type { WasmConfig } from '@vroomchart/core-wasm';

// The cross-platform props come from @vroomchart/types; `style`/`className` and the
// core-selection prop (`wasm`) are the web (DOM) flavor. This mirrors the React
// Native package's VroomChartProps, which adds StyleProp<ViewStyle> instead.
export type VroomChartProps = VroomChartCoreProps & {
  /** Class applied to the chart's root element. */
  className?: string;
  /** Inline style for the chart's root element. Defaults to filling its parent. */
  style?: CSSProperties;
  /**
   * Load the Skia-WASM core from your own module/font URLs instead of the build
   * bundled in `@vroomchart/core-wasm`. Read once when the chart first mounts;
   * the core is shared across all charts on the page.
   */
  wasm?: WasmConfig;
};
