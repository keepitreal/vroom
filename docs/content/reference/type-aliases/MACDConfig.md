# Type Alias: MACDConfig

```ts
type MACDConfig = {
  enabled?: boolean;
  fast?: number;
  signal?: number;
  slow?: number;
};
```

Defined in: [types/src/index.ts:251](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L251)

MACD indicator config. Rendered in its own pane below the candles.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types/src/index.ts:252](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L252)

***

### fast?

```ts
optional fast?: number;
```

Defined in: [types/src/index.ts:254](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L254)

Fast EMA length. Default 12.

***

### signal?

```ts
optional signal?: number;
```

Defined in: [types/src/index.ts:258](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L258)

Signal-line EMA length. Default 9.

***

### slow?

```ts
optional slow?: number;
```

Defined in: [types/src/index.ts:256](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L256)

Slow EMA length (forced > fast). Default 26.
