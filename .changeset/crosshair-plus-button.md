---
"@vroomchart/core-wasm": minor
"@vroomchart/react": minor
"react-native-vroom-chart": minor
---

Add an opt-in crosshair plus button for placing orders from the chart. With `crosshairButton={{ enabled: true }}` a small `+` sits on the crosshair's horizontal line, directly left of the price badge. Clicking or tapping it locks the crosshair and fires `onCrosshairButton` with the price, the time and the button's rect, so the host can render its own order menu beside it. The event fires again with `'move'` whenever the locked button shifts, and with `'close'` when it's dismissed. Size, corner radius, colors, the ring around the plus, the gap and the hover highlight are all configurable.
