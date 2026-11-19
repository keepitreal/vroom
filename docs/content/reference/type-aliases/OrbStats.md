# `OrbStats`

```ts
type OrbStats = {
  avgR: number | null;
  longs: {
    netR: number;
    trades: number;
    wins: number;
  };
  losses: number;
  maxDrawdownR: number;
  netR: number;
  profitFactor: number | null;
  sessions: number;
  shorts: {
    netR: number;
    trades: number;
    wins: number;
  };
  skipped: Partial<Record<OrbSkipReason, number>>;
  trades: number;
  winRate: number | null;
  wins: number;
};
```

Source: [types/src/index.ts:1245](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1245)

Aggregates over every session loaded. R values are in multiples of initial risk.

## Properties

### avgR

```ts
avgR: number | null;
```

Source: [types/src/index.ts:1251](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1251)

---

### longs

```ts
longs: {
  netR: number;
  trades: number;
  wins: number;
}
```

Source: [types/src/index.ts:1256](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1256)

#### netR

```ts
netR: number;
```

#### trades

```ts
trades: number;
```

#### wins

```ts
wins: number;
```

---

### losses

```ts
losses: number;
```

Source: [types/src/index.ts:1248](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1248)

---

### maxDrawdownR

```ts
maxDrawdownR: number;
```

Source: [types/src/index.ts:1255](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1255)

---

### netR

```ts
netR: number;
```

Source: [types/src/index.ts:1252](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1252)

---

### profitFactor

```ts
profitFactor: number | null;
```

Source: [types/src/index.ts:1254](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1254)

Gross winning R / gross losing R; null with no losing trades.

---

### sessions

```ts
sessions: number;
```

Source: [types/src/index.ts:1258](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1258)

---

### shorts

```ts
shorts: {
  netR: number;
  trades: number;
  wins: number;
}
```

Source: [types/src/index.ts:1257](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1257)

#### netR

```ts
netR: number;
```

#### trades

```ts
trades: number;
```

#### wins

```ts
wins: number;
```

---

### skipped

```ts
skipped: Partial<Record<OrbSkipReason, number>>;
```

Source: [types/src/index.ts:1259](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1259)

---

### trades

```ts
trades: number;
```

Source: [types/src/index.ts:1246](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1246)

---

### winRate

```ts
winRate: number | null;
```

Source: [types/src/index.ts:1250](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1250)

0–1; null with no closed trades.

---

### wins

```ts
wins: number;
```

Source: [types/src/index.ts:1247](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1247)
