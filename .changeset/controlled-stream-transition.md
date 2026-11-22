---
'@vroomchart/react': patch
'react-native-vroom-chart': patch
---

`streamTransition: 'transform'` now eases live ticks on charts with a controlled `visibleRange`. Previously any explicit range turned the animation off and every tick snapped. The window still never slides on its own when the host controls it, and a swap to a different series is never morphed.
