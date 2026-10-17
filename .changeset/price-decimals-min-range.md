---
'react-native-vroom-chart': minor
'@vroomchart/react': minor
'@vroomchart/core-wasm': minor
---

Add two opt-in theme keys for assets that barely move. `priceDecimals` prints every price label and badge at a fixed number of decimals, and spaces the price ticks so no two read the same. `minPriceRangeRatio` sets the narrowest range auto-fit will show, as a fraction of the visible midpoint, so `0.02` keeps a stablecoin's noise inside ±1%. Both default to the current behavior.
