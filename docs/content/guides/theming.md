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
| `bull` | Up candles, bull wicks, bull volume bars, rising price indicator |
| `bear` | Down candles, bear wicks, bear volume bars, falling price indicator |
| `grid` | Gridlines |
| `axisText` | Price + time axis label text |
| `crosshair` | Crosshair dashed lines |
| `crosshairTarget` | The hollow ring/dot at the crosshair intersection |

See [`VroomTheme`](../reference/index.md) for the canonical field list.
