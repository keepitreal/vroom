# `OrbResult`

```ts
type OrbResult = {
  events: OrbEvent[];
  sessions: OrbSession[];
  signalsEnabled: boolean;
  stats: OrbStats;
  trades: OrbTrade[];
  warnings: OrbWarning[];
};
```

Source: [types/src/index.ts:1281](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1281)

Everything the assistant computed.

## Properties

### events

```ts
events: OrbEvent[];
```

Source: [types/src/index.ts:1286](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1286)

Every event, in time order.

---

### sessions

```ts
sessions: OrbSession[];
```

Source: [types/src/index.ts:1282](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1282)

---

### signalsEnabled

```ts
signalsEnabled: boolean;
```

Source: [types/src/index.ts:1290](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1290)

False when a warning disabled signals.

---

### stats

```ts
stats: OrbStats;
```

Source: [types/src/index.ts:1287](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1287)

---

### trades

```ts
trades: OrbTrade[];
```

Source: [types/src/index.ts:1284](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1284)

Every trade, oldest first.

---

### warnings

```ts
warnings: OrbWarning[];
```

Source: [types/src/index.ts:1288](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1288)
