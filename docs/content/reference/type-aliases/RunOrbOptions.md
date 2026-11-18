# `RunOrbOptions`

```ts
type RunOrbOptions = {
  nowMs?: number;
  stepMs?: number;
};
```

Source: strategies/dist/index.d.ts:359

## Properties

### nowMs?

```ts
optional nowMs?: number;
```

Source: strategies/dist/index.d.ts:365

Wall-clock time (epoch ms) deciding whether the newest bar has closed.
Defaults to +Infinity: every bar is closed, as in a backtest. Pass
`Date.now()` on a live chart so the forming bar is never traded.

---

### stepMs?

```ts
optional stepMs?: number;
```

Source: strategies/dist/index.d.ts:367

Bar length in ms. Inferred from the bar spacing when omitted.
