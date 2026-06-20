import type { CSSProperties } from 'react';
import type { VroomChartCoreProps } from '@vroom/types';
import type { WasmConfig } from '@vroom/core-wasm';

// The cross-platform props come from @vroom/types; `style`/`className` and the
// core-selection props (`wasm`/`forceStub`) are the web (DOM) flavor. This
// mirrors the React Native package's VroomChartProps, which adds
// StyleProp<ViewStyle> instead.
export type VroomChartProps = VroomChartCoreProps & {
  /** Class applied to the chart's root element. */
  className?: string;
  /** Inline style for the chart's root element. Defaults to filling its parent. */
  style?: CSSProperties;
  /**
   * Use the real Skia-WASM core instead of the Canvas2D stub by pointing at the
   * built module/font assets. Omit to use the stub. Read once when the chart
   * first mounts; the core is shared across all charts on the page.
   */
  wasm?: WasmConfig;
  /** Force the Canvas2D stub even when `wasm` is provided (tests / SSR). */
  forceStub?: boolean;
};
