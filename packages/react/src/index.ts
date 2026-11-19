export { VroomChart } from './VroomChart';
export type { VroomChartProps } from './types';
export {
  classifyTransition,
  inferStepMs,
  timeframeWindow,
  type DataTransition,
} from './dataTransitions';
export type { WasmConfig } from '@vroomchart/core-wasm';
// Opening Range Breakout: the stats table <VroomChart> shows for `orb`, and the
// pure engine for running the same strategy off-chart (e.g. a backtest).
export { OrbStatsPanel, type OrbStatsPanelProps } from './OrbStatsPanel';
export { runOrb, type RunOrbOptions } from '@vroomchart/strategies';
// Envelope (de)serializers for the managed `drawingStore` — exposed for
// consumers who pre-seed a store, import/export drawings, or migrate payloads.
export { serializeDrawings, deserializeDrawings, DRAWINGS_VERSION } from './drawingStorage';
// Drawing undo/redo. Managed mode (`drawingStore`) has history built in — these
// are for controlled mode: `useDrawingHistory` owns the array + stack for you,
// `DrawingHistory` is the bare stack for custom state management.
export { useDrawingHistory, type DrawingHistoryApi } from './useDrawingHistory';
export {
  DrawingHistory,
  DEFAULT_HISTORY_LIMIT,
  type DrawingHistoryEntry,
} from './drawingHistory';

// Re-export the shared API types so web consumers can import everything from
// one package, matching react-native-vroom-chart's surface.
export type {
  Candle,
  CrosshairEvent,
  CrosshairIndicatorKind,
  VroomTheme,
  VroomColor,
  VisibleRange,
  RSIConfig,
  MACDConfig,
  ATRConfig,
  ATRSmoothing,
  MASource,
  MAKind,
  MovingAverageOverlay,
  VWAPConfig,
  BollingerBandsConfig,
  IchimokuConfig,
  FairValueGapsConfig,
  OrbConfig,
  OrbBreakUnit,
  OrbDirection,
  OrbDirectionMode,
  OrbEvent,
  OrbEventType,
  OrbRangePreset,
  OrbResult,
  OrbSession,
  OrbSignal,
  OrbSignalStatus,
  OrbSkipReason,
  OrbStats,
  OrbStopMode,
  OrbTargetMode,
  OrbTimeOfDay,
  OrbTrade,
  OrbWarning,
  OrbWeekday,
  VolumeConfig,
  LiquidityBand,
  LiquidityConfig,
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
  ChartMode,
  ChartType,
  PriceScaleMode,
  TransitionEasing,
  IntervalTransition,
  StreamTransition,
  DrawTool,
  DrawPoint,
  Drawing,
  LineDrawing,
  BoxDrawing,
  PencilDrawing,
  PathDrawing,
  DrawingStore,
  DrawingRect,
  DrawingSelection,
  DrawingStyle,
  DefaultDrawingStyle,
  DrawingControls,
  UndoRedoState,
  UndoRedoControls,
} from '@vroomchart/types';
