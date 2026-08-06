---
"react-native-vroom-chart": patch
---

Fix axis labels (price and time) never rendering. Two bugs compounded: on
Android, obtaining RN-Skia's platform context used a `dynamic_pointer_cast`
that always fails across the `librnskia.so`/`libvroomchart.so` boundary (same
class of cross-`.so` RTTI mismatch as the `JsiSkPicture` bridge), so the axis
typeface was never loaded at all; separately, on both platforms, requesting
the "default" system font with a null family name isn't honored by Android's
font manager (`SkFontMgr_New_Android`), which now falls back to `sans-serif`.
