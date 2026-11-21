import { createGetUrl, llms, loader } from 'fumadocs-core/source';
import { metaSchema, pageSchema } from 'fumadocs-core/source/schema';
import { defineDocs } from 'fumadocs-mdx/macro';
import { docsRoute, SITE_URL } from './site';

const docs = defineDocs({
  dir: 'content/docs',
  docs: {
    schema: pageSchema,
    postprocess: { includeProcessedMarkdown: true },
  },
  meta: { schema: metaSchema },
});

export const source = loader({
  baseUrl: docsRoute,
  source: docs.toFumadocsSource(),
});

export const docsLlms = llms(source, {
  renderPage: async (page) =>
    `# ${page.data.title} (${SITE_URL}${page.url})\n\n${await page.data.getText('processed')}`,
});

const getImageUrl = createGetUrl('/og/docs');

export function getPageImage(page: { slugs: string[] }) {
  const segments = [...page.slugs, 'image.png'];
  return { segments, url: getImageUrl(segments) };
}
