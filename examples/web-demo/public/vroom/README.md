# WASM core assets (served at /vroom/)

The web-demo loads the real Skia-WASM core from here when opened with `?wasm=1`.
These files are produced by the build and are gitignored — drop them in by:

    SKIA_DIR=/abs/path/to/skia COPY_TO_DEMO=1 pnpm --filter @vroom/core-wasm build:wasm

which copies `vroom_core.mjs` + `vroom_core.wasm` here. Then add a font:

    Inter-Regular.ttf   (any .ttf/.otf; required for axis/price/time labels)

With all three present, run `pnpm --filter web-demo dev` and open `/?wasm=1`.
