# Android bridge

Android equivalent of `../ios/`: a small TurboModule
([`VroomChartModule.kt`](src/main/java/com/vroom/chart/VroomChartModule.kt)) whose
`install()` hands the JSI runtime pointer to a native `vroomchart` library
(built by [`CMakeLists.txt`](CMakeLists.txt)) that installs
`global.VroomChartJSI` via the same platform-agnostic
[`../cpp/VroomJsiInstaller.cpp`](../cpp/VroomJsiInstaller.cpp) the iOS bridge
uses.

The chart core (`../cpp/_core_src`) and the platform font-manager helper
(`../cpp/VroomFontMgr.*`) are unmodified and shared across iOS and Android —
only the platform glue (this directory + `../ios/`) differs.
`../cpp/VroomChartHostObject.cpp` has one small `#if defined(__ANDROID__)`
branch (see "Cross-`.so` Skia objects" below).

`build.gradle` resolves `@shopify/react-native-skia`'s `cpp/` sources via
Node module resolution (mirroring `../react-native-vroom-chart.podspec`'s
`require.resolve` trick) so we can include RN-Skia's `JsiSkPicture.h` /
`JsiSkNativeObjects.h`. Android prebuilt `libskia.a` lives in the sibling
`react-native-skia-android` package (or `react-native-skia-graphite-android`
when `<skia-pkg>/libs/.graphite` exists); Gradle resolves that package and
passes its `libs/` dir to CMake. Graphite vs Ganesh is the same marker file
RN-Skia uses, so `SK_GRAPHITE` / `SK_GL`+`SK_GANESH` match RN-Skia's own
build — needed for vroom's direct Skia calls, not for a virtual call into
RN-Skia (we construct the font manager ourselves).

We link two RN-Skia-adjacent libraries into `libvroomchart.so`, for two
different reasons:
  - `shopify_react-native-skia::rnskia` (RN-Skia's own prefab-published
    shared library, `librnskia.so`) — for `JsiSkPicture` / `NativeObject`
    and friends, whose out-of-line methods are compiled only there.
  - `libskia.a` from `react-native-skia-android` (imported directly by path,
    since it isn't prefab-published) — for direct Skia calls
    (`SkCanvas::drawRect`, `SkFont`, `SkFontMgr_New_Android`, ...), since
    `librnskia.so` links `libskia.a` internally but doesn't re-export its
    symbols.

### Cross-`.so` Skia objects

Unlike iOS (one static binary), Android compiles RN-Skia into its own
`librnskia.so`, separate from `libvroomchart.so`. That's transparent for most
of the JSI bridge, but **`RNSkia::JsiSkPicture` — the object `render()` /
`pan()` / etc. return to JS — can't be constructed directly in
`libvroomchart.so` on Android.** RN-Skia's own C++ (e.g.
`cpp/api/recorder/Convertor.h`'s `getPropertyValue<sk_sp<SkPicture>>`, which
runs whenever `<Picture>` reads the value) does
`getJsiObject<JsiSkPicture>(rt, value)`, a `dynamic_pointer_cast` under the
hood. A `JsiSkPicture` we construct ourselves has a vtable/typeinfo compiled
into *our* `.so`; that cast — running inside `librnskia.so` — sees a
different, unmerged RTTI record for "the same" class and fails with
`Expected a Skia object of a different type`.

`VroomChartHostObject.cpp`'s `wrapPicture()` works around this only on
Android. Two copies of Skia cannot share an `sk_sp<SkPicture>` pointer, and
serializing the chart picture (the original workaround) embedded the Android
system typeface on every pan/zoom frame — an OOM that killed the process after
a few seconds of gesturing.

The hot path now rasterizes the picture in vroom's Skia and hands RN-Skia raw
pixels via `Skia.Image.MakeImage`, so `<Image>` paints a framebuffer whose size
is bounded by the view. If rasterization fails, we still serialize, but with
`SkTypeface::SerializeBehavior::kDontIncludeData` so the font file is not
copied; `MakePicture` re-resolves `sans-serif` inside `librnskia.so`. iOS is
unchanged (one binary, direct `JsiSkPicture` wrap).
