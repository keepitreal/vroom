# Type Alias: VWAPConfig

```ts
type VWAPConfig = {
  color?: string | number;
  enabled?: boolean;
  resetMinutes?: number;
  width?: number;
};
```

Defined in: [types/src/index.ts:193](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L193)

VWAP overlay config (session anchor). Drawn as a single line on the price
pane, resetting each session.

## Properties

### color?

```ts
optional color?: string | number;
```

Defined in: [types/src/index.ts:198](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L198)

Line color (hex string or packed ARGB number).

***

### enabled?

```ts
optional enabled?: boolean;
```

Defined in: [types/src/index.ts:194](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L194)

***

### resetMinutes?

```ts
optional resetMinutes?: number;
```

Defined in: [types/src/index.ts:196](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L196)

Session reset offset from UTC midnight, in minutes (default 0).

***

### width?

```ts
optional width?: number;
```

Defined in: [types/src/index.ts:200](https://github.com/keepitreal/vroom/blob/586d03172d9049213a34e30dc7132f06811d41a4/packages/types/src/index.ts#L200)

Stroke width in px. Default 1.5.
