# `OrbStopMode`

```ts
type OrbStopMode = "oppositeSide" | "midpoint" | "fixedPoints";
```

Source: [types/src/index.ts:962](https://github.com/keepitreal/vroom/blob/main/packages/types/src/index.ts#L962)

Where the stop goes:
'oppositeSide' — long stop at the range low, short stop at the range high
'midpoint' — the range midpoint
'fixedPoints' — `stopPoints` from the entry
