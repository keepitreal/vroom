# `OrbSession`

```ts
type OrbSession = {
  close?: number;
  date: string;
  entryCutoffMs: number;
  finalMs?: number;
  flattenMs: number;
  high?: number;
  low?: number;
  open?: number;
  pending?: {
    direction: OrbDirection;
    level: number;
    price: number;
    timeMs: number;
  };
  rangeEndMs: number;
  rangeStartMs: number;
  signals: OrbSignal[];
  skipReason?: OrbSkipReason;
  status: "forming" | "final" | "skipped";
  trades: OrbTrade[];
  widthPct?: number;
};
```

Source: [types/src/index.ts:1215](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1215)

One session (trading day) the assistant evaluated.

## Properties

### close?

```ts
optional close?: number;
```

Source: [types/src/index.ts:1233](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1233)

---

### date

```ts
date: string;
```

Source: [types/src/index.ts:1217](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1217)

`'YYYY-MM-DD'` in the session timezone.

---

### entryCutoffMs

```ts
entryCutoffMs: number;
```

Source: [types/src/index.ts:1220](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1220)

---

### finalMs?

```ts
optional finalMs?: number;
```

Source: [types/src/index.ts:1237](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1237)

When the range became final (the last window bar's close).

---

### flattenMs

```ts
flattenMs: number;
```

Source: [types/src/index.ts:1221](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1221)

---

### high?

```ts
optional high?: number;
```

Source: [types/src/index.ts:1229](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1229)

Range extremes; absent until a bar lands in the window.

---

### low?

```ts
optional low?: number;
```

Source: [types/src/index.ts:1230](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1230)

---

### open?

```ts
optional open?: number;
```

Source: [types/src/index.ts:1232](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1232)

Open of the first and close of the last bar in the window.

---

### pending?

```ts
optional pending?: {
  direction: OrbDirection;
  level: number;
  price: number;
  timeMs: number;
};
```

Source: [types/src/index.ts:1241](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1241)

A breakout on a still-forming signal candle — shown, never traded or alerted.

#### direction

```ts
direction: OrbDirection;
```

#### level

```ts
level: number;
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

### rangeEndMs

```ts
rangeEndMs: number;
```

Source: [types/src/index.ts:1219](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1219)

---

### rangeStartMs

```ts
rangeStartMs: number;
```

Source: [types/src/index.ts:1218](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1218)

---

### signals

```ts
signals: OrbSignal[];
```

Source: [types/src/index.ts:1238](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1238)

---

### skipReason?

```ts
optional skipReason?: OrbSkipReason;
```

Source: [types/src/index.ts:1227](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1227)

---

### status

```ts
status: "forming" | "final" | "skipped";
```

Source: [types/src/index.ts:1226](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1226)

'forming' — the range window hasn't closed yet; 'final' — evaluated;
'skipped' — see `skipReason`.

---

### trades

```ts
trades: OrbTrade[];
```

Source: [types/src/index.ts:1239](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1239)

---

### widthPct?

```ts
optional widthPct?: number;
```

Source: [types/src/index.ts:1235](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L1235)

(high − low) / midpoint × 100.
