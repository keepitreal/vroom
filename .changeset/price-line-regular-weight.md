---
'@vroomchart/core-wasm': patch
'react-native-vroom-chart': patch
---

Draw price-line labels at the axis typeface's regular weight. The renderer was applying a synthetic bold on top of that face, which left a gray halo on the stems, especially on high-DPI screens.

`priceLinesStyle.fontSize` is an integer CSS pixel size, clamped to 10–14. Omit it to inherit the axis size (11px by default).

`priceLinesStyle.cornerRadius` sets the label-pill corner radius in CSS px, clamped to 0–6. Omit it to keep the current 6px radius; 0 draws square corners.

Label pills add 1px of padding above and below the text, on top of the shared 4px inset, plus one more pixel above the glyphs. The bottom edge stays where the even padding puts it.
