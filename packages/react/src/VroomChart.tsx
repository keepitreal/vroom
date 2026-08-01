// VroomChart (web) — DOM React component that renders the candlestick chart to
// a <canvas> via @vroomchart/core-wasm and wires pointer/wheel gestures. API-matched
// to the React Native component (same props from @vroomchart/types).

import React, { useEffect } from 'react';
import type { CSSProperties } from 'react';

import { useChartCore } from './useChartCore';
import { useGestures } from './useGestures';
import { useManagedDrawings } from './useManagedDrawings';
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
  const {
    className,
    style,
    wasm,
    crosshairOffset = 40,
    crosshairOverride,
    mode,
    tool,
    drawings,
    onCrosshair,
    onViewportChange,
    onDrawingComplete,
    onDrawingChange,
    onDrawingDelete,
    onModeChange,
    drawingStore,
    seriesKey,
    historyLimit,
    onHistoryChange,
    historyRef,
    onUndo,
    onRedo,
  } = props;

  // Managed persistence: when a `drawingStore` is provided the chart owns the
  // drawings array itself (keyed by seriesKey) instead of the controlled props.
  // The hook is always called (rules of hooks) but no-ops without a store.
  const managed = useManagedDrawings(seriesKey, drawingStore, historyLimit);
  const stored = drawingStore != null;
  const effectiveDrawings = stored ? managed.drawings : drawings;

  // Managed-mode history surface: availability changes out via onHistoryChange,
  // programmatic controls in via historyRef. Controlled mode owns its own
  // history (see useDrawingHistory), so both are managed-only.
  const { historyState, undo, redo, clearHistory } = managed;
  useEffect(() => {
    if (stored) onHistoryChange?.(historyState);
  }, [stored, historyState, onHistoryChange]);
  useEffect(() => {
    if (!historyRef || !stored) return;
    historyRef.current = { undo, redo, clearHistory };
    return () => {
      historyRef.current = null;
    };
  }, [historyRef, stored, undo, redo, clearHistory]);

  const { containerRef, canvasRef, handleRef, scheduleRender } = useChartCore(
    stored ? { ...props, drawings: effectiveDrawings } : props,
    wasm ? { wasm } : undefined,
  );

  useGestures(containerRef, handleRef, scheduleRender, {
    crosshairOffset,
    crosshairOverride,
    mode,
    tool,
    drawings: effectiveDrawings,
    onCrosshair,
    onViewportChange,
    onDrawingComplete: stored ? managed.onDrawingComplete : onDrawingComplete,
    onDrawingChange: stored ? managed.onDrawingChange : onDrawingChange,
    onDrawingDelete: stored ? managed.onDrawingDelete : onDrawingDelete,
    onUndo: stored ? managed.undo : onUndo,
    onRedo: stored ? managed.redo : onRedo,
    onRequestMode: onModeChange,
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
