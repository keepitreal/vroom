---
"react-native-vroom-chart": minor
---

Implement the Android native bridge. `VroomChart` now renders and responds to
gestures (pan, pinch, crosshair, axis scaling) on Android via a JNI/JSI bridge
into the same C++ chart core and RN-Skia integration the iOS bridge uses.
Previously the Android TurboModule was a placeholder and `VroomChart` was
non-functional on Android.
