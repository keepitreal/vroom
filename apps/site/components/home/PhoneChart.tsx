'use client';

import { useMemo, useState } from 'react';
import { type Candle } from '@vroomchart/react';
import { VroomChart } from './ClientChart';
import { aggregate, dayChange, formatChange, formatPrice, STEP_MS } from './market';
import { QUIET_VOLUME, SITE_THEME } from './scenes';

const INTERVALS = [
  { label: '1M', ms: STEP_MS },
  { label: '5M', ms: 5 * STEP_MS },
  { label: '15M', ms: 15 * STEP_MS },
  { label: '1H', ms: 60 * STEP_MS },
] as const;

/** A second, touchable chart in a phone frame: the same core, phone-sized. */
export function PhoneChart({ candles, reduceMotion }: { candles: Candle[]; reduceMotion: boolean }) {
  const [active, setActive] = useState<(typeof INTERVALS)[number]>(INTERVALS[0]);
  const series = useMemo(() => aggregate(candles, active.ms), [candles, active]);
  const last = candles[candles.length - 1];
  const change = dayChange(candles);

  return (
    <div className="phone">
      <div className="phone__island" aria-hidden />
      <div className="phone__screen">
        <header className="phone__head">
          <span className="mono phone__sym">BTC-PERP</span>
          <div className="phone__quote">
            <span className="phone__price">{formatPrice(last.close)}</span>
            <span className={`mono ${change >= 0 ? 'up' : 'down'}`}>{formatChange(change)}</span>
          </div>
          <div className="phone__intervals" role="tablist" aria-label="Interval">
            {INTERVALS.map((iv) => (
              <button
                key={iv.label}
                role="tab"
                aria-selected={iv === active}
                className={iv === active ? 'is-active' : undefined}
                onClick={() => setActive(iv)}
              >
                {iv.label}
              </button>
            ))}
          </div>
        </header>
        <div className="phone__chart">
          <VroomChart
            candles={series}
            seriesKey="BTC-PERP-phone"
            theme={{ ...SITE_THEME, axisFontSize: 10 }}
            volume={QUIET_VOLUME}
            defaultCandleWidth={5}
            streamTransition="transform"
            transitionMs={350}
            crosshair="simple"
            reduceMotion={reduceMotion || undefined}
          />
        </div>
      </div>
    </div>
  );
}
