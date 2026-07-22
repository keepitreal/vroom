# Type Alias: VroomColor

```ts
type VroomColor = string | number;
```

Defined in: [types/src/index.ts:63](https://github.com/keepitreal/vroom/blob/ad6f4129cd8547aeef88698716895e0af77971c9/packages/types/src/index.ts#L63)

A color value: a hex string (`'#0d1117'`, or 8-digit `'#aarrggbb'`) or a
packed ARGB number. In `VroomTheme` every field is optional — omitted colors
keep the library default.
