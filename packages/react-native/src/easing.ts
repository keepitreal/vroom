// Mirror of packages/react/src/easing.ts — the platform packages don't depend on
// each other, and @vroomchart/types carries types only.

import type { TransitionEasing } from '@vroomchart/types';

/**
 * Maps linear animation progress (0..1) to eased progress (0..1) for the chart's
 * transitions. Unknown values fall back to `'ease-in-out'`, which is a
 * smoothstep — the curve the candle↔line transition has always used.
 */
export function ease(kind: TransitionEasing | undefined, p: number): number {
  switch (kind) {
    case 'linear':
      return p;
    case 'ease-in':
      return p * p;
    case 'ease-out':
      return p * (2 - p);
    default:
      return p * p * (3 - 2 * p);
  }
}

// Index order matches VroomEasing in vroom_chart.h.
const EASINGS: readonly TransitionEasing[] = [
  'linear',
  'ease-in',
  'ease-out',
  'ease-in-out',
];

/**
 * The curve as a `VroomEasing` index, for the animations the core paces itself
 * (see `setVolumeCollapse`) rather than taking pre-eased progress. Falls back to
 * `'ease-in-out'`, matching {@link ease}.
 */
export function easingIndex(kind: TransitionEasing | undefined): number {
  const i = kind ? EASINGS.indexOf(kind) : -1;
  return i < 0 ? EASINGS.indexOf('ease-in-out') : i;
}
