# Type Alias: VWAPConfig

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Defined in: [types.ts:115](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L115)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Defined in: [types.ts:120](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L120)

Line color (hex string or packed ARGB number).

***

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types.ts:116](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L116)

***

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Defined in: [types.ts:118](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L118)

Session reset offset from UTC midnight, in minutes (default 0).

***

### width?

```ts
optional width?: number;
```

Defined in: [types.ts:122](https://github.com/keepitreal/vroom/blob/main/packages/react-native/src/types.ts#L122)

Stroke width in px. Default 1.5.
