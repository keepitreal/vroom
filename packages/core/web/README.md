# vroom Skia-WASM core (web)

This directory holds the WebAssembly wrapper (`vroom_web.cpp`) that compiles the
C++ chart core together with Skia into a single emscripten ES module. It owns a
WebGL-backed Skia surface bound to an HTML `<canvas>` and paints it directly, so
the web chart renders **pixel-identically to native** — no SkPicture
serialization, no second renderer.

Architecture context: this is the deferred "A1c" half of the web port. The
TypeScript side (`@vroom/core-wasm`, `@vroom/react`) is already complete and runs
today against a Canvas2D **stub**; building this module and pointing `loadVroom`
at it swaps in the real core with no API changes.

> Status: **build-ready but not yet compiled in CI.** The C++/CMake/build script
> here have not been run through emscripten yet — expect to adjust a few Skia GPU
> symbol names to match the exact Skia revision you build against (they moved
> during Skia's "Ganesh" reorg; call-sites are commented in `vroom_web.cpp`).

## Toolchain (the heavy part)

You need two things on your machine (or in CI):

### 1. emscripten

```sh
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
source ./emsdk_env.sh   # puts emcmake/emcc on PATH
```

### 2. Skia, built for WASM (a static `libskia.a` + headers)

This is the long pole — Skia uses its own GN/Ninja build via `depot_tools`.
Roughly:

```sh
git clone https://skia.googlesource.com/skia.git
cd skia
python3 tools/git-sync-deps
bin/fetch-gn

# WASM build with the GL (Ganesh) backend, GPU on. Trim codecs you don't need,
# but KEEP font/text (we draw axis labels). Use emscripten's clang.
bin/gn gen out/wasm --args='
  is_official_build=true
  target_cpu="wasm"
  skia_use_freetype=true
  skia_use_system_freetype2=false
  skia_enable_ganesh=true
  skia_use_gl=true
  skia_use_webgl=true
  skia_enable_fontmgr_custom_directory=false
  skia_enable_fontmgr_custom_embedded=false
  skia_enable_fontmgr_custom_empty=true
  skia_use_dng_sdk=false skia_use_expat=false skia_use_libjpeg_turbo_decode=false
'
ninja -C out/wasm skia    # target is `skia`; produces out/wasm/libskia.a
```

Match the Skia revision to the one `@shopify/react-native-skia` ships on native
if you want guaranteed parity. (Tip: CanvasKit's own build args are the best
reference if the above needs tuning.)

## Build the module

```sh
SKIA_DIR=/abs/path/to/skia pnpm --filter @vroom/core-wasm build:wasm
# or directly:
SKIA_DIR=/abs/path/to/skia packages/core/scripts/build-wasm.sh
```

Output lands in `packages/core-wasm/wasm/`:

- `vroom_core.mjs` — the emscripten ES module (default export is the factory)
- `vroom_core.wasm` — the binary

## Wire it into an app

The web component uses the stub by default. To use the real core, pass `wasm`
URLs through `loadVroom` (these are served static assets in your app):

```ts
import { loadVroom } from '@vroom/core-wasm';

await loadVroom({
  wasm: {
    moduleUrl: '/vroom/vroom_core.mjs',
    wasmUrl: '/vroom/vroom_core.wasm',
    fontUrl: '/vroom/Inter-Regular.ttf', // any .ttf/.otf; required for labels
  },
});
```

`@vroom/react` will expose this via a prop in a follow-up; for now `loadVroom`
is the seam.

### Fonts

The WASM sandbox has no system fonts, so you must ship a `.ttf`/`.otf` and pass
its URL as `fontUrl`. The loader fetches it and installs it as the axis typeface
(`WebChart.setTypeface` → `vroom::set_axis_typeface`). Without it, candles/axes
draw but text labels are blank.

## SSR

The module touches `WebGL`/`canvas`, so it is client-only. In Next.js, load the
chart with `dynamic(() => import('...'), { ssr: false })`, or only call
`loadVroom({ wasm })` in the browser.
