// Ambient declaration for the JSI HostObject installed by VroomChartModule.

import type { SkPicture } from '@shopify/react-native-skia';

export interface ChartHandle {
  setCandles(buffer: ArrayBuffer): void;
  setSize(width: number, height: number, pxRatio: number): void;
  /** Pass 0, 0 to show all candles. */
  setVisibleRange(startMs: number, endMs: number): void;
  render(): SkPicture | null;
}

export interface VroomChartJSI {
  /** Creates a fresh chart instance. Destroyed when the JS reference is GC'd. */
  create(): ChartHandle;
}

declare global {
  // eslint-disable-next-line no-var
  var VroomChartJSI: VroomChartJSI | undefined;
}

// eslint-disable-next-line @typescript-eslint/no-empty-interface
export {};
