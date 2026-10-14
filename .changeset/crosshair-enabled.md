---
'react-native-vroom-chart': minor
'@vroomchart/react': minor
---

Add a `crosshairEnabled` prop (default `true`). Set it to `false` for a sparkline-style chart: hover, long-press and press-drag no longer raise the crosshair or fire `onCrosshair`, and one already showing is dismissed with a `'hide'`. Pan and zoom still work, and a `crosshairOverride` still draws. Pair it with `theme.showXAxis` / `theme.showYAxis` to hide the axes.
