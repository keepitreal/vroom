---
'@vroomchart/core-wasm': patch
'react-native-vroom-chart': patch
---

Draw price-line labels at the axis typeface's regular weight. The renderer was applying a synthetic bold on top of that face, which left a gray halo on the stems, especially on high-DPI screens.
