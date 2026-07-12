export { VroomChart } from './VroomChart';
export type { VroomChartProps } from './types';
export {
  classifyTransition,
  inferStepMs,
  timeframeWindow,
  type DataTransition,
} from './dataTransitions';
export type { WasmConfig } from '@vroomchart/core-wasm';

// Re-export the shared API types so web consumers can import everything from
// one package, matching react-native-vroom-chart's surface.
export type {
  Candle,
  CrosshairEvent,
  VroomTheme,
  VroomColor,
  VisibleRange,
  RSIConfig,
  MACDConfig,
  MASource,
  MovingAverageOverlay,
  VWAPConfig,
  LiquidityBand,
  LiquidityConfig,
  ChartMode,
  DrawTool,
  DrawPoint,
  Drawing,
} from '@vroomchart/types';
