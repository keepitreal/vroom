# Type Alias: VWAPConfig

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Defined in: [types/src/index.ts:185](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L185)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Defined in: [types/src/index.ts:190](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L190)

Line color (hex string or packed ARGB number).

***

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types/src/index.ts:186](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L186)

***

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Defined in: [types/src/index.ts:188](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L188)

Session reset offset from UTC midnight, in minutes (default 0).

***

### width?

```ts
optional width?: number;
```

Defined in: [types/src/index.ts:192](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L192)

Stroke width in px. Default 1.5.
