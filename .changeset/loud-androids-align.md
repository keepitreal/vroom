---
'react-native-vroom-chart': patch
---

Align `libvroomchart.so` to 16KB ELF pages for Android 15+.

NDK r27 and older default to 4KB LOAD alignment (`align=2**12`). Apps targeting
SDK 35 then fail Play's 16KB page-size check for both arm64-v8a and x86_64.
The Android CMake target now passes `-Wl,-z,max-page-size=16384` (and
`common-page-size`) so consumers no longer need to patch the published
package. NDK r28+ already aligns to 16KB; the flags are redundant there.
