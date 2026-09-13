---
'@vroomchart/core-wasm': minor
'react-native-vroom-chart': minor
'@vroomchart/react': minor
---

Add inverse Fair Value Gaps, opt-in through `showInverse` on the existing
`fairValueGaps` prop. A gap that price closes through keeps its place on the
chart with its polarity flipped — a violated bullish gap becomes bearish
resistance overhead, a violated bearish gap becomes bullish support — and stays
there until price reclaims the band by the same rule `fillType` sets.

The inverse box starts where the original one stops, so the default pairing with
`deleteAfterFill` reads as one continuous story. It takes its own fill colors
through `inverseBullishColor` / `inverseBearishColor` and labels itself `iFVG`,
overridable with `inverseLabel`.
