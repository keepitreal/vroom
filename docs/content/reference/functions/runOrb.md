# `runOrb()`

```ts
function runOrb(candles, config?, opts?): OrbResult;
```

Source: strategies/dist/index.d.ts:372

Runs the Opening Range Breakout assistant over `candles` (oldest first).
Pure and deterministic: the same bars, config and `nowMs` always give the
same result, and nothing about a session depends on bars after the one
being evaluated.

## Parameters

| Parameter | Type                                                |
| --------- | --------------------------------------------------- |
| `candles` | readonly `Candle`[]                                 |
| `config?` | `OrbConfig`                                         |
| `opts?`   | [`RunOrbOptions`](../type-aliases/RunOrbOptions.md) |

## Returns

`OrbResult`
