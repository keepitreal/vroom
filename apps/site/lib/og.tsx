import { readFile } from 'node:fs/promises';
import { join } from 'node:path';
import { ImageResponse } from 'next/og';

export const OG_SIZE = { width: 1200, height: 630 };

const fontDir = join(process.cwd(), 'assets/fonts');
const fonts = Promise.all([
  readFile(join(fontDir, 'BarlowCondensed-Bold.ttf')),
  readFile(join(fontDir, 'IBMPlexMono-Medium.ttf')),
]);

/** Decorative candles along the bottom edge; fixed so every card looks the same. */
const CANDLES = [
  [40, 70, 52, 1], [60, 95, 70, 1], [55, 85, 60, 0], [70, 120, 90, 1], [85, 130, 110, 1],
  [80, 115, 95, 0], [95, 150, 120, 1], [110, 160, 140, 1], [100, 145, 118, 0], [120, 185, 160, 1],
  [140, 200, 180, 1], [130, 190, 150, 0], [150, 230, 205, 1], [175, 250, 235, 1], [165, 240, 190, 0],
  [185, 270, 250, 1], [205, 290, 275, 1], [195, 280, 230, 0], [220, 320, 300, 1], [250, 345, 330, 1],
] as const;

export async function renderOg({ eyebrow, title, description }: { eyebrow: string; title: string; description?: string }) {
  const [barlow, plex] = await fonts;
  return new ImageResponse(
    <div
      style={{
        width: '100%',
        height: '100%',
        display: 'flex',
        flexDirection: 'column',
        padding: '64px 72px',
        background: '#0B0B0C',
        color: '#ECE9E2',
        position: 'relative',
      }}
    >
      <div style={{ display: 'flex', alignItems: 'center', gap: 8 }}>
        {'VROOM'.split('').map((c, i) => (
          <div
            key={i}
            style={{
              width: 40,
              height: 48,
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              background: '#161618',
              border: '1px solid #26262a',
              borderRadius: 4,
              fontFamily: 'Plex',
              fontSize: 26,
            }}
          >
            {c}
          </div>
        ))}
        <div style={{ marginLeft: 24, fontFamily: 'Plex', fontSize: 22, letterSpacing: 4, color: '#3DDC84' }}>
          {eyebrow.toUpperCase()}
        </div>
      </div>
      <div
        style={{
          marginTop: 56,
          fontFamily: 'Barlow',
          fontSize: title.length > 28 ? 84 : 104,
          lineHeight: 0.95,
          maxWidth: 1000,
        }}
      >
        {title}
      </div>
      {description && (
        <div style={{ marginTop: 28, fontFamily: 'Plex', fontSize: 26, lineHeight: 1.4, color: 'rgba(236,233,226,0.62)', maxWidth: 860 }}>
          {description}
        </div>
      )}
      <div style={{ position: 'absolute', right: 72, bottom: 0, display: 'flex', alignItems: 'flex-end', gap: 10, height: 360 }}>
        {CANDLES.map(([lo, hi, body, up], i) => (
          <div key={i} style={{ position: 'relative', width: 14, height: 360, display: 'flex' }}>
            <div style={{ position: 'absolute', left: 6, bottom: lo - 40, width: 2, height: hi - lo, background: up ? '#3a3a3e' : '#2a2a2d' }} />
            <div
              style={{
                position: 'absolute',
                left: 0,
                bottom: (up ? lo : body) - 30,
                width: 14,
                height: Math.abs(body - lo),
                background: i === CANDLES.length - 1 ? '#3DDC84' : up ? '#3a3a3e' : '#222225',
              }}
            />
          </div>
        ))}
      </div>
    </div>,
    {
      ...OG_SIZE,
      fonts: [
        { name: 'Barlow', data: barlow, weight: 700 },
        { name: 'Plex', data: plex, weight: 500 },
      ],
    },
  );
}
