// The only module that names the RN-Skia package.
//
// RN-Skia v3 is published as `react-native-skia`; v2 is
// `@shopify/react-native-skia`. Metro only treats a require as optional inside
// a `try` block, so each package gets its own. Never install both: they would
// register two copies of the native module.

type SkiaModule = typeof import('@shopify/react-native-skia');

function load(): SkiaModule {
  try {
    return require('react-native-skia');
  } catch {}
  try {
    return require('@shopify/react-native-skia');
  } catch {}
  throw new Error(
    'react-native-vroom-chart: install @shopify/react-native-skia (v2) or react-native-skia (v3).',
  );
}

const skia = load();

// Explicit annotations keep declaration emit from inferring pnpm-store paths.
export const AlphaType: SkiaModule['AlphaType'] = skia.AlphaType;
export const ColorType: SkiaModule['ColorType'] = skia.ColorType;
export const Canvas: SkiaModule['Canvas'] = skia.Canvas;
export const Image: SkiaModule['Image'] = skia.Image;
export const Picture: SkiaModule['Picture'] = skia.Picture;
export const Skia: SkiaModule['Skia'] = skia.Skia;
export type { SkImage, SkPicture } from '@shopify/react-native-skia';

type ImageFactory = typeof Skia.Image;

/** v3 renamed `Skia.Image.MakeImage` to `MakeRasterImage`. */
export function makeRasterImage(
  ...args: Parameters<ImageFactory['MakeImage']>
): ReturnType<ImageFactory['MakeImage']> {
  const factory = Skia.Image as ImageFactory & {
    MakeRasterImage?: ImageFactory['MakeImage'];
  };
  // Host methods recover their factory via `this`, so call, don't detach.
  return (factory.MakeRasterImage ?? factory.MakeImage).apply(factory, args);
}
