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

Source: [types/src/index.ts:1275](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1275)

Everything the assistant computed.

## Properties

### events

```ts
events: OrbEvent[];
```

Source: [types/src/index.ts:1280](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1280)

Every event, in time order.

---

### sessions

```ts
sessions: OrbSession[];
```

Source: [types/src/index.ts:1276](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1276)

---

### signalsEnabled

```ts
signalsEnabled: boolean;
```

Source: [types/src/index.ts:1284](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1284)

False when a warning disabled signals.

---

### stats

```ts
stats: OrbStats;
```

Source: [types/src/index.ts:1281](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1281)

---

### trades

```ts
trades: OrbTrade[];
```

Source: [types/src/index.ts:1278](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1278)

Every trade, oldest first.

---

### warnings

```ts
warnings: OrbWarning[];
```

Source: [types/src/index.ts:1282](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1282)
