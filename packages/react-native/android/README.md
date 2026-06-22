# Android shim (placeholder)

When native is wired up, this directory will hold the JNI bindings and a
Java/Kotlin view that hosts the chart and forwards gestures into the C++ core.

`build.gradle` (also to come) will compile `cpp/` + this directory + the
linked `@vroomchart/core` static library via CMake.
