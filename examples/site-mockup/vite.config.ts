import { fileURLToPath } from 'node:url';
import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

const src = (p: string) => fileURLToPath(new URL(p, import.meta.url));

// No COOP/COEP here, unlike web-demo: the WASM core is single-threaded, and
// cross-origin isolation would block the Google Fonts and Hyperliquid requests.
export default defineConfig({
  plugins: [react()],
  resolve: {
    dedupe: ['react', 'react-dom'],
    alias: {
      '@vroomchart/react': src('../../packages/react/src/index.ts'),
      '@vroomchart/core-wasm': src('../../packages/core-wasm/src/index.ts'),
      '@vroomchart/types': src('../../packages/types/src/index.ts'),
    },
  },
});
