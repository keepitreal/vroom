---
'@vroomchart/core-wasm': minor
'@vroomchart/react': minor
'react-native-vroom-chart': minor
---

**The line chart's corners can now be rounded** with a new `lineTension` theme
field, from `0` (the existing straight segments between closes) to `1` (fully
smooth). Defaults to `0`, so nothing changes without opting in:

```tsx
theme={{ lineTension: 0.5 }}
```

The curve is monotone-limited rather than a plain spline, which matters for
price: smoothing can never overshoot into a price that didn't trade. Every peak
and trough stays on an actual close, and the curve never leaves the range of the
two closes it connects — so a rounded line can't invent a high that would read as
a level someone might trade against. The gradient fill beneath the line follows
the same path, so the two stay flush.
