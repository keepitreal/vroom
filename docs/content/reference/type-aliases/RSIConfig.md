# Type Alias: RSIConfig

```ts
type RSIConfig = {
  enabled?: boolean;
  lowerBand?: number;
  maEnabled?: boolean;
  maPeriod?: number;
  period?: number;
  upperBand?: number;
};
```

Defined in: [types/src/index.ts:148](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L148)

RSI indicator config. Rendered in a pane below the candles when enabled.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types/src/index.ts:149](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L149)

***

### lowerBand?

```ts
optional lowerBand?: number;
```

Defined in: [types/src/index.ts:155](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L155)

Oversold band level (0..100). Default 30.

***

### maEnabled?

```ts
optional maEnabled?: boolean;
```

Defined in: [types/src/index.ts:157](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L157)

Show the RSI-based moving-average trendline. Default true.

***

### maPeriod?

```ts
optional maPeriod?: number;
```

Defined in: [types/src/index.ts:159](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L159)

Trendline (MA of RSI) length. Default 14, clamped to >= 1.

***

### period?

```ts
optional period?: number;
```

Defined in: [types/src/index.ts:151](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L151)

Lookback period in candle counts. Default 14, clamped to >= 2.

***

### upperBand?

```ts
optional upperBand?: number;
```

Defined in: [types/src/index.ts:153](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L153)

Overbought band level (0..100). Default 70.
