---
"@vroomchart/core-wasm": minor
---

Animate the line chart across a timeframe switch: each vertex slides to its new
close instead of the whole polyline jumping.

Line mode reuses the geometry the candle interval morph already captures — a
snapshot's `x` and `close` fractions are exactly what a polyline vertex needs —
so `transitionMs` and `transitionEasing` drive it with no extra API. Vertices
pair by slot, the same position-from-the-right-edge rule the candle morph uses,
so only y moves.

The close line now has its own entry point rather than borrowing the MA-overlay
routine, which also drops a per-frame heap allocation on the line-mode draw path.
Indicator overlays (SMA/EMA, VWAP, Bollinger) and volume still snap; their series
aren't candle closes, so the capture can't stand in for them.
