import { defineConfig } from 'tsup';

export default defineConfig({
  entry: ['src/index.ts'],
  format: ['cjs', 'esm'],
  // Inline @vroom/types into the .d.ts so it isn't a published dependency.
  // RN peers stay external; Metro consumes src/ via the "react-native" field.
  dts: { resolve: ['@vroom/types'] },
  clean: true,
  sourcemap: true,
  target: 'es2020',
  outDir: 'lib',
  external: [
    'react',
    'react-native',
    '@shopify/react-native-skia',
    'react-native-gesture-handler',
    'react-native-reanimated',
  ],
  noExternal: ['@vroom/types'],
});
