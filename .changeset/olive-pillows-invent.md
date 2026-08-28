---
'@vroomchart/core-wasm': minor
'@vroomchart/react': minor
'react-native-vroom-chart': patch
---

Expose the selected drawing, add `fill` and `locked`, and let managed mode restyle

Four additions for hosts building formatting UI around drawings (web only):

- **`onSelectionChange`** fires with the selected `Drawing` and its `rect` — the
  bounds in CSS px relative to the chart container — and again whenever that rect
  moves. The rect is recomputed from live core state once per painted frame, so a
  floating toolbar anchored to it tracks the drawing through pans, zooms, resizes
  and drags rather than snapping into place when the gesture ends. Mid-drag,
  `rect` is live while `selection.drawing` still holds the last committed
  geometry, which reaches you through `onDrawingChange` on release.
- **`fill`** on `BoxDrawing` paints the rectangle's interior beneath the stroke.
  Note that vroom reads 8-digit hex as `#aarrggbb`, not CSS's `#rrggbbaa` — a
  green at 33% is `'#5400ce2c'`. Boxes without a `fill` are unchanged.
- **`locked`** on any drawing keeps it selectable — so a toolbar can offer to
  unlock it — while blocking drags, reshapes and delete. Its grab handles are no
  longer drawn, since nothing about it can be grabbed.
- **`restyle(id, patch)`** on the object published through `historyRef`, for
  changing a drawing's appearance in managed mode (where the chart owns the
  drawings array). It records one undo step and persists like any other edit.
  That type is now named `DrawingControls`; `UndoRedoControls` remains as a
  deprecated alias.

Drawings created by a gesture now carry their stroke `color` and `width`
explicitly instead of leaving them unset, so a host color swatch shows what is
actually on screen. Rendering is unchanged.

`react-native-vroom-chart` bundles the C++ core, so it takes a patch for the
rebuilt binary; drawings remain web-only there.
