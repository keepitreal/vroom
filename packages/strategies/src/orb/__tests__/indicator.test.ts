import { describe, expect, it } from 'vitest';
import { OrbEventTracker } from '../tracker';
import { ORB_DEFAULT_COLORS, orbToAnnotations } from '../annotations';
import { runOrb } from '../run';
import { at, day, type OHLC } from './helpers';

const D = '2026-06-01';
const STEP = 5 * 60_000;
const BREAK: Record<string, OHLC> = {
  '09:45': [20012, 20075, 20010, 20070],
  '09:50': [20072, 20080, 20065, 20075],
  '09:55': [20072, 20300, 19970, 20100],
};

describe('OrbEventTracker', () => {
  const bars = day(D, { ...BREAK });
  const live = (k: number) => {
    const prefix = bars.slice(0, k);
    const newest = prefix.at(-1)!.timeMs;
    return { res: runOrb(prefix, undefined, { nowMs: newest + STEP / 2, stepMs: STEP }), newest };
  };

  it('seeds silently, then fires each new event once', () => {
    const tr = new OrbEventTracker();
    const types: string[] = [];
    // First run at 09:40 (range forming): nothing to fire.
    for (let k = 3; k <= 8; k++) {
      const { res, newest } = live(k);
      types.push(...tr.update(res, newest, 'BTC').map((e) => e.type));
      // A re-run with the same data fires nothing new.
      expect(tr.update(res, newest, 'BTC')).toEqual([]);
    }
    expect(types).toEqual(['rangeFinalized', 'breakoutConfirmed', 'entry', 'stopHit']);
  });

  it('does not fire history on the first run or after a context change', () => {
    const tr = new OrbEventTracker();
    const { res, newest } = live(bars.length);
    expect(tr.update(res, newest, 'BTC')).toEqual([]);
    expect(tr.update(res, newest, 'SOL')).toEqual([]);
  });
});

describe('orbToAnnotations', () => {
  it('draws the range, lines, entry triangle, levels and an X on a stop', () => {
    const bars = day(D, { ...BREAK });
    const res = runOrb(bars, undefined, { stepMs: STEP });
    const a = orbToAnnotations(res, bars, undefined, STEP);
    expect(a.boxes).toHaveLength(1);
    expect(a.boxes[0]).toMatchObject({ a: { timeMs: at(D, '09:30'), price: 20060 }, b: { timeMs: at(D, '09:45'), price: 19980 } });
    const flat = a.segments.filter((s) => s.a.timeMs === at(D, '09:45') && s.b.timeMs === at(D, '15:55'));
    expect(flat.map((s) => s.a.price).sort()).toEqual([19980, 20060]);
    expect(a.markers.find((m) => m.shape === 'triangleUp')).toMatchObject({ timeMs: at(D, '09:50'), price: 20065 });
    expect(a.markers.find((m) => m.shape === 'cross')).toMatchObject({ timeMs: at(D, '09:55'), price: 19980, color: '#ef5350' });
    // entry, stop and target levels from entry to exit
    expect(a.segments.filter((s) => s.a.timeMs === at(D, '09:50') && s.b.timeMs === at(D, '09:55'))).toHaveLength(3);
  });

  it('labels an entry with its risk in price, and a skipped day', () => {
    const bars = day(D, { ...BREAK });
    // Entry at the 09:50 open (20072), stop at the range low (19980).
    const entry = orbToAnnotations(runOrb(bars, undefined, { stepMs: STEP }), bars, undefined, STEP);
    expect(entry.labels.map((l) => l.text)).toContain('Long · risk 92');

    const wide = day(D, { '09:40': [20040, 20055, 19500, 20010] });
    const skipped = orbToAnnotations(runOrb(wide), wide, undefined, STEP);
    expect(skipped.labels.map((l) => l.text)).toContain('ORB skipped: range too wide');
  });

  it('emits only hex colors, the format the chart cores parse', () => {
    const bars = day(D, { ...BREAK });
    const res = runOrb(bars, undefined, { stepMs: STEP });
    const a = orbToAnnotations(res, bars, { display: { showMidline: true, showOpenPrice: true } }, STEP);
    const colors = [
      ...Object.values(ORB_DEFAULT_COLORS),
      ...a.segments.map((s) => s.color),
      ...a.boxes.flatMap((b) => [b.fill, b.border ?? '#000000']),
      ...a.markers.map((m) => m.color),
      ...a.labels.flatMap((l) => [l.color, l.background ?? '#000000']),
    ];
    for (const c of colors) expect(String(c)).toMatch(/^#([0-9a-f]{6}|[0-9a-f]{8})$/i);
  });

  it('draws every session unless onlyShowToday is on', () => {
    const bars = [...day('2026-05-29'), ...day(D)];
    const res = runOrb(bars);
    expect(orbToAnnotations(res, bars, undefined, STEP).boxes).toHaveLength(2);
    expect(orbToAnnotations(res, bars, { display: { onlyShowToday: true } }, STEP).boxes).toHaveLength(1);
  });
});
