---
"@vroomchart/core-wasm": minor
"react-native-vroom-chart": minor
"@vroomchart/react": minor
---

Add consumer-supplied horizontal price status lines, in the style of
TradingView's order and position lines. Pass `priceLines` to render a line at a
price with a label group — body text, an optional quantity segment, and an
optional close button — plus a matching badge in the price axis.

Lines can be dragged vertically when `draggable` is set: `onPriceLineDrag` fires
continuously and `onPriceLineDragEnd` fires on drop, and because `priceLines` is
a controlled prop a host that rejects the move simply doesn't update state and
the line snaps back. Supplying `onPriceLineClose` renders the close button on
lines marked `closable`. Escape cancels an in-progress drag on web, where the
hovered segment also brightens and the cursor becomes `ns-resize`.

Rendering and hit-testing live in the C++ core, so web and React Native share
one implementation.
