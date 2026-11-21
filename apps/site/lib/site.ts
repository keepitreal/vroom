/**
 * Canonical origin for metadata, sitemap, and OG URLs. Set NEXT_PUBLIC_SITE_URL
 * once the custom domain is live; Vercel previews fall back to their own host.
 */
export const SITE_URL = (
  process.env.NEXT_PUBLIC_SITE_URL ??
  (process.env.VERCEL_PROJECT_PRODUCTION_URL
    ? `https://${process.env.VERCEL_PROJECT_PRODUCTION_URL}`
    : 'http://localhost:3000')
).replace(/\/$/, '');

export const SITE_NAME = 'vroom';
export const SITE_TITLE = 'vroom: candlestick charts that keep up';
export const SITE_DESCRIPTION =
  'Skia-powered candlestick charts for React Native and the web. One C++ core, 60+ FPS pan, pinch, and zoom, nine built-in indicators, drawing tools, and an AI agent toolkit.';

export const GITHUB_URL = 'https://github.com/keepitreal/vroom';

export const docsRoute = '/docs';

export const NAV = [
  { label: 'Docs', href: '/docs' },
  { label: 'Quickstart', href: '/docs/getting-started/quickstart' },
  { label: 'API', href: '/docs/reference' },
  { label: 'Agents', href: '/docs/guides/ai-agents/overview' },
  { label: 'GitHub', href: GITHUB_URL },
] as const;
