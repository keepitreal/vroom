# Type Alias: VroomColor

```ts
type VroomColor = string | number;
```

Defined in: [types/src/index.ts:45](https://github.com/keepitreal/vroom/blob/d6db35e53e01a5fc4d6ab203602aed3c6f0338da/packages/types/src/index.ts#L45)

A color value: a hex string (`'#0d1117'`, or 8-digit `'#aarrggbb'`) or a
packed ARGB number. In `VroomTheme` every field is optional — omitted colors
keep the library default.
