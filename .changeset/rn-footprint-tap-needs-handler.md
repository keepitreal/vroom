---
'react-native-vroom-chart': patch
---

Footprint badges are only tap targets when `onFootprint` is passed. Without a handler a tap on a badge no longer highlights it and swallows the tap; it reaches the price-line close button or dismisses the crosshair as if the badge weren't there.
