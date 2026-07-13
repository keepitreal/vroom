# Type Alias: VroomColor

```ts
type VroomColor = string | number;
```

Defined in: [types/src/index.ts:63](https://github.com/keepitreal/vroom/blob/0a8d2ca5e13b0152b7f2240281e36ee9b18049fd/packages/types/src/index.ts#L63)

A color value: a hex string (`'#0d1117'`, or 8-digit `'#aarrggbb'`) or a
packed ARGB number. In `VroomTheme` every field is optional — omitted colors
keep the library default.
