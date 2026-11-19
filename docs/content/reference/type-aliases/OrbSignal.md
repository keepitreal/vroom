# `OrbSignal`

```ts
type OrbSignal = {
  close: number;
  direction: OrbDirection;
  level: number;
  status: OrbSignalStatus;
  timeMs: number;
  tradeId?: string;
};
```

Source: [types/src/index.ts:1177](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1177)

A confirmed breakout.

## Properties

### close

```ts
close: number;
```

Source: [types/src/index.ts:1184](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1184)

The confirming close.

---

### direction

```ts
direction: OrbDirection;
```

Source: [types/src/index.ts:1178](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1178)

---

### level

```ts
level: number;
```

Source: [types/src/index.ts:1182](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1182)

The level it broke: range high for a long, range low for a short.

---

### status

```ts
status: OrbSignalStatus;
```

Source: [types/src/index.ts:1185](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1185)

---

### timeMs

```ts
timeMs: number;
```

Source: [types/src/index.ts:1180](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1180)

Close time of the confirming signal candle.

---

### tradeId?

```ts
optional tradeId?: string;
```

Source: [types/src/index.ts:1187](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1187)

The trade it opened, when `status` is 'entered'.
