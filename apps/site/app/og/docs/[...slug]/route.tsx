import { notFound } from 'next/navigation';
import { renderOg } from '@/lib/og';
import { getPageImage, source } from '@/lib/source';

export const revalidate = false;

export async function GET(_req: Request, { params }: { params: Promise<{ slug: string[] }> }) {
  const { slug } = await params;
  const page = source.getPage(slug.slice(0, -1));
  if (!page) notFound();
  return renderOg({ eyebrow: 'Docs', title: page.data.title, description: page.data.description });
}

export function generateStaticParams() {
  return source.getPages().map((page) => ({ slug: getPageImage(page).segments }));
}
