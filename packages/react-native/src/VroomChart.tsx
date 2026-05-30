// VroomChart — Phase 1.
//
// Creates a C++ chart core via JSI, pushes candle data + size in, and
// renders the returned SkPicture inside a <Canvas><Picture/>. No gestures
// yet — visible range is whatever the core defaults to (full data range).

import React from 'react';
import { View } from 'react-native';
import { Canvas, Picture } from '@shopify/react-native-skia';

import { useChartCore } from './useChartCore';
import type { VroomChartProps } from './types';
import './jsi.d';

export function VroomChart(props: VroomChartProps) {
  const { candles, width = 360, height = 240, visibleRange } = props;

  const picture = useChartCore(candles, { width, height }, visibleRange);

  return (
    <View style={{ width, height }}>
      <Canvas style={{ flex: 1 }}>
        {picture ? <Picture picture={picture} /> : null}
      </Canvas>
    </View>
  );
}
