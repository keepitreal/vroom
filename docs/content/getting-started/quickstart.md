# Quickstart

Render a chart by passing an array of [`Candle`](../reference/index.md) objects.
The chart fills its parent by default, so size it with `style`.

```tsx
import { View } from 'react-native';
import { GestureHandlerRootView } from 'react-native-gesture-handler';
import { VroomChart, type Candle } from 'react-native-vroom-chart';

const candles: Candle[] = [
  { timeMs: 1700000000000, open: 100, high: 105, low: 98, close: 103, volume: 1200 },
  { timeMs: 1700003600000, open: 103, high: 108, low: 102, close: 107, volume: 1500 },
  // …more bars
];

export function Chart() {
  return (
    <GestureHandlerRootView style={{ flex: 1 }}>
      <View style={{ flex: 1 }}>
        <VroomChart candles={candles} style={{ flex: 1 }} />
      </View>
    </GestureHandlerRootView>
  );
}
```

That's the whole required surface: **`candles`**. Everything else is optional.

## Interacting

- **Pan** — drag horizontally to scroll through time (and vertically to move the
  price window).
- **Pinch** — zoom in/out around your fingers.
- **Axis drag** — drag the price (right) or time (bottom) axis to rescale just
  that axis.
- **Long-press** — show a crosshair that snaps to the nearest candle.

## What's next

- Control the initial window with [`visibleRange`](../reference/index.md), or omit
  it to show a sensible recent window.
- [Theme](../guides/theming.md) the colors.
- Add [indicators](../guides/indicators.md) — RSI, MACD, moving averages, VWAP.
- React to [crosshair and viewport events](../guides/crosshair-and-events.md).

See the [API reference](../reference/index.md) for every prop and type.
