import { defineConfig } from 'tsup';

export default defineConfig({
  entry: ['src/index.ts'],
  format: ['esm'],
  // resolve: inline @vroomchart/types into the .d.ts (it's types-only and not a
  // published dependency). noExternal does the same for the JS bundle.
  dts: { resolve: ['@vroomchart/types'] },
  clean: true,
  sourcemap: true,
  target: 'es2020',
  noExternal: ['@vroomchart/types'],
});
