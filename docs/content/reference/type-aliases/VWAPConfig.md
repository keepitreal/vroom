# Type Alias: VWAPConfig

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Defined in: [types/src/index.ts:119](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L119)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Defined in: [types/src/index.ts:124](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L124)

Line color (hex string or packed ARGB number).

***

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types/src/index.ts:120](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L120)

***

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Defined in: [types/src/index.ts:122](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L122)

Session reset offset from UTC midnight, in minutes (default 0).

***

### width?

```ts
optional width?: number;
```

Defined in: [types/src/index.ts:126](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L126)

Stroke width in px. Default 1.5.
