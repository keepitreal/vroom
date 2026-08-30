---
'react-native-vroom-chart': patch
'@vroomchart/react': patch
---

Fix the line-tip pulse freezing after a candle → line switch.

With `lineTipPulse` on, the ring stopped animating once you switched to candle
mode and back. The chart drives two independent frame loops: the candle↔line
morph runs its own clock, and a second loop sustains anything the core reports
as still animating. Settling in line mode is the moment the pulse turns on, but
the morph loop ended without handing frames over, so the ring sat frozen until
an unrelated repaint — a pan, a zoom, a data push — happened to restart the
other loop.

Every exit from the morph now hands off: the animated completion, the
`transitionMs: 0` snap, the fresh-handle snap, and the already-at-target
no-op. Settling in candle mode is unaffected, since there is nothing to
animate. Reduced motion is also unaffected, as it disables the pulse outright.
