import type { MetadataRoute } from 'next';
import { SITE_URL } from '@/lib/site';
import { source } from '@/lib/source';

export default function sitemap(): MetadataRoute.Sitemap {
  return [
    { url: SITE_URL, changeFrequency: 'weekly', priority: 1 },
    ...source.getPages().map((page) => ({
      url: `${SITE_URL}${page.url}`,
      changeFrequency: 'weekly' as const,
      priority: page.slugs[0] === 'reference' ? 0.5 : 0.8,
    })),
  ];
}
