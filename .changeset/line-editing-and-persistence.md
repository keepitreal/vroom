---
"@vroomchart/core-wasm": minor
"@vroomchart/react": minor
---

Line tool editing + managed drawing persistence (web):

- **Select, edit, and delete lines** — click a line to select it (endpoint handles
  appear), drag a handle to move an endpoint, drag the line body to translate the
  whole line, and press Delete/Backspace to remove a selected line.
- **Copy / paste** a selected line with Cmd/Ctrl+C / Cmd/Ctrl+V — pasted under the
  crosshair at the same grab point, or above/below the original when the pointer
  hasn't moved.
- **Shift-constrained drawing** — hold Shift while placing the second point to snap
  the line to the nearest 45°.
- **Cancel an in-progress line** — Escape/Delete/Backspace after the first point
  cancels it while staying in draw mode.
- **Managed drawing persistence** — new optional `drawingStore` prop. Provide a
  small string key-value adapter and the chart owns, loads, and saves drawings for
  you (keyed by `seriesKey`, so they persist across timeframes but not markets),
  instead of wiring the controlled `drawings` prop + `onDrawing*` callbacks. The
  stored string is a versioned, migratable envelope owned by the library, so the
  adapter never changes as the drawing schema grows. `serializeDrawings` /
  `deserializeDrawings` helpers are exported for pre-seeding or import/export.

All additive — the controlled `drawings` model is unchanged. Drawing tools remain
web-only.
