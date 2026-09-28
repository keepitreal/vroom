---
'@vroomchart/core-wasm': patch
'react-native-vroom-chart': patch
---

Draw price-line labels at the axis typeface's regular weight. The renderer was applying a synthetic bold on top of that face, which left a gray halo on the stems, especially on high-DPI screens.

`priceLinesStyle.fontSize` is an integer CSS pixel size, clamped to 10–14. Omit it to follow `theme.axisFontSize`.

`theme.axisFontSize` is that same 10–14px range for the price ticks, time labels, current-price badge, crosshair badges, and indicator-pane labels. Omit it for 11.

`priceLinesStyle.cornerRadius` sets the label-pill corner radius in CSS px, clamped to 0–6. Omit it to keep the current 6px radius; 0 draws square corners.

Label pills add 1px of padding above and below the text, on top of the shared 4px inset, plus one more pixel above the glyphs. The bottom edge stays where the even padding puts it.

The y-axis strip grows to the widest price badge in the column — tick labels, the current-price pill, and price-line pills at their own font size — and keeps 8px between that badge and the canvas edge, so a larger price-line font no longer clips the right side. The labels stay centered on one column.
