// Instantiates the Skia-WASM core (the emscripten ES module built from
// packages/core/web + CMake VROOM_WASM) and adapts it to a VroomModule.
//
// The module URL is resolved at runtime (not statically imported) so bundlers
// don't try to resolve a not-yet-built artifact — keeping the stub-only build
// green until the WASM file actually exists.

import type { VroomModule } from '../handle';
import { makeWasmModule, type VroomWasmModule } from './adapter';

export type WasmConfig = {
  /** URL to the emscripten ES module (vroom_core.mjs). */
  moduleUrl: string;
  /** URL to vroom_core.wasm. Defaults to resolving next to the module. */
  wasmUrl?: string;
  /** URL to a .ttf/.otf used for axis/price/time labels. */
  fontUrl?: string;
};

// The emscripten MODULARIZE/EXPORT_ES6 default export is a factory:
//   createVroomCore(moduleOverrides) => Promise<Module>
type EmscriptenFactory = (overrides?: Record<string, unknown>) => Promise<VroomWasmModule>;

// Import the emscripten ES module from a runtime URL WITHOUT the bundler trying
// to resolve/transform it. A plain import(url) (even with /* @vite-ignore */) is
// still intercepted by Vite's "don't import from /public" guard and by other
// bundlers' static analysis. Routing through Function keeps it fully opaque.
// (Requires 'unsafe-eval' under a strict CSP — self-host the module differently
// if that's a constraint.)
const dynamicImport = new Function('u', 'return import(u)') as (
  u: string,
) => Promise<{ default: EmscriptenFactory }>;

export async function loadWasmCore(cfg: WasmConfig): Promise<VroomModule> {
  const mod = await dynamicImport(cfg.moduleUrl);
  const instance = await mod.default({
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
