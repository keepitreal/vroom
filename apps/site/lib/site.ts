/**
 * Canonical origin for metadata, sitemap, and OG URLs. Previews and local dev
 * still point canonicals at production; robots.ts keeps previews unindexed.
 */
export const SITE_URL = (process.env.NEXT_PUBLIC_SITE_URL ?? 'https://www.vroomvroom.dev').replace(/\/$/, '');

export const SITE_NAME = 'vroom';
export const SITE_TITLE = 'vroom: candlestick charts that keep up';
export const SITE_DESCRIPTION =
  'Skia-powered candlestick charts for React Native and the web. One C++ core, 60+ FPS pan, pinch, and zoom, nine built-in indicators, drawing tools, and an AI agent toolkit.';

export const GITHUB_URL = 'https://github.com/keepitreal/vroom';

export const docsRoute = '/docs';

export const NAV = [
  { label: 'Docs', href: '/docs' },
  { label: 'Quickstart', href: '/docs/getting-started/quickstart' },
  { label: 'GitHub', href: GITHUB_URL },
] as const;
