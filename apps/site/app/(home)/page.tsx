import type { Metadata } from 'next';
import { App } from '@/components/home/App';
import { JsonLd } from '@/components/json-ld';
import { getBtcCandles } from '@/lib/candles';
import { GITHUB_URL, SITE_DESCRIPTION, SITE_NAME, SITE_TITLE, SITE_URL } from '@/lib/site';
import '@/components/home/home.css';

export const revalidate = 60;

export const metadata: Metadata = {
  title: { absolute: SITE_TITLE },
  alternates: { canonical: '/' },
};

export default async function HomePage() {
  const { candles } = await getBtcCandles();
  return (
    <>
      <JsonLd
        data={{
          '@context': 'https://schema.org',
          '@type': 'SoftwareApplication',
          name: SITE_NAME,
          description: SITE_DESCRIPTION,
          url: SITE_URL,
          applicationCategory: 'DeveloperApplication',
          operatingSystem: 'iOS, Web',
          license: 'https://opensource.org/licenses/MIT',
          codeRepository: GITHUB_URL,
          offers: { '@type': 'Offer', price: '0', priceCurrency: 'USD' },
        }}
      />
      <App initialCandles={candles} />
    </>
  );
}
