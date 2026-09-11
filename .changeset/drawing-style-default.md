---
'@vroomchart/react': minor
'react-native-vroom-chart': minor
---

Add a host-controlled default drawing style

New drawings always stamped a hardcoded `#ff2962ff` stroke, so a host fallback
(`drawing.color ?? accent`) never won, and CSS swatches previewed that 8-digit
`#aarrggbb` as red. Pass `drawingStyle` on web and RN `VroomChart` to set the
stroke (and draft preview) for newly created line, box, pencil, and path
drawings — 6-digit hex like `'#00FFFF'` is treated as opaque. `fill` applies to
new boxes only; omit it to keep the 10% stroke tint. Paste still copies the
source drawing's style.

Drawings remain web-only on React Native; the prop type-checks there so the
surfaces stay aligned.

