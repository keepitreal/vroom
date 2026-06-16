# Crosshair & events

vroom surfaces what the user is looking at back to your app so you can render
your own overlays, tooltips, or haptics. The library never plays haptics itself —
it just tells you what happened.

## Crosshair

A long-press shows a crosshair that snaps to the nearest candle. Subscribe with
`onCrosshair`; the callback receives a [`CrosshairEvent`](../reference/index.md):

```tsx
import * as Haptics from 'expo-haptics';

<VroomChart
  candles={candles}
  onCrosshair={(e) => {
    if (e.reason === 'show' || e.reason === 'move') {
      Haptics.selectionAsync();
    }
    setSelected(e.active ? e.candle : null); // full OHLCV under the crosshair
  }}
/>
```

- `active` — whether the crosshair is showing.
- `candle` — the [`Candle`](../reference/index.md) under the crosshair (OHLCV), or
  `null` when inactive.
- `reason` — `'show'` (long-press activated), `'move'` (snapped to a different
  candle — one event per candle crossed, not per frame), or `'hide'` (dismissed).

Use `crosshairOffset` to lift the dot/horizontal line above the touch point so
they aren't hidden under the thumb (default 40px).

## Viewport changes

`onViewportChange` fires when the visible time range changes via gesture, giving
you the new window bounds:

```tsx
<VroomChart
  candles={candles}
  onViewportChange={(startMs, endMs) => {
    // e.g. fetch more candles when the user scrolls into the past
  }}
/>
```

This is the read-side counterpart to the [`visibleRange`](../reference/index.md)
prop — see [Gestures & viewport](gestures-and-viewport.md).
