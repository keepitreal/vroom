import { defineConfig } from 'tsup';

export default defineConfig({
  entry: ['src/index.ts'],
  // CJS too, for the React Native package's CommonJS build.
  format: ['esm', 'cjs'],
  // Inline @vroomchart/types into the .d.ts (types-only, not published).
  dts: { resolve: ['@vroomchart/types'] },
  clean: true,
  sourcemap: true,
  target: 'es2020',
  noExternal: ['@vroomchart/types'],
});
