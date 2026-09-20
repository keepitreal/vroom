---
'@vroomchart/core-wasm': patch
'react-native-vroom-chart': patch
'@vroomchart/react': patch
---

Fix the line chart's tip dot being sliced in half by the price axis when the
view sits on the newest bar. The dot marks the newest close, which a view pinned
to the latest bar parks within a few pixels of the plot's right edge — closer
than the dot's own radius — so it was clipped against that edge and read as a
rendering fault next to the price badge.

The marker now paints after the axis backgrounds instead of before them, so it
can use the gutter between the plot and the y-axis strip, and in line mode that
gutter widens to the dot's radius plus a few pixels of clearance. The dot is
therefore drawn in full, clear of the axis, in any framing — including a host
supplied `visibleRange` that ends flush at the newest candle's slot, which
bypasses the chart's own default framing. The extra gutter eases in with the
candles-to-line crossfade rather than stepping when the mode changes.

The pulse ring is deliberately not covered by this. At its widest it is several
times the dot's radius, and reserving that much gutter would cost the plot real
estate, so the ring still clips against the axis in tight framings.
