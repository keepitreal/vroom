export { VroomChart } from './VroomChart';
export type { VroomChartProps } from './types';
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
} from '@vroomchart/types';
