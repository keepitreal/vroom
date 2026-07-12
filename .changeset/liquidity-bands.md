---
"@vroomchart/core-wasm": patch
"@vroomchart/react": patch
---

feat: liquidity bands overlay (order-book depth)

Render resting-order / L2 liquidity as horizontal price-interval bands behind the
candles. Each band is anchored at the inner edge of the price axis, stretches left
and fades out with a gradient, is colored by buy/sell side, and takes its opacity
from the order volume. Bands are defined in price space so they scale with the
y-axis. New controlled `liquidity` prop on `<VroomChart>` (`LiquidityConfig` /
`LiquidityBand`), plumbed through the core (`vroom_chart_set_liquidity`), the WASM
handle (`setLiquidity`), and React.
