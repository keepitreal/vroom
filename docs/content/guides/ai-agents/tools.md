# Agent tools

These are the tools `@vroomchart/agent` gives a model. The model sees each
tool's description and full JSON Schema, so this page is for you: what each tool
does, and the rules it enforces.

Every tool returns a short result string meant to go straight back to the model.
Tools that create something put its id in the result, so the model can refer to
it later.

## Indicators

### `set_indicators`

Shows, hides, or configures indicators. Pass only the ones to change. Settings
merge over what's already set, and passing an indicator turns it on unless it
says `enabled: false`.

```json
{ "rsi": { "period": 14, "lowerBand": 30, "upperBand": 70 } }
```

```json
{ "macd": { "fast": 12, "slow": 26, "signal": 9 }, "volume": { "enabled": false } }
```

It covers `rsi`, `macd`, `atr`, `vwap`, `bollingerBands`, `ichimoku`,
`fairValueGaps`, and `volume`, and accepts the settings an agent is likely to
need for each: periods, levels, price sources, averaging types, visibility, and
the main colors. The full styling options are in the
[indicators guide](../indicators.md).

Values out of range are rejected with an explanation rather than clamped: an
RSI band outside 0 to 100, a lower band at or above the upper band, or a MACD
fast period that isn't shorter than the slow one. Unknown indicator names and
settings are rejected too.

### `add_moving_average` and `remove_moving_average`

Adds an SMA or EMA line to the price pane:

```json
{ "maType": "ema", "period": 200, "color": "#f5a623" }
```

The id defaults to `<maType>-<period>`, such as `ema-200`. Adding with an id that
already exists replaces that line. Remove a line with `{ "id": "ema-200" }`.

## Drawings

### Points

Drawing tools take points in data space: a `price`, plus exactly one of:

- `timeMs`: a Unix epoch time in milliseconds.
- `barsAgo`: the candle this many bars before the latest. `0` is the latest
  candle. A negative value reaches into the future, one candle interval per
  step, which is how a zone extends to the right of the last bar.

`barsAgo` is usually easier for a model, because it doesn't have to work out
timestamps.

### `add_drawing`

Draws a `line` (two-point trendline), a `box` (a rectangle between two opposite
corners), or a `path` (a polyline ending in an arrowhead). Lines and boxes take
exactly two points; paths take 2 to 64.

A support zone from 120 bars ago to 20 bars past the latest candle:

```json
{
  "type": "box",
  "points": [
    { "barsAgo": 120, "price": 64100 },
    { "barsAgo": -20, "price": 63800 }
  ],
  "label": "Support",
  "fill": "#332962ff"
}
```

Optional `color`, `width`, and `label` apply to every type, and `fill` to boxes.
The `label` isn't drawn on the chart; it names the drawing in results and in
`get_market_context`. The new drawing is tagged `meta: { source: "agent" }`.

### `update_drawing`

Moves or restyles a drawing by `id`. Fields left out keep their current values.
Drawings the user locked are refused.

### `remove_drawings`

Removes drawings by id (`{ "ids": ["box-1a2b3c4d"] }`), or every drawing the
agent created (`{ "allAgentDrawings": true }`). Locked drawings are kept, and
the result says how many.

## Price levels

### `add_price_level` and `remove_price_levels`

Draws a full-width horizontal line at a price, with an optional `text` label,
`color`, and `lineStyle` (`solid`, `dotted`, or `dashed`). Good for key levels,
targets, and stops:

```json
{ "price": 64250, "text": "Resistance", "lineStyle": "dashed" }
```

Adding with an existing `id` moves that level. Remove levels by id, or all of the
agent's levels with `{ "all": true }`. Price lines your app passes to the hook
aren't visible to these tools.

## View

### `set_viewport`

Frames the most recent candles with `{ "lastBars": 200 }`, or an explicit window
with `startMs` and `endMs`. With `useAgentChart`, the window then follows the
user's own pans and zooms, so the chart doesn't snap back on the next render.

### `set_chart_display`

Switches `chartType` between `candles` and `line`, and `priceScaleMode` between
`linear` and `log`.

## Reading the chart

### `get_market_context`

Describes what the chart shows and lists support and resistance levels:

```text
Interval: 15m. 500 candles loaded, 2026-10-25T07:00:00Z to 2026-10-30T11:45:00Z.
In view: 100 bars, 2026-10-29T10:45:00Z to 2026-10-30T11:45:00Z. Open 63950, close 64480 (+0.83%). ...
Indicators: RSI(14) 61.2, bands 30/70; EMA(200) 63720.5 [id ema-200].

Swing levels:
resistance 64610.25 (4 touches, last 2026-10-30T09:15:00.000Z)
support 63805.5 (3 touches, last 2026-10-29T22:30:00.000Z)
```

The levels come from `findSwingLevels`. It finds swing highs and lows that stand
above or below `strength` bars on each side (default 3), clusters ones within
half an ATR of each other, and ranks the clusters by how many swings landed
there. A higher `strength` finds fewer, more major levels. Levels above the last
close are resistance; the rest are support.

"In view" is the user's actual viewport when `useAgentChart` has seen one,
then the window the agent set, then the last 100 candles.

### `get_chart_state`

Returns the current configuration as JSON: indicators, moving averages, the
agent's price levels, drawings, and display settings.

## Helpers

The functions behind the read tools are exported for your own prompts and UI:

| Function | Returns |
| --- | --- |
| `describeChart(candles, state, viewport?)` | The text summary `get_market_context` starts with |
| `findSwingLevels(candles, options?)` | Support and resistance levels with touch counts |
| `latestRSI`, `latestMACD`, `latestATR`, `latestMA` | The newest indicator value |
| `inferStepMs(candles)`, `formatInterval(ms)` | The candle interval, and a label like `15m` |
