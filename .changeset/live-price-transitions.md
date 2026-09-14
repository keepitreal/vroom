---
'@vroomchart/core-wasm': minor
'react-native-vroom-chart': minor
'@vroomchart/react': minor
---

Animate live price updates with the new `streamTransition` prop. A chart fed a
growing `candles` array used to apply every update on the next frame with no
motion at all: the in-progress bar jumped between shapes, and a newly closed bar
appeared without the series moving, so at any real tick rate the chart flickered
rather than ran.

Set `streamTransition: 'transform'` and the two kinds of update animate
separately, because they are different motions:

- A **tick** — the last bar's values change — eases that bar from its old shape
  into its new one, carrying every indicator reading from it along with the
  close line. Ticks that arrive faster than the animation lands resume from the
  shape on screen rather than snapping back, so a fast feed stays smooth.
- An **append** — a new bar arrives — slides the window one slot left, so the
  series translates and the new bar enters at the right edge. Only when the view
  is still following the newest bar: panned back into history, it is left alone,
  as is a chart driving its own `visibleRange`. A pan or pinch mid-slide takes
  over immediately.

`streamTransitionMs` sets the duration, defaulting to 150 — shorter than
`transitionMs`, since ticks can land faster than a 300ms curve. Easing follows
`transitionEasing`, and a reduced-motion preference snaps.

Unlike a timeframe switch, the price axis holds still through a tick: the
interval hasn't changed, so its ticks are still the right ones.

`streamTransition` defaults to `'none'`, which behaves exactly as before.
