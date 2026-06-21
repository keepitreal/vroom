import { defineConfig } from 'tsup';

export default defineConfig({
  entry: ['src/index.ts'],
  format: ['esm'],
  // Inline @vroom/types into the .d.ts (types-only, not published). Keep
  // @vroom/core-wasm external — it's a published runtime dependency.
  dts: { resolve: ['@vroom/types'] },
  clean: true,
  sourcemap: true,
  target: 'es2020',
  external: ['react', 'react-dom', '@vroom/core-wasm'],
  noExternal: ['@vroom/types'],
});
