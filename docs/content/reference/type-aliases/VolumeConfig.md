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

Source: [types/src/index.ts:845](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L845)

Volume bar config. One bottom-anchored bar per candle on the price pane,
drawn under the candles and sharing their x position and body width.

Unlike the other indicator configs the bars are on by default, so omitting
this prop leaves the chart looking as it always has.

## Properties

### downColor?

```ts
optional downColor?: string | number;
```

Source: [types/src/index.ts:865](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L865)

Down-bar color (hex string or packed ARGB number). Defaults to `theme.accentBear`.

---

### enabled?

```ts
optional enabled?: boolean;
```

Source: [types/src/index.ts:847](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L847)

Draw the bars. Default true.

---

### height?

```ts
optional height?: number;
```

Source: [types/src/index.ts:859](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L859)

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

Source: [types/src/index.ts:849](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L849)

Bar opacity 0..1 (1 = opaque). Default 0.5, so bars read quieter than the candles.

---

### radius?

```ts
optional radius?: number;
```

Source: [types/src/index.ts:861](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L861)

Corner radius (px) of the _top_ of each bar. Defaults to `theme.volumeRadius`, else 0 (square).

---

### upColor?

```ts
optional upColor?: string | number;
```

Source: [types/src/index.ts:863](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L863)

Up-bar color (hex string or packed ARGB number). Defaults to `theme.accentBull`.
