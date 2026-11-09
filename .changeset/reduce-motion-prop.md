---
"react-native-vroom-chart": minor
"@vroomchart/react": minor
"@vroomchart/core-wasm": minor
---

Add a `reduceMotion` prop that turns every animation off, for low-end devices or any host that wants a static chart. `true` behaves as an OS reduced-motion request does; `false` or omitted still follows the OS preference, so the prop can't override a user who asked for less motion.

The OS preference now also snaps the axis-label fades, which it left animating before. The core gains a `ReduceMotion` float key (`VROOM_FLOAT_REDUCE_MOTION`) for the animations it paces itself, and it drops the line-tip pulse too. On React Native, momentum after a fling is skipped under reduced motion.

Under reduced motion the candle↔line switch now lands at once instead of crossfading.
