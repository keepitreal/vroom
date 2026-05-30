// VroomChart — Phase 0 wiring.
//
// On mount: ensure globalThis.VroomChartJSI is installed (TurboModule.install()),
// then call helloPicture() and render the returned SkPicture inside a Canvas.
// If a red 100×100 rect shows up, the entire C++ → JsiSkPicture → SharedValue
// → <Picture> chain is working and we can move to real chart rendering.

import React, { useMemo } from 'react';
import { View } from 'react-native';
import { Canvas, Picture } from '@shopify/react-native-skia';

import NativeVroomChart from './NativeVroomChart';
import type { VroomChartProps } from './types';
import './jsi.d';

let installed = false;
function ensureInstalled(): void {
  if (installed) return;
  const ok = NativeVroomChart.install();
  if (!ok) {
    throw new Error(
      'VroomChartModule.install() returned false. JSI runtime unavailable?',
    );
  }
  if (typeof globalThis.VroomChartJSI === 'undefined') {
    throw new Error(
      'VroomChartModule.install() succeeded but globalThis.VroomChartJSI is undefined.',
    );
  }
  installed = true;
}

export function VroomChart(props: VroomChartProps) {
  const { width = 360, height = 240 } = props;

  // Build the picture once on first render. We're not animating yet — Phase 0
  // just proves the round-trip. Real lifecycle (per-chart handle, gestures,
  // re-renders on data change) lands in Phase 1+.
  const picture = useMemo(() => {
    ensureInstalled();
    return globalThis.VroomChartJSI!.helloPicture();
  }, []);

  return (
    <View style={{ width, height, backgroundColor: '#0d1117' }}>
      <Canvas style={{ flex: 1 }}>
        <Picture picture={picture} />
      </Canvas>
    </View>
  );
}
