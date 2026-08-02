# Contributing to vroom

Thanks for your interest in contributing! Here's what you need to know.

## Prerequisites

- **Node.js** ≥20
- **pnpm** 9.x (version is pinned in `package.json`'s `packageManager` field)
- **Emscripten** (emsdk) — only needed if you change the C++ core and need to rebuild WASM
- **Skia checkout** — only needed for WASM rebuilds; set `SKIA_DIR` env var

```sh
pnpm install
```

## Repository structure

```
packages/
  core/          C++ engine (Skia). Public `extern "C"` facade in vroom_chart.h.
  core-wasm/     WASM glue + JS loader. Exposes loadVroom().
  react/         React DOM wrapper around core-wasm. VroomChart component.
  react-native/  React Native wrapper. JSI bridge around the C++ core.
  types/         Shared TypeScript types (private, bundled into published packages).
docs/            Docusaurus documentation site.
examples/        test-bench (Expo dev client) + web-demo (Vite).
```

## Development workflow

### 1. Make your change

Pick the right package based on what you're changing:

| You changed | Rebuild |
|---|---|
| C++ core (`packages/core/`) | `build:wasm` → `build:packages` |
| WASM glue (`packages/core-wasm/src/`) | `core-wasm build` → `react build` |
| Web component (`packages/react/src/`) | `react build` |
| RN component (`packages/react-native/src/`) | `react-native build` |
| Types (`packages/types/src/`) | `build:packages` (rebuilds all consumers) |

### 2. Build and verify

```sh
# Build all packages
pnpm build:packages

# Typecheck everything
pnpm -r --filter './packages/*' run typecheck

# Run TS unit tests
pnpm --filter @vroomchart/react --filter react-native-vroom-chart run test

# Run C++ unit tests
pnpm test:core

# Full CI check locally
pnpm build:packages && pnpm -r --filter './packages/*' run typecheck && pnpm --filter @vroomchart/react --filter react-native-vroom-chart run test && pnpm audit --audit-level high
```

### 3. Rebuild WASM (C++ core changes only)

```sh
source ~/emsdk/emsdk_env.sh
SKIA_DIR=/Users/darion/skia pnpm --filter @vroomchart/core-wasm build:wasm
```

Output lands in `packages/core-wasm/assets/`. CI skips this step (binaries are committed).

### 4. Update docs

If your change affects public types or props:

```sh
pnpm docs:build
```

This regenerates `docs/content/reference/` from source types. Commit the result — CI verifies it's fresh.

### 5. Add a changeset

Every PR that touches a published package needs a changeset:

```sh
pnpm changeset
```

Select the affected packages and choose a semver bump (major/minor/patch). Commit the generated `.changeset/*.md` file.

### 6. Submit your PR

- Use [Conventional Commits](https://www.conventionalcommits.org/) (`feat:`, `fix:`, `docs:`, `chore:`, etc.)
- CI must pass: build + typecheck + test + audit + docs freshness check
- Include a changeset if touching published packages

## Code conventions

- **TypeScript**: strict mode, ES modules, named exports (no default exports)
- **C++**: C++17, public surface is `extern "C"`, internal code uses modern C++
- **Commits**: `type(scope): message` — common scopes: `react`, `core-wasm`, `react-native`, `core`, `docs`, `web-demo`

## Testing

- **TS unit tests**: vitest, colocated with source (`*.test.ts`)
- **C++ unit tests**: doctest, in `packages/core/tests/` (Skia-free TUs only)
- **Visual testing**: run `pnpm --filter web-demo dev --port 5199` and drive with Playwright

## Architecture

vroom uses a shared C++ core (Skia renderer) exposed through an `extern "C"` facade. Platform wrappers own the canvas surface and forward gesture events:

- **React Native**: JSI bridge → `react-native-vroom-chart`
- **Web**: Emscripten/WASM → `@vroomchart/core-wasm` → `@vroomchart/react`
- **Planned**: JNI for Android-native, Swift for iOS-native

The core owns all drawing. Platform wrappers provide an `SkCanvas` and forward gesture events. WASM assets are bundled inside `@vroomchart/core-wasm`; consumers don't host them.

## Need help?

Open an issue on [GitHub](https://github.com/keepitreal/vroom/issues).
