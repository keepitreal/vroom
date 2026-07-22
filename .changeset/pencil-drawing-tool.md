---
"@vroomchart/core-wasm": minor
"@vroomchart/react": minor
"react-native-vroom-chart": minor
---

New **pencil** drawing tool (web) — freehand strokes, alongside the existing line
and box:

- **`tool="pencil"`** — press and drag to draw for as long as the button is held;
  releasing commits the stroke (`onDrawingComplete`). The tool stays active so you
  can draw several strokes in a row. The path is rendered as a smoothed,
  constant-width stroke with round caps.
- **Translate-only editing** — a committed stroke is never reshaped. Selecting it
  shows anchors on its first and last point as a visual cue that it can be moved;
  dragging an anchor translates the whole path exactly like dragging any other
  part of it. Delete and copy/paste work as they do for the other tools.
- **Automatic thinning** — a drag generates hundreds of samples, so strokes are
  simplified before being committed and persisted (typically a few dozen points),
  keeping `drawingStore` payloads reasonable.

**`Drawing` is now a discriminated union.** `'line'` and `'box'` keep their exact
`[DrawPoint, DrawPoint]` tuple; `'pencil'` carries a variable-length
`DrawPoint[]`. Code that reads `points[1]` on a general `Drawing` now needs to
narrow on `type` first — existing code that already works with a known line or
box is unaffected. `LineDrawing`, `BoxDrawing` and `PencilDrawing` are exported.

Persisted payloads are unchanged in shape (no envelope version bump); a build
without the pencil tool drops unknown strokes on load as before. Deserialization
now also validates each drawing's `points`, so a corrupt entry from a store is
dropped instead of reaching the renderer.
