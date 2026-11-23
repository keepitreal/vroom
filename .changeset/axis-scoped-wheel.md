---
'@vroomchart/react': patch
---

The scroll wheel now acts on the axis under the pointer. Over the price axis (or an indicator pane's axis) it rescales that axis only, the same as dragging it; over the time axis it only ever moves time, a trackpad pinch included. Previously a wheel over the price axis zoomed time, and a pinch over either axis zoomed both.
