---
'@vroomchart/core-wasm': patch
'react-native-vroom-chart': patch
'@vroomchart/react': patch
---

Animate the current-price indicator with the candles. The dashed line and its
price badge mark the latest close, so every tick moved them — but they were read
straight off the final close while the candle they belong to was still easing
into place. For the length of the transition the badge sat at a price the chart
was not yet showing, which read as the indicator coming loose from the series.

The indicator now rides the same capture and the same clock the candles reshape
through, so it tracks the newest bar's close edge on every frame instead of only
at the two ends. Its y is the expression the line chart's tip already uses, which
glues the badge to the tip by construction rather than by giving the two a
matching duration. The badge text interpolates alongside the movement, so the
number always agrees with where the box is sitting, and the accent cross-fades
when a tick flips the candle's direction instead of snapping from one to the
other.

This covers live ticks under `streamTransition: 'transform'` and timeframe
switches under `intervalTransition: 'transform'`, since both drive the same
capture. A fade swap is unchanged — there is no geometry to interpolate through
a cross-dissolve, so the indicator still lands directly. Panned back into
history the newest candle is off-screen and the capture does not pair with it,
so the indicator places directly there too, matching the tip marker.

No API change: the transition and its duration are the existing
`streamTransition` / `streamTransitionMs` and `intervalTransition` props.
