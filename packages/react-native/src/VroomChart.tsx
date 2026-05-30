// VroomChart — Phase 3.
//
// Owns a SharedValue<SkPicture> driven by:
//   - useChartCore's "initial" picture (when data/size/range change), AND
//   - Pan gesture callbacks that call handle.pan(dx, dy) → fresh picture.
//
// Reanimated 4 + RN-Skia 2 propagate SharedValue<SkPicture> changes to
// <Picture> without a React re-render, so gesture-driven redraws are cheap.
//
// Gestures run on the JS thread for now (`runOnJS(true)`) — installing the
// JSI bindings on the worklet runtime is a later perf optimization.

import React, { useEffect } from 'react';
import { View } from 'react-native';
import { Canvas, Picture, type SkPicture } from '@shopify/react-native-skia';
import {
  Gesture,
  GestureDetector,
  GestureHandlerRootView,
} from 'react-native-gesture-handler';
import { useSharedValue, type SharedValue } from 'react-native-reanimated';

import { useChartCore } from './useChartCore';
import type { VroomChartProps } from './types';
import './jsi.d';

export function VroomChart(props: VroomChartProps) {
  const { candles, width = 360, height = 240, visibleRange, onViewportChange } = props;

  const { handle, picture } = useChartCore(candles, { width, height }, visibleRange);
  const pictureSV = useSharedValue(picture);

  // Sync the initial picture from useChartCore into the SV whenever it
  // refreshes (data load, size change, externally-controlled range change).
  useEffect(() => {
    pictureSV.value = picture;
  }, [picture, pictureSV]);

  const pan = Gesture.Pan()
    .runOnJS(true)
    .onChange((e) => {
      if (!handle) return;
      const next = handle.pan(e.changeX, e.changeY);
      if (next) pictureSV.value = next;
    })
    .onEnd(() => {
      // Inform the host so it can mirror the C++ visible range if needed.
      // (The actual range is held in C++ and read on next gesture.)
      onViewportChange?.(0, 0);
    });

  return (
    <GestureHandlerRootView style={{ width, height }}>
      <GestureDetector gesture={pan}>
        <View style={{ width, height }}>
          <Canvas style={{ flex: 1 }}>
            {picture ? (
              // Once the initial picture has landed, pictureSV is guaranteed
              // non-null — gesture callbacks only ever assign non-null values.
              <Picture picture={pictureSV as SharedValue<SkPicture>} />
            ) : null}
          </Canvas>
        </View>
      </GestureDetector>
    </GestureHandlerRootView>
  );
}
