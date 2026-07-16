# Type Alias: VWAPConfig

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Defined in: [types/src/index.ts:225](https://github.com/keepitreal/vroom/blob/11ea0554127cf648fe93fe2063a46952fa64b5ac/packages/types/src/index.ts#L225)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Defined in: [types/src/index.ts:230](https://github.com/keepitreal/vroom/blob/11ea0554127cf648fe93fe2063a46952fa64b5ac/packages/types/src/index.ts#L230)

Line color (hex string or packed ARGB number).

***

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types/src/index.ts:226](https://github.com/keepitreal/vroom/blob/11ea0554127cf648fe93fe2063a46952fa64b5ac/packages/types/src/index.ts#L226)

***

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Defined in: [types/src/index.ts:228](https://github.com/keepitreal/vroom/blob/11ea0554127cf648fe93fe2063a46952fa64b5ac/packages/types/src/index.ts#L228)

Session reset offset from UTC midnight, in minutes (default 0).

***

### width?

```ts
optional width?: number;
```

Defined in: [types/src/index.ts:232](https://github.com/keepitreal/vroom/blob/11ea0554127cf648fe93fe2063a46952fa64b5ac/packages/types/src/index.ts#L232)

Stroke width in px. Default 1.5.
