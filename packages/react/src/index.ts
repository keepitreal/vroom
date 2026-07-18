export { VroomChart } from './VroomChart';
export type { VroomChartProps } from './types';
export {
  classifyTransition,
  inferStepMs,
  timeframeWindow,
  type DataTransition,
} from './dataTransitions';
export type { WasmConfig } from '@vroomchart/core-wasm';
// Envelope (de)serializers for the managed `drawingStore` — exposed for
// consumers who pre-seed a store, import/export drawings, or migrate payloads.
export { serializeDrawings, deserializeDrawings, DRAWINGS_VERSION } from './drawingStorage';

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
  ChartType,
  DrawTool,
  DrawPoint,
  Drawing,
  DrawingStore,
} from '@vroomchart/types';
