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
# Canonical home for the published artifacts: @vroom/core-wasm/assets/, which is
# tracked and shipped in the package (referenced via new URL(..., import.meta.url)).
BUILD_DIR="$CORE_DIR/build-wasm"
OUT_DIR="$REPO_ROOT/packages/core-wasm/assets"

if ! command -v emcmake >/dev/null 2>&1; then
  echo "error: emcmake not found. Install + activate emsdk first (see web/README.md)." >&2
  exit 1
fi
if [[ -z "${SKIA_DIR:-}" ]]; then
  echo "error: set SKIA_DIR to your Skia source root (with include/ and a wasm libskia.a)." >&2
  exit 1
fi

# Skia names the wasm static lib libskia.wasm.a (older trees: libskia.a).
if [[ -z "${SKIA_LIB:-}" ]]; then
  if [[ -f "$SKIA_DIR/out/wasm/libskia.wasm.a" ]]; then
    SKIA_LIB="$SKIA_DIR/out/wasm/libskia.wasm.a"
  else
    SKIA_LIB="$SKIA_DIR/out/wasm/libskia.a"
  fi
fi
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

echo "==> Done. @vroom/core-wasm now bundles the updated core; rebuild dependents."
echo "    (The web-demo and any consumer pick these up via new URL(import.meta.url).)"

echo "==> Done. Point loadVroom({ wasm: { moduleUrl, wasmUrl, fontUrl } }) at the served files."
