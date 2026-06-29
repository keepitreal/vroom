# Theming

Override any chart color through the `theme` prop. Every field is optional —
omitted colors keep the library default.

```tsx
<VroomChart
  candles={candles}
  theme={{
    background: '#0d1117',
    bull: '#26a69a',
    bear: '#ef5350',
    accentBull: '#26a69a',
    accentBear: '#ef5350',
    borderBull: '#1b7a70',
    borderBear: '#c0392b',
    wickBull: '#26a69a',
    wickBear: '#ef5350',
    grid: '#1f2630',
    axisText: '#8b949e',
    crosshair: '#c9d1d9',
    crosshairTarget: '#ffffff',
  }}
/>
```

## Color formats

A color ([`VroomColor`](../reference/index.md)) is either:

- a **hex string** — `'#0d1117'` (6-digit RGB) or `'#ff0d1117'` (8-digit
  `#aarrggbb` with alpha), or
- a **packed ARGB number** — e.g. `0xff0d1117`.

## What each field paints

| Field | Affects |
| --- | --- |
| `background` | Chart + axis-strip background |
| `bull` | Up candle body fill |
| `bear` | Down candle body fill |
| `borderBull` | Up candle body 1px border (defaults to `bull`) |
| `borderBear` | Down candle body 1px border (defaults to `bear`) |
| `wickBull` | Up candle wick (defaults to `bull`) |
| `wickBear` | Down candle wick (defaults to `bear`) |
| `accentBull` | Generic up color: rising price indicator, up volume bars, MACD histogram |
| `accentBear` | Generic down color: falling price indicator, down volume bars, MACD histogram |
| `grid` | Gridlines |
| `axisText` | Price + time axis label text |
| `crosshair` | Crosshair dashed lines |
| `crosshairTarget` | The hollow ring/dot at the crosshair intersection |

See [`VroomTheme`](../reference/index.md) for the canonical field list.
