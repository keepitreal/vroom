import {
  DocsBody,
  DocsDescription,
  DocsPage,
  DocsTitle,
  MarkdownCopyButton,
  ViewOptionsPopover,
} from 'fumadocs-ui/layouts/docs/page';
import { createRelativeLink } from 'fumadocs-ui/mdx';
import type { Metadata } from 'next';
import type { ComponentProps } from 'react';
import { notFound } from 'next/navigation';
import { getMDXComponents } from '@/components/mdx';
import { JsonLd } from '@/components/json-ld';
import { GITHUB_URL, SITE_NAME, SITE_URL } from '@/lib/site';
import { getPageImage, source } from '@/lib/source';

type Props = { params: Promise<{ slug?: string[] }> };

export default async function Page({ params }: Props) {
  const { slug } = await params;
  const page = source.getPage(slug);
  if (!page) notFound();

  const MDX = page.data.body;
  const markdownUrl = `${page.url}.md`;
  const crumbs = page.slugs.flatMap((_, i) => {
    const p = source.getPage(page.slugs.slice(0, i + 1));
    return p ? [{ name: p.data.title, url: `${SITE_URL}${p.url}` }] : [];
  });

  return (
    <DocsPage toc={page.data.toc} full={page.data.full}>
      <JsonLd
        data={[
          {
            '@context': 'https://schema.org',
            '@type': 'TechArticle',
            headline: page.data.title,
            description: page.data.description,
            url: `${SITE_URL}${page.url}`,
            isPartOf: { '@type': 'WebSite', name: SITE_NAME, url: SITE_URL },
          },
          {
            '@context': 'https://schema.org',
            '@type': 'BreadcrumbList',
            itemListElement: [{ name: 'Docs', url: `${SITE_URL}/docs` }, ...crumbs].map((c, i) => ({
              '@type': 'ListItem',
              position: i + 1,
              name: c.name,
              item: c.url,
            })),
          },
        ]}
      />
      <DocsTitle>{page.data.title}</DocsTitle>
      <DocsDescription className="mb-0">{page.data.description}</DocsDescription>
      <div className="flex flex-row items-center gap-2 border-b pb-6">
        <MarkdownCopyButton markdownUrl={markdownUrl} />
        <ViewOptionsPopover
          markdownUrl={markdownUrl}
          githubUrl={`${GITHUB_URL}/blob/main/apps/site/content/docs/${encodeURI(page.path)}`}
        />
      </div>
      <DocsBody>
        <MDX components={getMDXComponents({ a: relativeLink(page) })} />
      </DocsBody>
    </DocsPage>
  );
}

/** Fumadocs only resolves `./` and `../` links; TypeDoc emits bare ones like `Candle.md`. */
function relativeLink(page: NonNullable<ReturnType<typeof source.getPage>>) {
  const Link = createRelativeLink(source, page);
  return function DocLink({ href, ...props }: ComponentProps<'a'>) {
    const bare = href && /^[\w-][^:]*\.mdx?(#.*)?$/.test(href);
    return <Link href={bare ? `./${href}` : href} {...props} />;
  };
}

export function generateStaticParams() {
  return source.generateParams();
}

export async function generateMetadata({ params }: Props): Promise<Metadata> {
  const { slug } = await params;
  const page = source.getPage(slug);
  if (!page) notFound();

  const image = getPageImage(page).url;
  return {
    title: page.data.title,
    description: page.data.description,
    alternates: { canonical: page.url },
    openGraph: { type: 'article', url: page.url, images: image },
    twitter: { card: 'summary_large_image', images: image },
  };
}
