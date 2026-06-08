import { defineConfig } from 'vitest/config';

// Only the pure-logic TS is under test (no React/RN/native imports), so the
// plain node environment is enough — no jsdom or RN preset needed.
export default defineConfig({
  test: {
    environment: 'node',
    include: ['src/**/*.test.ts'],
  },
});
