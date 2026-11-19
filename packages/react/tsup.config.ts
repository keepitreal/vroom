import { defineConfig } from 'tsup';

export default defineConfig({
  entry: ['src/index.ts'],
  format: ['esm'],
  // Inline @vroomchart/types into the .d.ts (types-only, not published). Keep
  // @vroomchart/core-wasm and @vroomchart/strategies external — they're
  // published runtime dependencies.
  dts: { resolve: ['@vroomchart/types'] },
  clean: true,
  sourcemap: true,
  target: 'es2020',
  external: ['react', 'react-dom', '@vroomchart/core-wasm', '@vroomchart/strategies'],
  noExternal: ['@vroomchart/types'],
});
