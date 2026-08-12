---
"@vroomchart/core-wasm": minor
"@vroomchart/react": minor
"react-native-vroom-chart": minor
---

Give every indicator config one set of naming conventions, and bring RSI and
VWAP up to the styling other indicators already had.

RSI now takes colors, widths, and visibility for the index line, the trendline,
and the pair of dashed band rules, plus an SMA/EMA choice for the trendline
(`maType`). VWAP keeps its `color` and `width` but is now marshalled as a struct
like the rest. Both are unchanged visually until you set something: unset colors
and widths still resolve to the stock look in the renderer.

`'sma' | 'ema'` is now the shared `MAKind` union wherever it appears, matching
how `MASource` is already shared.

Renamed fields:

| Before | After |
| --- | --- |
| `MovingAverageOverlay.kind` | `maType` |
| `MovingAverageOverlay.length` | `period` |
| `BollingerBandsConfig.basis` | `maType` |
| `BollingerBandsConfig.fill` | `fillVisible` |
| `RSIConfig.maEnabled` | `maVisible` |
| `MACDConfig.macdColor` / `macdWidth` / `macdVisible` | `lineColor` / `lineWidth` / `lineVisible` |

The low-level `VroomChartHandle.setRSI` and `setVWAP` now take a single spec
object instead of positional arguments, matching `setMACD` and `setBollinger`.
