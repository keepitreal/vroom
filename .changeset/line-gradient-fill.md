---
"@vroomchart/core-wasm": minor
"react-native-vroom-chart": minor
"@vroomchart/react": minor
---

Line-chart mode now defaults to a violet stroke and fills a gradient beneath the
line.

The default `lineColor` moves from the neutral foreground to `#8957e5`, matching
the RSI line. A gradient using that same color is now filled under the polyline,
strongest at the line's peak and ramping to fully transparent at the bottom of
the price pane. It fades in with the candle-to-line morph and reshapes with the
line through a timeframe switch, since it's built from the same vertices.

New `theme.lineGradientOpacity` sets the fill's strength at its strongest point
(default 0.28); set it to 0 to render the bare line as before.

The fill draws behind the volume bars rather than with the line, so bars stay
legible on top of it. Skia's linear-gradient API differs between the Skia the
WASM build uses and the one react-native-skia bundles; the selection shim that
already existed for liquidity bands moved into a shared `gradient.h` so both
layers build the ramp the same way.
