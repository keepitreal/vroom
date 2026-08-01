import type { Config } from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';
import { themes as prismThemes } from 'prism-react-renderer';

// NOTE: update `url` once the custom domain is live (used for canonical URLs
// and the sitemap). `baseUrl` stays '/' for a root-domain Vercel deployment.
const config: Config = {
  title: 'Vroom',
  tagline: 'Lightning-fast, Skia-powered candlestick charts for React Native and the web',
  favicon: 'img/favicon.svg',

  url: 'https://vroom-docs.vercel.app',
  baseUrl: '/',

  organizationName: 'keepitreal',
  projectName: 'vroom',

  // Generated API reference can contain TypeScript type syntax that isn't valid
  // MDX, so parse plain .md as CommonMark and reserve MDX for .mdx files.
  markdown: {
    format: 'detect',
    hooks: { onBrokenMarkdownLinks: 'warn' },
  },

  onBrokenLinks: 'throw',

  i18n: { defaultLocale: 'en', locales: ['en'] },

  plugins: [
    [
      'docusaurus-plugin-typedoc',
      {
        // Generated from the package's public types — never hand-edit
        // content/reference/. Re-run a build (or `pnpm docs:gen`) after
        // changing public types.
        entryPoints: ['../packages/react-native/src/index.ts'],
        tsconfig: '../packages/react-native/tsconfig.json',
        // Pin source links to the branch, not the commit SHA typedoc defaults
        // to — SHA-pinned links change on every commit, which made the CI
        // "reference is committed" check fail on every push by construction.
        gitRevision: 'main',
        out: 'content/reference',
        readme: 'none',
        excludeInternal: true,
        excludePrivate: true,
        entryFileName: 'index.md',
        hideBreadcrumbs: true,
        useCodeBlocks: true,
        expandObjects: true,
        indexFormat: 'table',
        parametersFormat: 'table',
        typeDeclarationFormat: 'table',
        sidebar: { pretty: true },
      },
    ],
  ],

  presets: [
    [
      'classic',
      {
        docs: {
          path: 'content',
          routeBasePath: '/',
          sidebarPath: './sidebars.ts',
          editUrl: 'https://github.com/keepitreal/vroom/edit/main/docs/content/',
        },
        blog: false,
        theme: {
          customCss: './src/css/custom.css',
        },
      } satisfies Preset.Options,
    ],
  ],

  themes: [
    [
      '@easyops-cn/docusaurus-search-local',
      {
        hashed: true,
        indexBlog: false,
        docsDir: 'content',
        docsRouteBasePath: '/',
      },
    ],
  ],

  themeConfig: {
    colorMode: { respectPrefersColorScheme: true },
    navbar: {
      title: 'vroom',
      items: [
        { type: 'docSidebar', sidebarId: 'docsSidebar', position: 'left', label: 'Docs' },
        {
          href: 'https://github.com/keepitreal/vroom',
          label: 'GitHub',
          position: 'right',
        },
      ],
    },
    footer: {
      style: 'dark',
      links: [
        {
          title: 'Docs',
          items: [
            { label: 'Quickstart', to: '/getting-started/quickstart' },
            { label: 'API reference', to: '/reference/' },
          ],
        },
        {
          title: 'More',
          items: [{ label: 'GitHub', href: 'https://github.com/keepitreal/vroom' }],
        },
      ],
      copyright: `Built with Docusaurus.`,
    },
    prism: {
      theme: prismThemes.github,
      darkTheme: prismThemes.dracula,
      additionalLanguages: ['bash', 'json'],
    },
  } satisfies Preset.ThemeConfig,
};

export default config;
