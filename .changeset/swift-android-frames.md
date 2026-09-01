---
'react-native-vroom-chart': patch
---

Stop Android pan/zoom from OOM-killing the app.

Every gesture frame serialized the chart `SkPicture` into RN-Skia, and the
default serializer embedded the system typeface (megabytes) each time. RSS
climbed to multiple GB in seconds and the process vanished with no JS error.
Android now rasterizes to an `SkImage` (one framebuffer) and only falls back
to serialize without embedding font bytes.
