---
'@vroomchart/core-wasm': minor
'@vroomchart/react': minor
---

**New `path` drawing tool** — a multi-segment polyline that ends in an arrowhead,
for marking out a move leg by leg.

Set `tool="path"` in draw mode. The first click drops the starting vertex and
each click after adds a segment, with a preview segment tracking the cursor in
between. Since there's no point count to finish on, the path ends when you say
so: **Escape**, **double-click**, or **right-click** all keep it as drawn, and
**⌘Z / Backspace / Delete** take back the last vertex. Hold **Shift** to
constrain a segment to 45°. Once committed every vertex is its own drag handle,
so a leg can be reshaped without redrawing. Capped at 64 vertices.

`DrawTool` gains `'path'` and the `Drawing` union gains a `PathDrawing` variant
(variable-length `points`, like `'pencil'`). Code that already narrows on `type`
before reading `points[1]` is unaffected. `LineDrawing`, `BoxDrawing`,
`PencilDrawing` and `PathDrawing` are now all exported from `@vroomchart/react`.

Persisted payloads are unchanged in shape (no envelope version bump); a build
without the path tool drops unknown drawings on load as before.

Drawing remains web-only, so `react-native-vroom-chart` is unaffected.
