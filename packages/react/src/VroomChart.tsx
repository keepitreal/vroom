// VroomChart (web) — DOM React component that renders the candlestick chart to
// a <canvas> via @vroom/core-wasm and wires pointer/wheel gestures. API-matched
// to the React Native component (same props from @vroom/types).

import React from 'react';
import type { CSSProperties } from 'react';

import { useChartCore } from './useChartCore';
import { useGestures } from './useGestures';
import type { VroomChartProps } from './types';

const FILL: CSSProperties = { position: 'relative', width: '100%', height: '100%' };
const CANVAS_STYLE: CSSProperties = {
  display: 'block',
  width: '100%',
  height: '100%',
  touchAction: 'none', // we own pan/zoom; don't let the browser scroll/zoom
};

/**
 * Skia-quality candlestick chart for the web. Pass OHLCV `candles` and size it
 * via `style`/`width`/`height` (it fills its parent by default). Drag to pan,
 * pinch or wheel to zoom, drag the price/time axes to rescale, hover (mouse) or
 * long-press (touch) for the crosshair. Optional indicators (`rsi`, `macd`,
 * `movingAverages`, `vwap`), colors (`theme`), and events (`onCrosshair`,
 * `onViewportChange`) are configured through props.
 *
 * @see {@link VroomChartProps} for the full prop reference.
 */
export function VroomChart(props: VroomChartProps) {
  const { className, style, wasm, forceStub, crosshairOffset = 40, onCrosshair, onViewportChange } =
    props;
  const { containerRef, canvasRef, handleRef, scheduleRender } = useChartCore(
    props,
    wasm || forceStub ? { wasm, forceStub } : undefined,
  );

  useGestures(containerRef, handleRef, scheduleRender, {
    crosshairOffset,
    onCrosshair,
    onViewportChange,
  });

  const rootStyle: CSSProperties = {
    ...FILL,
    ...(props.width != null ? { width: props.width } : null),
    ...(props.height != null ? { height: props.height } : null),
    ...style,
  };

  return (
    <div ref={containerRef} className={className} style={rootStyle}>
      <canvas ref={canvasRef} style={CANVAS_STYLE} />
    </div>
  );
}
