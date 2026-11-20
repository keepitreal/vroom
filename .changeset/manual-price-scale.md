---
'react-native-vroom-chart': minor
'@vroomchart/react': minor
'@vroomchart/core-wasm': minor
---

Add a `manualPriceScale` prop. With `autoscaleY` on, it lets a drag on the price axis take the price scale over, starting from the range on screen; a double tap on the price axis (double-click on web) hands it back to autoscale, which eases to the fit. While taken over, a vertical drag moves price and a pinch scales it. Panning or pinching the plot still never takes the scale over, and hiding the price axis hands it back. Without the prop, `autoscaleY` behaves as before: no gesture moves the price range.

A double tap / double-click on the price axis now also returns a manually scaled axis to auto-fit when `autoscaleY` is off.

In the core, `vroom_chart_scale_price_axis` no longer refuses under autoscale, `vroom_chart_zoom` scales price there once the scale has been taken over, `vroom_chart_reset_price_scale` re-engages autoscale, and the new `vroom_chart_is_price_scale_manual` reports whether the user owns the range. React Native reads it so a pinch scales each axis by its own span once the scale is taken over, and only time before that. The web takeover needs a WASM build that includes this core change.
