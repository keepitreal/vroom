import { fileURLToPath } from 'node:url';
import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

// Cross-origin isolation headers — required if the Skia-WASM build uses pthreads
// (SharedArrayBuffer). Harmless otherwise; all demo assets are same-origin.
const coopCoep = {
  'Cross-Origin-Opener-Policy': 'same-origin',
  'Cross-Origin-Embedder-Policy': 'require-corp',
};

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
  server: { headers: coopCoep },
  preview: { headers: coopCoep },
});
