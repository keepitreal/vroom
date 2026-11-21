'use client';

import dynamic from 'next/dynamic';

/** The chart needs WebGL and WASM, so it only ever renders in the browser. */
export const VroomChart = dynamic(() => import('@vroomchart/react').then((m) => m.VroomChart), {
  ssr: false,
});
