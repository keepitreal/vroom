---
"@vroomchart/core-wasm": minor
"@vroomchart/react": minor
---

Scale-lock the price axis across a timeframe switch, so candles keep their
vertical size when the interval changes.

Switching intervals re-buckets the same price action into a smaller or larger
high-low span. The time axis already preserved each candle's pixel width; the
price axis dropped back to auto-fit, which threw away any zoom the user had
applied to the y-axis and made the candles abruptly shrink or grow. The switch
now rescales that zoom instead: the visible high-low envelope keeps the exact
pixel height and position it had a frame earlier, and the price labels crossfade
to the new values.

Auto-y is unchanged — auto-fit widens the envelope by a fixed factor, so its
pixel height was already interval-invariant.

Two new handle methods back this, both web-only for now: `getVisiblePriceEnvelope`
reads the visible min-low/max-high, and `preservePriceEnvelope` applies the
rescale (a no-op in auto-y mode).
