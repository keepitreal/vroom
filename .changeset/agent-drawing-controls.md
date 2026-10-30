---
"@vroomchart/react": minor
"react-native-vroom-chart": patch
---

Groundwork for driving the chart from code and AI agents (web):

- `onViewportChange` now reports the real visible window (epoch ms) once a pan, zoom, or wheel gesture settles, and only when the window changed. Previously it always sent `(0, 0)`.
- `historyRef` (managed drawings) gains `add`, `update`, `remove`, and `list`. Each edit is one undo step and persists like a gesture.
- Drawings accept optional `label` and `meta` fields. Neither is drawn; both survive user edits and persistence.
- `MovingAverageOverlay` accepts an optional `id`.
