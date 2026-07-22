---
"@vroomchart/core-wasm": patch
---

The web core no longer requires `script-src 'unsafe-eval'`, so it works under a
strict production CSP. Two independent things needed eval, and both are fixed:

- **The module loader** used `new Function('u', 'return import(u)')` to hide the
  URL from bundler static analysis. It is now a plain dynamic `import()` carrying
  `/* webpackIgnore: true */` and `/* @vite-ignore */`, which webpack, Turbopack,
  Vite and Rollup all honor (verified: a Vite production build still emits
  `vroom_core.mjs` as a standalone asset rather than inlining it).
- **The emscripten module itself** — embind and emval generate their JS invoker
  functions with `new Function` at startup, so fixing only the loader would have
  moved the CSP failure into `vroom_core.mjs`. The WASM core is now built with
  `-sDYNAMIC_EXECUTION=0`, selecting emscripten's closure-based invokers. The
  shipped `vroom_core.mjs` contains zero `eval` / `new Function` call sites.

`script-src 'self' 'wasm-unsafe-eval'` is now sufficient. `'wasm-unsafe-eval'` is
still required — it is what browsers gate `WebAssembly.instantiate` behind — but
it does not permit `eval` and is far narrower than `'unsafe-eval'`.

**New `wasm.importModule` escape hatch** for toolchains that rewrite the import
anyway, since magic comments only take effect in first-party source:

```tsx
<VroomChart wasm={{ importModule: (url) => import(/* webpackIgnore: true */ url) }} />
```

`WasmConfig.moduleUrl` is now optional: options other than the URLs layer onto
the bundled assets, so overriding just the importer no longer means self-hosting
the core. Passing `moduleUrl` keeps the previous all-or-nothing behavior.
