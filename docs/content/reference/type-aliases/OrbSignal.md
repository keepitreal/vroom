# `OrbSignal`

```ts
type OrbSignal = {
  close: number;
  direction: OrbDirection;
  level: number;
  size?: number;
  status: OrbSignalStatus;
  timeMs: number;
  tradeId?: string;
};
```

Source: [types/src/index.ts:1179](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1179)

A confirmed breakout.

## Properties

### close

```ts
close: number;
```

Source: [types/src/index.ts:1186](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1186)

The confirming close.

---

### direction

```ts
direction: OrbDirection;
```

Source: [types/src/index.ts:1180](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1180)

---

### level

```ts
level: number;
```

Source: [types/src/index.ts:1184](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1184)

The level it broke: range high for a long, range low for a short.

---

### size?

```ts
optional size?: number;
```

Source: [types/src/index.ts:1191](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1191)

Suggested size, when one was computed.

---

### status

```ts
status: OrbSignalStatus;
```

Source: [types/src/index.ts:1187](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1187)

---

### timeMs

```ts
timeMs: number;
```

Source: [types/src/index.ts:1182](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1182)

Close time of the confirming signal candle.

---

### tradeId?

```ts
optional tradeId?: string;
```

Source: [types/src/index.ts:1189](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1189)

The trade it opened, when `status` is 'entered'.
