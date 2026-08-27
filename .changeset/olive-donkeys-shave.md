---
'@vroomchart/core-wasm': minor
'@vroomchart/react': minor
'react-native-vroom-chart': minor
---

**Price labels now follow the asset's own scale instead of the tick interval.**
Deriving decimals from the interval got both ends of the range wrong: a
five-figure asset printed `84000` where it should read `84,000.00`, and a
sub-cent one printed `0.000060` where the digits that distinguish it start at
the seventh decimal.

Precision is now a fixed five significant digits with a two-decimal floor, keyed
to the latest close, and prices carry thousands separators:

| Price | Renders as |
|---|---|
| 0.000044094 | `0.000044094` |
| 0.023397 | `0.023397` |
| 2513.92 | `2,513.92` |
| 80285.20 | `80,285.20` |

Because it keys to the asset rather than the visible range, the precision holds
steady while you pan and zoom rather than shifting under your finger. The y-axis
strip measures itself against the resulting labels, so it widens to fit nine
decimals and stays narrow for two. The tick interval keeps one job: a zoom deep
enough to make adjacent labels collide still adds decimals.

This applies everywhere a price is drawn — axis labels, the current-price
indicator, the crosshair badge, and price-line labels.
