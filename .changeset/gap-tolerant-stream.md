---
"@vroomchart/react": patch
---

Fix cross-chart viewport reset with gappy candle series. Streaming an in-place update (or append) to a chart whose series has interior gaps — missing bars from downtime / illiquid periods — was misclassified as a full data reset, snapping the panned/zoomed viewport back to auto-fit. Stream detection now locates the previous last bar by timestamp (binary search) rather than a uniform-grid index, so it tolerates non-uniform grids. Most visible with two charts on different timeframes, where the deeper-history pane accumulates gaps.
