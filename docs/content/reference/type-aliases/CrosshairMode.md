# `CrosshairMode`

```ts
type CrosshairMode = "advanced" | "simple" | "none";
```

Source: [types/src/index.ts:1228](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1228)

Which crosshair hover, long-press and press-drag raise.
'advanced' — default: vertical and horizontal lines, a ring where they
meet, a time badge in the x-axis strip and a price (or
indicator) badge in the y-axis strip. The plus button lives here.
'simple' — one vertical line snapped to the candle, with a time badge on
top. Dragging moves it along the time axis only. Suits
sparkline-style charts. See [CrosshairConfig.simple](CrosshairConfig.md#simple).
'none' — gestures never raise a crosshair or fire `onCrosshair`, and
one already up is dismissed with a `'hide'`. Pan and zoom are
unaffected, and a `crosshairOverride` still draws.
