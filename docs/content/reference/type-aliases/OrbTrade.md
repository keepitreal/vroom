# `OrbTrade`

```ts
type OrbTrade = {
  currentStop: number;
  date: string;
  direction: OrbDirection;
  entryPrice: number;
  entryTimeMs: number;
  exitPrice: number | null;
  exitReason: "target" | "stop" | "time" | null;
  exitTimeMs: number | null;
  id: string;
  partial?: {
    pct: number;
    price: number;
    timeMs: number;
  };
  r: number | null;
  signalTimeMs: number;
  size: number;
  stop: number;
  target: number | null;
};
```

Source: [types/src/index.ts:1195](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1195)

A simulated trade. Prices are fixed at entry; only a breakeven move after a partial changes the stop.

## Properties

### currentStop

```ts
currentStop: number;
```

Source: [types/src/index.ts:1206](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1206)

The stop in force at the end (moved to the entry after a breakeven partial).

---

### date

```ts
date: string;
```

Source: [types/src/index.ts:1197](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1197)

---

### direction

```ts
direction: OrbDirection;
```

Source: [types/src/index.ts:1198](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1198)

---

### entryPrice

```ts
entryPrice: number;
```

Source: [types/src/index.ts:1202](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1202)

---

### entryTimeMs

```ts
entryTimeMs: number;
```

Source: [types/src/index.ts:1201](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1201)

---

### exitPrice

```ts
exitPrice: number | null;
```

Source: [types/src/index.ts:1213](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1213)

---

### exitReason

```ts
exitReason: "target" | "stop" | "time" | null;
```

Source: [types/src/index.ts:1214](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1214)

---

### exitTimeMs

```ts
exitTimeMs: number | null;
```

Source: [types/src/index.ts:1212](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1212)

Null while open.

---

### id

```ts
id: string;
```

Source: [types/src/index.ts:1196](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1196)

---

### partial?

```ts
optional partial?: {
  pct: number;
  price: number;
  timeMs: number;
};
```

Source: [types/src/index.ts:1217](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1217)

#### pct

```ts
pct: number;
```

#### price

```ts
price: number;
```

#### timeMs

```ts
timeMs: number;
```

---

### r

```ts
r: number | null;
```

Source: [types/src/index.ts:1216](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1216)

Realized R (partial blended in); null while open.

---

### signalTimeMs

```ts
signalTimeMs: number;
```

Source: [types/src/index.ts:1200](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1200)

Close time of the confirming candle.

---

### size

```ts
size: number;
```

Source: [types/src/index.ts:1210](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1210)

Suggested contracts: floor(riskPerTrade / (|entry − stop| × pointValue)).

---

### stop

```ts
stop: number;
```

Source: [types/src/index.ts:1204](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1204)

The stop at entry, which R is measured against.

---

### target

```ts
target: number | null;
```

Source: [types/src/index.ts:1208](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1208)

Null under 'holdToFlatten'.
