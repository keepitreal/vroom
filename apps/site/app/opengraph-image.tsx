import { OG_SIZE, renderOg } from '@/lib/og';

export const alt = 'vroom: candlestick charts that keep up';
export const size = OG_SIZE;
export const contentType = 'image/png';

export default function Image() {
  return renderOg({
    eyebrow: 'React Native + Web',
    title: 'Charts that keep up.',
    description: 'Candlestick charts drawn by one C++ core, at 60+ FPS.',
  });
}
