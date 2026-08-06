# Android bridge

Android equivalent of `../ios/`: a small TurboModule
([`VroomChartModule.kt`](src/main/java/com/vroom/chart/VroomChartModule.kt)) whose
`install()` hands the JSI runtime pointer to a native `vroomchart` library
(built by [`CMakeLists.txt`](CMakeLists.txt)) that installs
`global.VroomChartJSI` via the same platform-agnostic
[`../cpp/VroomJsiInstaller.cpp`](../cpp/VroomJsiInstaller.cpp) the iOS bridge
uses.

The chart core itself (`../cpp/_core_src`) and the RN-Skia font/context glue
(`../cpp/VroomSkiaContext.*`) are unmodified and shared across iOS and
Android — only the platform glue (this directory + `../ios/`) differs.
`../cpp/VroomChartHostObject.cpp` has one small `#if defined(__ANDROID__)`
branch (see "Cross-`.so` Skia objects" below).

`build.gradle` resolves `@shopify/react-native-skia`'s `cpp/` sources via
Node module resolution (mirroring `../react-native-vroom-chart.podspec`'s
`require.resolve` trick) so `VroomSkiaContext.cpp` can include RN-Skia's
`JsiSkHostObjects.h` / `RNSkPlatformContext.h`. `CMakeLists.txt` also
replicates RN-Skia's own Graphite-vs-Ganesh detection (probing its bundled
`libskia.a`) so `SK_GRAPHITE` is defined identically to however RN-Skia's own
Android build was compiled — `RNSkPlatformContext`'s vtable layout depends on
that macro, and a mismatch would corrupt the virtual call in
`VroomJsiInstaller.cpp`'s `ensureAxisTypeface`.

We link two RN-Skia-adjacent libraries into `libvroomchart.so`, for two
different reasons:
  - `shopify_react-native-skia::rnskia` (RN-Skia's own prefab-published
    shared library, `librnskia.so`) — for `RNJsi::JsiHostObject` and friends,
    whose out-of-line methods are compiled only there.
  - RN-Skia's bundled `libskia.a` (imported directly by path, since it isn't
    prefab-published) — for direct Skia calls (`SkCanvas::drawRect`, `SkFont`,
    ...), since `librnskia.so` links `libskia.a` internally but doesn't
    re-export its symbols.

### Cross-`.so` Skia objects

Unlike iOS (one static binary), Android compiles RN-Skia into its own
`librnskia.so`, separate from `libvroomchart.so`. That's transparent for most
of the JSI bridge, but **`RNSkia::JsiSkPicture` — the object `render()` /
`pan()` / etc. return to JS — can't be constructed directly in
`libvroomchart.so` on Android.** RN-Skia's own C++ (e.g.
`cpp/api/recorder/Convertor.h`'s `getPropertyValue<sk_sp<SkPicture>>`, which
runs whenever `<Picture>` reads the value) does
`value.asObject(rt).asHostObject<JsiSkPicture>(rt)`, a `dynamic_pointer_cast`
under the hood. A `JsiSkPicture` we construct ourselves has a vtable/typeinfo
compiled into *our* `.so`; that cast — running inside `librnskia.so` — sees a
different, unmerged RTTI record for "the same" class and fails with `Object
is not a HostObject of desired type`.

`VroomChartHostObject.cpp`'s `wrapPicture()` works around this only on
Android: instead of constructing `RNSkia::JsiSkPicture` directly (as iOS
does), it serializes the `SkPicture` (`SkPicture::serialize()`) and calls
RN-Skia's own public JS API, `Skia.Picture.MakePicture(bytes)`, via JSI. That
runs `JsiSkPictureFactory::MakePicture` — genuinely compiled inside
`librnskia.so` — so the returned object's RTTI matches what
`librnskia.so`'s own code expects. This costs a serialize + re-parse of the
picture's draw commands on every `render()`/gesture call, which is real
overhead on what's meant to be a 60fps hot path; if that shows up in
profiling, revisit (e.g. a merged-`.so` build, or a lighter-weight bridge
that avoids the round trip).
