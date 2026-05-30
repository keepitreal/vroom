// Ambient declaration for the JSI HostObject installed by VroomChartModule.

import type { SkPicture } from '@shopify/react-native-skia';

export interface VroomChartJSI {
  /**
   * Phase 0 sanity check: returns a fresh SkPicture containing one
   * red 100×100 rect, drawn by C++ Skia code via SkPictureRecorder.
   */
  helloPicture(): SkPicture;
}

declare global {
  // eslint-disable-next-line no-var
  var VroomChartJSI: VroomChartJSI | undefined;
}

// eslint-disable-next-line @typescript-eslint/no-empty-interface
export {};
