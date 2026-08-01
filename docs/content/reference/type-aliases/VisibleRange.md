# Type Alias: VisibleRange

```ts
type VisibleRange = {
  endMs: number;
  startMs: number;
};
```

Defined in: [types/src/index.ts:108](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L108)

A time window over the candle data, as Unix epoch milliseconds.

## Properties

### endMs

```ts
endMs: number;
```

Defined in: [types/src/index.ts:112](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L112)

Window end (inclusive), Unix epoch milliseconds.

***

### startMs

```ts
startMs: number;
```

Defined in: [types/src/index.ts:110](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L110)

Window start (inclusive), Unix epoch milliseconds.
