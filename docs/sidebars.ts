import type { SidebarsConfig } from '@docusaurus/plugin-content-docs';

// docusaurus-plugin-typedoc regenerates content/reference/ (including this
// sidebar file) on every build. It's committed so this require resolves on a
// clean checkout / first build.
const typedocSidebar = require('./content/reference/typedoc-sidebar.cjs');

const sidebars: SidebarsConfig = {
  docsSidebar: [
    'intro',
    {
      type: 'category',
      label: 'Getting started',
      items: ['getting-started/installation', 'getting-started/quickstart'],
    },
    {
      type: 'category',
      label: 'Guides',
      items: [
        'guides/why-vroom',
        'guides/architecture',
        'guides/theming',
        'guides/indicators',
        'guides/crosshair-and-events',
        'guides/gestures-and-viewport',
        'guides/drawing-tools',
        'guides/price-lines',
      ],
    },
    {
      type: 'category',
      label: 'API reference',
      // Hand-written platform-differences page lives at content root (outside
      // content/reference/, which docusaurus-plugin-typedoc wipes on every build),
      // slotted in ahead of the generated entries.
      items: ['reference-platform-differences', ...(typedocSidebar.items ?? typedocSidebar)],
    },
  ],
};

export default sidebars;
