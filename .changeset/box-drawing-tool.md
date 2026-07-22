---
"@vroomchart/core-wasm": minor
"@vroomchart/react": minor
"react-native-vroom-chart": minor
---

New **box** drawing tool (web) — an axis-aligned rectangle annotation, alongside the
existing line:

- **`tool="box"`** — in `draw` mode, click to place one corner and click again to
  commit. A live preview rectangle tracks the cursor between the two clicks; hold
  **Shift** to constrain it to a perfect square.
- **4 corner anchors** — selecting a box shows a handle on each corner. Dragging one
  resizes the box from that corner while the diagonally opposite corner stays fixed,
  so all four corners always stay at 90°. Shift snaps to a square here too.
- **Move / edit** — the faint interior fill is grabbable to drag the whole box; delete
  and copy/paste work exactly as they do for lines.
- **Persistence** — boxes serialize through the same `drawingStore` envelope as lines.
  The `Drawing` type's `type` field widens to `'line' | 'box'`, with `points` holding
  the two opposite corners; `DrawTool` widens to `'line' | 'box'`.

Additive — existing line drawings and their behavior are unchanged.
