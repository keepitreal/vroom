// Instantiates the Skia-WASM core (the emscripten ES module built from
// packages/core/web + CMake VROOM_WASM) and adapts it to a VroomModule.
//
// The module URL is resolved at runtime (not statically imported) so bundlers
// don't pre-bundle the emscripten glue — it must stay an opaque asset.

import type { VroomModule } from '../handle';
import { makeWasmModule, type VroomWasmModule } from './adapter';

export type WasmConfig = {
  /**
   * URL to the emscripten ES module (vroom_core.mjs). Omit to use the build
   * bundled in this package — in which case `wasmUrl` and `fontUrl` default to
   * the bundled assets too.
   */
  moduleUrl?: string;
  /** URL to vroom_core.wasm. Defaults to resolving next to the module. */
  wasmUrl?: string;
  /** URL to a .ttf/.otf used for axis/price/time labels. */
  fontUrl?: string;
  /**
   * Custom loader for the emscripten ES module, if the default `import(url)`
   * doesn't survive your bundler. Bundler magic comments only take effect in
   * your own source, so this is the seam for supplying them:
   *
   * ```ts
   * // webpack / Next.js
   * importModule: (url) => import(/* webpackIgnore: true *\/ url)
   * // Vite / Rollup
   * importModule: (url) => import(/* @vite-ignore *\/ url)
   * ```
   */
  importModule?: (url: string) => Promise<unknown>;
};

/** A WasmConfig with the module URL resolved. */
export type ResolvedWasmConfig = WasmConfig & { moduleUrl: string };

// The emscripten MODULARIZE/EXPORT_ES6 default export is a factory:
//   createVroomCore(moduleOverrides) => Promise<Module>
type EmscriptenFactory = (overrides?: Record<string, unknown>) => Promise<VroomWasmModule>;

// A plain dynamic import — NOT `new Function('return import(u)')`, which would
// require a `script-src 'unsafe-eval'` CSP that production apps rightly refuse.
//
// The magic comments keep bundlers from trying to resolve/transform the URL at
// build time: it points at an emitted asset, and the emscripten glue breaks if
// it's pre-bundled. Both comments are inert everywhere else. Bundlers that honor
// neither should be handed a `cfg.importModule` instead.
const defaultImport = (url: string): Promise<unknown> =>
  import(/* webpackIgnore: true */ /* @vite-ignore */ url);

export async function loadWasmCore(cfg: ResolvedWasmConfig): Promise<VroomModule> {
  const importModule = cfg.importModule ?? defaultImport;

  let factory: EmscriptenFactory;
  try {
    factory = ((await importModule(cfg.moduleUrl)) as { default: EmscriptenFactory }).default;
  } catch (e) {
    throw new Error(
      `[vroom] failed to import the WASM core from ${cfg.moduleUrl}. If your bundler ` +
        'rewrote or inlined the URL, pass a custom `wasm.importModule` ' +
        '(see @vroomchart/core-wasm README → Bundlers & CSP).',
      { cause: e },
    );
  }

  const instance = await factory({
    locateFile: (path: string) =>
      path.endsWith('.wasm') && cfg.wasmUrl ? cfg.wasmUrl : path,
  });

  let fontBytes: Uint8Array | null = null;
  if (cfg.fontUrl) {
    try {
      const res = await fetch(cfg.fontUrl);
      if (res.ok) {
        fontBytes = new Uint8Array(await res.arrayBuffer());
        console.info(`[vroom] axis font loaded (${cfg.fontUrl}, ${fontBytes.length} bytes)`);
      } else {
        console.warn(`[vroom] axis font fetch ${cfg.fontUrl} → HTTP ${res.status}; labels will be blank.`);
      }
    } catch (e) {
      console.warn(`[vroom] axis font fetch failed (${cfg.fontUrl}); labels will be blank.`, e);
    }
  } else {
    console.warn('[vroom] no fontUrl provided; text labels will be blank.');
  }

  return makeWasmModule(instance, fontBytes);
}
