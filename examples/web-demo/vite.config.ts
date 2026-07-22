import { fileURLToPath } from 'node:url';
import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

// Cross-origin isolation headers — required if the Skia-WASM build uses pthreads
// (SharedArrayBuffer). Harmless otherwise; all demo assets are same-origin.
const coopCoep = {
  'Cross-Origin-Opener-Policy': 'same-origin',
  'Cross-Origin-Embedder-Policy': 'require-corp',
};

// Production-shaped CSP, served by both `dev` and `preview` so that any `eval` /
// `new Function` creeping into the chart core fails here rather than in a
// consumer app whose policy forbids it. `'wasm-unsafe-eval'` is what browsers
// gate WebAssembly.instantiate behind; it does NOT permit eval.
//
// Dev additionally needs 'unsafe-inline' (the React Refresh preamble is an
// inline script) and ws: (the HMR socket). Neither weakens the eval ban, which
// is the thing under test. `preview` runs the strict policy consumers get.
const csp = (dev: boolean) =>
  [
    "default-src 'self'",
    `script-src 'self' 'wasm-unsafe-eval'${dev ? " 'unsafe-inline'" : ''}`,
    "style-src 'self' 'unsafe-inline'",
    "img-src 'self' data: blob:",
    "font-src 'self' data:",
    `connect-src 'self'${dev ? ' ws:' : ''}`,
    "worker-src 'self' blob:",
  ].join('; ');

const src = (p: string) => fileURLToPath(new URL(p, import.meta.url));

export default defineConfig({
  plugins: [react()],
  resolve: {
    // Dedupe React so hooks share one copy.
    dedupe: ['react', 'react-dom'],
    // The published packages resolve to built dist/; in the monorepo demo, point
    // the workspace packages at their TS source so dev has live HMR (no build step).
    alias: {
      '@vroomchart/react': src('../../packages/react/src/index.ts'),
      '@vroomchart/core-wasm': src('../../packages/core-wasm/src/index.ts'),
      '@vroomchart/types': src('../../packages/types/src/index.ts'),
    },
  },
  server: { headers: { ...coopCoep, 'Content-Security-Policy': csp(true) } },
  preview: { headers: { ...coopCoep, 'Content-Security-Policy': csp(false) } },
});
