# `VolumeConfig`

```ts
type VolumeConfig = {
  downColor?: string | number;
  enabled?: boolean;
  height?: number;
  opacity?: number;
  radius?: number;
  upColor?: string | number;
};
```

Source: [types/src/index.ts:458](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L458)

Volume bar config. One bottom-anchored bar per candle on the price pane,
drawn under the candles and sharing their x position and body width.

Unlike the other indicator configs the bars are on by default, so omitting
this prop leaves the chart looking as it always has.

## Properties

### downColor?

```ts
optional downColor?: string | number;
```

Source: [types/src/index.ts:478](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L478)

Down-bar color (hex string or packed ARGB number). Defaults to `theme.accentBear`.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:460](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L460)

Draw the bars. Default true.

---

### height?

```ts
optional height?: number;
```

Source: [types/src/index.ts:472](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L472)

Height of the tallest bar as a fraction of the price pane, 0..1.
Default 0.2.

This is a ceiling rather than a reserved strip: raising it lets the bars
reach further up over the candles rather than compressing them, matching
the conventional volume overlay. Heights always auto-fit the loudest volume
in view, so the tallest bar sits exactly at the ceiling.

---

### opacity?

```ts
optional opacity?: number;
```

Source: [types/src/index.ts:462](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L462)

Bar opacity 0..1 (1 = opaque). Default 0.5, so bars read quieter than the candles.

---

### radius?

```ts
optional radius?: number;
```

Source: [types/src/index.ts:474](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L474)

Corner radius (px) of the _top_ of each bar. Defaults to `theme.volumeRadius`, else 0 (square).

---

### upColor?

```ts
optional upColor?: string | number;
```

Source: [types/src/index.ts:476](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L476)

Up-bar color (hex string or packed ARGB number). Defaults to `theme.accentBull`.
