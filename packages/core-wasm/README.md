# @vroomchart/core-wasm

Framework-agnostic web core for the vroom chart: the C++/Skia engine compiled to
WebAssembly, painting a `<canvas>` through a WebGL surface. Most apps should use
[`@vroomchart/react`](../react) instead — this is the layer underneath it.

```ts
import { loadVroom } from '@vroomchart/core-wasm';

const mod = await loadVroom();
const chart = mod.create(document.querySelector('canvas')!);
```

The WASM binary and axis font ship inside this package and are referenced with
`new URL(..., import.meta.url)`, so bundlers emit them automatically — no asset
hosting required.

## Bundlers & CSP

The core is loaded with a plain dynamic `import()` of an emitted asset URL. It
requires **no `script-src 'unsafe-eval'`** — neither this package nor the
emscripten module it loads calls `eval` or `new Function` (the WASM module is
built with `-sDYNAMIC_EXECUTION=0`, which selects embind's closure-based
invokers over its default runtime codegen).

A strict policy like this is sufficient:

```
script-src 'self' 'wasm-unsafe-eval';
```

`'wasm-unsafe-eval'` **is** required — it's what browsers gate
`WebAssembly.compile`/`instantiate` behind. It does not permit `eval`, and is
much narrower than `'unsafe-eval'`. Without it the module can't start at all.

### When your bundler mangles the import

The import site carries `/* webpackIgnore: true */` and `/* @vite-ignore */`, so
webpack, Turbopack, Vite and Rollup all leave it alone. Magic comments only take
effect in first-party source, though, so if some other toolchain rewrites or
inlines the URL, supply the import yourself:

```tsx
<VroomChart
  candles={candles}
  wasm={{ importModule: (url) => import(/* webpackIgnore: true */ url) }}
/>
```

That keeps the bundled assets — override `moduleUrl` only if you're self-hosting
the core, in which case `wasmUrl` and `fontUrl` become yours to provide too.

### SSR

The module touches `WebGL` and `canvas`, so it is client-only. Under Next.js,
render the chart from a client component and load it with
`dynamic(() => import('...'), { ssr: false })`.
