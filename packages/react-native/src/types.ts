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

export type VroomChartProps = {
  candles: Candle[];
  width?: number;
  height?: number;
  theme?: VroomTheme;
  onCrosshair?: (e: CrosshairEvent) => void;
  onViewportChange?: (startMs: number, endMs: number) => void;
};
