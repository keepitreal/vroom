import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

// Cross-origin isolation headers — required if the Skia-WASM build uses pthreads
// (SharedArrayBuffer). Harmless otherwise; all demo assets are same-origin.
const coopCoep = {
  'Cross-Origin-Opener-Policy': 'same-origin',
  'Cross-Origin-Embedder-Policy': 'require-corp',
};

export default defineConfig({
  plugins: [react()],
  // The workspace packages ship TS source; dedupe React so hooks share one copy.
  resolve: { dedupe: ['react', 'react-dom'] },
  server: { headers: coopCoep },
  preview: { headers: coopCoep },
});
