# Type Alias: MACDConfig

```ts
type MACDConfig = {
  enabled?: boolean;
  fast?: number;
  signal?: number;
  slow?: number;
};
```

Defined in: [types/src/index.ts:283](https://github.com/keepitreal/vroom/blob/11ea0554127cf648fe93fe2063a46952fa64b5ac/packages/types/src/index.ts#L283)

MACD indicator config. Rendered in its own pane below the candles.

## Properties

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types/src/index.ts:284](https://github.com/keepitreal/vroom/blob/11ea0554127cf648fe93fe2063a46952fa64b5ac/packages/types/src/index.ts#L284)

***

### fast?

```ts
optional fast?: number;
```

Defined in: [types/src/index.ts:286](https://github.com/keepitreal/vroom/blob/11ea0554127cf648fe93fe2063a46952fa64b5ac/packages/types/src/index.ts#L286)

Fast EMA length. Default 12.

***

### signal?

```ts
optional signal?: number;
```

Defined in: [types/src/index.ts:290](https://github.com/keepitreal/vroom/blob/11ea0554127cf648fe93fe2063a46952fa64b5ac/packages/types/src/index.ts#L290)

Signal-line EMA length. Default 9.

***

### slow?

```ts
optional slow?: number;
```

Defined in: [types/src/index.ts:288](https://github.com/keepitreal/vroom/blob/11ea0554127cf648fe93fe2063a46952fa64b5ac/packages/types/src/index.ts#L288)

Slow EMA length (forced > fast). Default 26.
