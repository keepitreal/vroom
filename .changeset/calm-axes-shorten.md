---
'@vroomchart/core-wasm': minor
'react-native-vroom-chart': minor
'@vroomchart/react': minor
---

Abbreviate million/billion/trillion y-axis labels as M / B / T.

A market-cap series in the tens of millions was printing `32,192,381.38` and
widening the price strip to hold it. Assets at or above a million now share a
compact format (`32.19M`, `34.54B`, `1.20T`) across ticks, the last-price pill,
the crosshair, and price-line badges. The axis strip sizes to the shorter
strings. Prices below a million, including thousands with grouping, are
unchanged.
