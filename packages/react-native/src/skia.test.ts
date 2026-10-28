import { afterEach, describe, expect, it, vi } from 'vitest';

const Module = require('node:module');

// Skia's optional packages use CommonJS require, which vi.mock does not intercept.
const loader = Module as unknown as {
  _load: (request: string, ...args: unknown[]) => unknown;
};

afterEach(() => {
  vi.restoreAllMocks();
  vi.resetModules();
});

describe('makeRasterImage', () => {
  it.each(['MakeImage', 'MakeRasterImage', 'MakeRaster'])(
    'supports %s and preserves the native factory receiver',
    async (name) => {
      const image = {};
      const factory = {
        [name]: vi.fn(function (this: unknown) {
          expect(this).toBe(factory);
          return image;
        }),
      };
      const original = loader._load;
      vi.spyOn(loader, '_load').mockImplementation((request, ...args) => {
        if (request === 'react-native-skia') {
          if (name === 'MakeImage') throw new Error('v3 not installed');
          return { Skia: { Image: factory } };
        }
        if (request === '@shopify/react-native-skia' && name === 'MakeImage') {
          return { Skia: { Image: factory } };
        }
        return original(request, ...args);
      });
      const { makeRasterImage } = await import('./skia');
      const info = { width: 1, height: 1, colorType: 4, alphaType: 2 };
      const data = {} as Parameters<typeof makeRasterImage>[1];
      expect(makeRasterImage(info, data, 4)).toBe(image);
      expect(factory[name]).toHaveBeenCalledWith(info, data, 4);
    },
  );

  it('prefers MakeRaster when legacy names are also available', async () => {
    const factory = {
      MakeRaster: vi.fn(() => null),
      MakeRasterImage: vi.fn(),
      MakeImage: vi.fn(),
    };
    const original = loader._load;
    vi.spyOn(loader, '_load').mockImplementation((request, ...args) => {
      if (request === 'react-native-skia') return { Skia: { Image: factory } };
      return original(request, ...args);
    });
    const { makeRasterImage } = await import('./skia');
    makeRasterImage(
      { width: 1, height: 1, colorType: 4, alphaType: 2 },
      {} as Parameters<typeof makeRasterImage>[1],
      4,
    );
    expect(factory.MakeRaster).toHaveBeenCalledOnce();
    expect(factory.MakeRasterImage).not.toHaveBeenCalled();
    expect(factory.MakeImage).not.toHaveBeenCalled();
  });
});
