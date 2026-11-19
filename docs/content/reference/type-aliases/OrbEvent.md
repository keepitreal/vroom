# `OrbEvent`

```ts
type OrbEvent = {
  date: string;
  direction?: OrbDirection;
  id: string;
  price?: number;
  r?: number;
  reason?: OrbSkipReason;
  timeMs: number;
  tradeId?: string;
  type: OrbEventType;
};
```

Source: [types/src/index.ts:1147](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1147)

One alert-worthy moment. `id` is stable across re-runs, so the same event is
never reported twice, and history already on the chart when it loads is not
replayed.

## Properties

### date

```ts
date: string;
```

Source: [types/src/index.ts:1153](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1153)

Session date, `'YYYY-MM-DD'` in the session timezone.

---

### direction?

```ts
optional direction?: OrbDirection;
```

Source: [types/src/index.ts:1154](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1154)

---

### id

```ts
id: string;
```

Source: [types/src/index.ts:1148](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1148)

---

### price?

```ts
optional price?: number;
```

Source: [types/src/index.ts:1156](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1156)

The relevant price: range high/low midpoint, confirming close, fill price.

---

### r?

```ts
optional r?: number;
```

Source: [types/src/index.ts:1162](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1162)

Realized R, on exits.

---

### reason?

```ts
optional reason?: OrbSkipReason;
```

Source: [types/src/index.ts:1158](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1158)

For 'daySkipped'.

---

### timeMs

```ts
timeMs: number;
```

Source: [types/src/index.ts:1151](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1151)

When it happened (epoch ms): a bar open, or a candle close for confirmations.

---

### tradeId?

```ts
optional tradeId?: string;
```

Source: [types/src/index.ts:1160](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1160)

The trade an entry or exit belongs to.

---

### type

```ts
type: OrbEventType;
```

Source: [types/src/index.ts:1149](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1149)
