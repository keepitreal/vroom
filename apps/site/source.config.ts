import { defineConfig } from 'fumadocs-mdx/config';
import { remarkAdmonition } from 'fumadocs-core/mdx-plugins/remark-admonition';

export default defineConfig({
  mdxOptions: {
    remarkPlugins: [remarkAdmonition],
    rehypeCodeOptions: {
      themes: { light: 'github-light', dark: 'github-dark-dimmed' },
    },
  },
});
