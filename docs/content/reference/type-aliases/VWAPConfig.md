# Type Alias: VWAPConfig

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Defined in: [types/src/index.ts:268](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L268)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Defined in: [types/src/index.ts:273](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L273)

Line color (hex string or packed ARGB number).

***

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types/src/index.ts:269](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L269)

***

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Defined in: [types/src/index.ts:271](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L271)

Session reset offset from UTC midnight, in minutes (default 0).

***

### width?

```ts
optional width?: number;
```

Defined in: [types/src/index.ts:275](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L275)

Stroke width in px. Default 1.5.
