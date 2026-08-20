---
'react-native-vroom-chart': minor
---

**Interval switches now animate on React Native**, matching web. Swapping the
`candles` prop for a different interval of the same asset reshapes each candle
into its counterpart in the new data instead of jumping, and the axis labels
cross-fade with it.

The animation itself has been in the C++ core since 0.7.0, but the JSI handle
never exposed the methods that drive it, so mobile always snapped. The handle
gains the seven that were web-only: `getVisibleRange`, `resetView`,
`resetPriceScale`, `getVisiblePriceEnvelope`, `preservePriceEnvelope`,
`beginIntervalMorph` and `setIntervalMorph`.

With those in place the chart now classifies a `candles` change the way the web
component does, rather than pushing every array straight through:

- a **streaming** update (appended bar, updated last bar, rolling buffer) leaves
  the viewport exactly where it is
- a **timeframe switch** re-anchors the time window so each candle keeps its
  pixel width, scale-locks a manually zoomed price axis so the candle envelope
  keeps its pixel height, and morphs into the new shape
- a **different asset** resets to the default view

`transitionMs` and `transitionEasing` drive the morph, so `transitionMs={0}`
snaps. An OS reduced-motion preference now also snaps — previously it was
honored on web only, and it now applies to the candle↔line switch and the
volume-bar collapse on mobile too.

`seriesKey` is the explicit escape hatch for an ambiguous switch (two assets
trading at similar prices), same as on web. `classifyTransition`, `inferStepMs`
and `timeframeWindow` are exported for consumers driving their own data flow.

Apps that worked around the snap by remounting the chart on interval change
(a `key` prop) should drop the remount — a fresh handle can't morph.
