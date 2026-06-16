# Gestures & viewport

All navigation is handled inside the chart — you don't wire up any gesture code.

| Gesture | Effect |
| --- | --- |
| Drag (one finger) | Scroll through time; vertical drag moves the price window |
| Pinch | Zoom around the focus point |
| Drag the price axis (right) | Rescale price only |
| Drag the time axis (bottom) | Rescale time only |
| Long-press | Show the snapping [crosshair](crosshair-and-events.md) |

## Controlling the visible window

By default the chart shows a sensible recent window on first render. To control
it explicitly, pass [`visibleRange`](../reference/index.md):

```tsx
<VroomChart
  candles={candles}
  visibleRange={{ startMs: 1700000000000, endMs: 1700090000000 }}
/>
```

Omit it (or pass both `0`) to fit all candles. When you do drive `visibleRange`,
treat [`onViewportChange`](crosshair-and-events.md) as the source of truth for the
current window after the user pans/zooms, and feed it back into state if you want
a fully controlled chart.

## Sizing

The chart fills its parent, so prefer layout-driven sizing via `style` (flex,
`aspectRatio`, or absolute fill). Pass explicit `width` / `height` only to
override measured layout. See [`VroomChartProps`](../reference/index.md).
