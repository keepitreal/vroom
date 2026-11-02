---
'react-native-vroom-chart': minor
'@vroomchart/core-wasm': patch
'@vroomchart/react': patch
---

New `transitionKey` prop (React Native): when it changes on the same `seriesKey`, the data change crossfades as a timeframe switch, even with an unchanged candle step or an explicit `visibleRange` (which it lands on). The timeframe morph now runs on the core's frame clock, so a busy JS thread can't freeze or skip it, and live ticks during it no longer cut it. An `intervalTransition: 'fade'` keeps the outgoing scene's theme, current-price line and footprint badges, and fades the price badge, price lines and FVG labels in with the incoming scene instead of popping them in.
