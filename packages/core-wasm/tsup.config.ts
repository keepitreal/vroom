import { defineConfig } from 'tsup';

export default defineConfig({
  entry: ['src/index.ts'],
  format: ['esm'],
  // resolve: inline @vroom/types into the .d.ts (it's types-only and not a
  // published dependency). noExternal does the same for the JS bundle.
  dts: { resolve: ['@vroom/types'] },
  clean: true,
  sourcemap: true,
  target: 'es2020',
  noExternal: ['@vroom/types'],
});
