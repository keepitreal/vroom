---
'@vroomchart/react': patch
---

Fix drawing edits jumping back or lagging when the chart's props change mid-drag. Streamed `candles`, a resize or any other prop update re-pushed the committed `drawings` and wiped the live preview: a dragged handle snapped back to where it started, and a translated pencil or path trailed the cursor until release. The edit in progress is now restated right after each re-sync, and a drag is cancelled if the host removes or replaces the drawing being dragged. The `drawings` dependency key is also only serialized once per array instead of on every render.
