#!/usr/bin/env bash
# Build the vroom Skia-WASM core (A1c). Requires the emscripten toolchain on
# PATH (emcmake/emcc) and a Skia source tree with a wasm-built libskia.a.
#
# Usage:
#   SKIA_DIR=/abs/path/to/skia ./packages/core/scripts/build-wasm.sh
#   # optional: SKIA_LIB=/abs/path/to/libskia.a (defaults to $SKIA_DIR/out/wasm/libskia.a)
#
# Output: packages/core-wasm/wasm/vroom_core.mjs + vroom_core.wasm
# See packages/core/web/README.md for toolchain setup (emsdk + building Skia).
set -euo pipefail

CORE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
REPO_ROOT="$(cd "$CORE_DIR/../.." && pwd)"
BUILD_DIR="$CORE_DIR/build-wasm"
OUT_DIR="$REPO_ROOT/packages/core-wasm/wasm"

if ! command -v emcmake >/dev/null 2>&1; then
  echo "error: emcmake not found. Install + activate emsdk first (see web/README.md)." >&2
  exit 1
fi
if [[ -z "${SKIA_DIR:-}" ]]; then
  echo "error: set SKIA_DIR to your Skia source root (with include/ and a wasm libskia.a)." >&2
  exit 1
fi

SKIA_LIB="${SKIA_LIB:-$SKIA_DIR/out/wasm/libskia.a}"
if [[ ! -f "$SKIA_LIB" ]]; then
  echo "error: Skia static lib not found at $SKIA_LIB (build Skia for wasm, or set SKIA_LIB)." >&2
  exit 1
fi

echo "==> Configuring (SKIA_DIR=$SKIA_DIR)"
emcmake cmake -S "$CORE_DIR" -B "$BUILD_DIR" \
  -DVROOM_WASM=ON \
  -DSKIA_DIR="$SKIA_DIR" \
  -DSKIA_LIB="$SKIA_LIB" \
  -DCMAKE_BUILD_TYPE=Release

echo "==> Building"
cmake --build "$BUILD_DIR" -j

echo "==> Publishing artifacts to $OUT_DIR"
mkdir -p "$OUT_DIR"
cp "$BUILD_DIR/vroom_core.mjs" "$OUT_DIR/"
cp "$BUILD_DIR/vroom_core.wasm" "$OUT_DIR/"

# Optional: also drop them where the web-demo serves them, so `?wasm=1` works.
if [[ "${COPY_TO_DEMO:-}" == "1" ]]; then
  DEMO_DIR="$REPO_ROOT/examples/web-demo/public/vroom"
  echo "==> Copying to web-demo ($DEMO_DIR)"
  mkdir -p "$DEMO_DIR"
  cp "$BUILD_DIR/vroom_core.mjs" "$DEMO_DIR/"
  cp "$BUILD_DIR/vroom_core.wasm" "$DEMO_DIR/"
  if [[ ! -f "$DEMO_DIR/Inter-Regular.ttf" ]]; then
    echo "    note: drop a font at $DEMO_DIR/Inter-Regular.ttf (labels need it)."
  fi
fi

echo "==> Done. Point loadVroom({ wasm: { moduleUrl, wasmUrl, fontUrl } }) at the served files."
